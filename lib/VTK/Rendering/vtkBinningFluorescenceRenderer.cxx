/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkBinningFluorescenceRenderer.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkBinningFluorescenceRenderer.h"

#include <vector>

#include "vtkCamera.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGLExtensionManager.h"
#include "vtkRenderWindow.h"

#include <cuda.h>
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include <cufft.h>

//#define TIME

#ifdef TIME
#include <stopwatch.h>
#endif // TIME

#ifndef VTK_IMPLEMENT_MESA_CXX
# include "vtkOpenGL.h"
#endif

#include <math.h>

#ifndef VTK_IMPLEMENT_MESA_CXX
vtkCxxRevisionMacro(vtkBinningFluorescenceRenderer, "$Revision: 1.12 $");
vtkStandardNewMacro(vtkBinningFluorescenceRenderer);
#endif


vtkBinningFluorescenceRenderer::vtkBinningFluorescenceRenderer() {
  this->PSFTexture = NULL;

  // Create render textures here. The accumulation texture will hold the final results
  // of the slice convolution operation and should be accessed by outside objects
  // that want the convolved result.
  vtkFramebufferObjectTexture *accumulationTexture = vtkFramebufferObjectTexture::New();
  vtkFramebufferObjectTexture *binningTexture = vtkFramebufferObjectTexture::New();
  vtkFramebufferObjectTexture *psfTexture = vtkFramebufferObjectTexture::New();
  psfTexture->SetTextureFormatToLuminance();

  binningTexture->AutomaticDimensionsOff();
  psfTexture->AutomaticDimensionsOff();

  this->AddFramebufferTexture(accumulationTexture);
  this->AddFramebufferTexture(binningTexture);
  this->AddFramebufferTexture(psfTexture);
  this->ExtensionsLoaded = 0;
  this->BinningPBO = -1;
  this->PSFPBO     = -1;
  this->PBOBufferSize    = -1;
}


vtkBinningFluorescenceRenderer::~vtkBinningFluorescenceRenderer() {
  this->DeletePBOs();
}


// Concrete open gl render method.
void vtkBinningFluorescenceRenderer::DeviceRender(void) {
  // Do not remove this MakeCurrent! Due to Start / End methods on
  // some objects which get executed during a pipeline update, 
  // other windows might get rendered since the last time
  // a MakeCurrent was called.
  this->RenderWindow->MakeCurrent();

  this->ResetTimer();
  this->StartTimer();

#ifdef TIME
  unsigned int renderTime = StopWatch::create();
  StopWatch::get(renderTime).reset();
  StopWatch::get(renderTime).start();
#endif // TIME

  this->LoadFragmentProgram();
  this->LoadExtensions();

  // Clear out OpenGL errors here
  while (glGetError());

  // Get shorter names for the various buffers
  vtkFramebufferObjectTexture *accumulationTexture =
    this->GetFramebufferTexture(ACCUMULATION_TEXTURE);
  vtkFramebufferObjectTexture *binningTexture =
    this->GetFramebufferTexture(BINNING_TEXTURE);
  vtkFramebufferObjectTexture *psfTexture =
    this->GetFramebufferTexture(PSF_RENDER_TEXTURE);

  // Clear out the accumulation buffer
  accumulationTexture->EnableTarget(this);
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  accumulationTexture->DisableTarget();

  // Compute the size of the binning and PSF textures. The size should
  // include enough padding to eliminate edge effects due to the cyclic
  // operation of FFT-based convolution. This should be the size of the
  // render window plus the size of the extent of the PSF after applying
  // the transform defined by the simulation region.
  int *size = this->GetSize();
  vtkImageData *psf = this->PSFTexture->GetInput();
  psf->Update();
  double *bbox = psf->GetBounds();
  double psfSize[3] = {bbox[1], bbox[3], bbox[5]};
  int textureSize[2];
  for (int i = 0; i < 2; i++) {
    textureSize[i] = size[i] + static_cast<int>(psfSize[i]/this->PixelSize + 0.5);
  }

  // Bumping up the texture size to a multiple of 64 should (presumably)
  // increase performance of FFT algorithm.
  int multipleSize = 64;
  int multiples = textureSize[0] / multipleSize;
  int remainder = textureSize[0] % multipleSize;
  int newSize   = multipleSize * (multiples + (remainder ? 1 : 0));
  textureSize[0] = newSize;
  textureSize[1] = newSize;

  int width  = textureSize[0];
  int height = textureSize[1];

  // Set the size of the binning and PSF textures
  binningTexture->SetTextureWidth(width);
  binningTexture->SetTextureHeight(height);
  psfTexture->SetTextureWidth(width);
  psfTexture->SetTextureHeight(height);

  // Iterate through the planes in the PSF. The slabs are centered
  // in z at each PSF slice.
  double halfPSFHeight = 0.5 * psfSize[2];
  double fIncr  = 0.25 * psf->GetSpacing()[2];
  double offset = 0.5 * fIncr; // We'll bin in sections one fIncr thick.
  double fStart = this->FocalPlaneDepth - halfPSFHeight + offset;
  double fEnd   = this->FocalPlaneDepth + halfPSFHeight - offset;
  double epsilon = 1.0e-3 * fIncr;

  // We need the position of the camera so we can set the clipping planes correctly.
  double camZ = this->GetActiveCamera()->GetPosition()[2];

  // Clear OpenGL errors.
  while (glGetError());

  // Initialize the buffer collection that gets passed to the
  // convolution routine.
  ConvolutionData cd;
  this->CreateConvolutionData(&cd, width, height);

  // Count number of occlusion queries we need to issue.
  int numQueries = 0;
  for (double depth = fStart; depth <= fEnd; depth += fIncr) numQueries++;

  // Setup occlusion queries.
  GLuint *queries      = new GLuint[numQueries];
  GLuint sampleCount;
  vtkgl::GenQueries(numQueries, queries);

  // Set the binning framebuffer texture as the rendering target.
  binningTexture->EnableTarget(this);

  // Turn off writing to the color and depth buffers.
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  glDepthMask(GL_FALSE);

  // Issue the queries to see which sections we actually need to convolve.
  int query = 0;
  for (double depth = fStart; depth <= fEnd; depth += fIncr) {
    this->GetActiveCamera()->SetClippingRange(camZ - depth - offset, camZ - depth + offset - epsilon);

    vtkgl::BeginQuery(vtkgl::SAMPLES_PASSED_ARB, queries[query++]);

    // Standard render method. It looks kind of ridiculous to go up 
    // so many levels in the class hierarchy, but we have to do it.
    this->Superclass::Superclass::Superclass::DeviceRender();

    vtkgl::EndQuery(vtkgl::SAMPLES_PASSED_ARB);
  }

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDepthMask(GL_TRUE);
  binningTexture->DisableTarget();

  query = 0;
  for (double depth = fStart; depth <= fEnd; depth += fIncr) {
    // Retrieve result of occlusion query
    vtkgl::GetQueryObjectuiv(queries[query++], vtkgl::QUERY_RESULT, &sampleCount);

    // Skip the slab convolution phase because nothing was rendered.
    if (sampleCount == 0) {
      binningTexture->DisableTarget();
      continue;
    }

    this->GetActiveCamera()->SetClippingRange(camZ - depth - offset, camZ - depth + offset - epsilon);

    //---------------------//
    // BINNING PHASE       //
    //---------------------//
#ifdef TIME    
    unsigned int binningTime = StopWatch::create();
    StopWatch::get(binningTime).reset();
    StopWatch::get(binningTime).start();
#endif // TIME

    // Set the binning framebuffer texture as the rendering target.
    binningTexture->EnableTarget(this);

    // Clear the binning framebuffer texture.
    glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT);
    glViewport(0, 0, width-1, height-1);
    glScissor(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPopAttrib();

    // Now render points for real
    this->Superclass::Superclass::Superclass::DeviceRender();

    // There is an OpenGL error generated because of a call in
    // vtkOpenGLCamera that sets the draw buffer to a buffer
    // that doesn't exist when a framebuffer object is bound.
    // It appears to not be a big deal. We clear it here.
    while (glGetError());

    // Now that binning is complete, disable the framebuffer texture as the rendering target.
    binningTexture->DisableTarget();

#ifdef TIME
    StopWatch::get(binningTime).stop();
    cout << "binning phase: " << StopWatch::get(binningTime).getTime() << endl;
#endif // TIME

    //---------------------//
    // PSF RENDERING PHASE //
    //---------------------//
#ifdef TIME
    unsigned int psfTime = StopWatch::create();
    StopWatch::get(psfTime).reset();
    StopWatch::get(psfTime).start();
#endif // TIME

    this->RenderPSFSlice(psfSize, this->FocalPlaneDepth - depth);
    if (glGetError() != GL_NO_ERROR)
      vtkErrorMacro(<< "Here");

#ifdef TIME
    StopWatch::get(psfTime).stop();
    cout << "psf phase: " << StopWatch::get(psfTime).getTime() << endl;
#endif // TIME

    //---------------------//
    // CONVOLUTION PHASE   //
    //---------------------//
    this->ComputeConvolution(&cd);

    //---------------------//
    // ACCUMULATION PHASE  //
    //---------------------//
#ifdef TIME
    unsigned int accumTime = StopWatch::create();
    StopWatch::get(accumTime).reset();
    StopWatch::get(accumTime).start();
#endif // TIME

    // Draw textured rectangle into the accumulation buffer.
    // Load the binning framebuffer texture.
    accumulationTexture->EnableTarget(this);

    while (glGetError());

    // Turn blending on for the accumulation buffer
    glEnable(GL_BLEND);
    vtkgl::BlendEquation(vtkgl::FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    // Disable a few things.
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    binningTexture->Render(this);
    this->TextureRender(BINNING_TEXTURE);
    binningTexture->PostRender();

    // Turn blending back off
    glDisable(GL_BLEND);

    accumulationTexture->DisableTarget();

#ifdef TIME
    StopWatch::get(accumTime).stop();
    cout << "accum phase: " << StopWatch::get(accumTime).getTime() << endl;
#endif // TIME
  } // for loop

  // Clean up buffers and occlusion query
  this->DestroyConvolutionData(&cd);
  vtkgl::DeleteQueries(numQueries, queries);

  // Reset camera clipping range so that the image will be rendered correctly.
  this->ResetCameraClippingRange();

  if (this->GenerateNoise) {
    // Add noise.
    accumulationTexture->EnableTarget(this);

    // Turn on blending and add noise.
    glEnable(GL_BLEND);
    vtkgl::BlendEquation(vtkgl::FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    this->RenderNoise(ACCUMULATION_TEXTURE);

    glDisable(GL_BLEND);

    accumulationTexture->DisableTarget();
  }

  this->RemapAndDisplay(ACCUMULATION_TEXTURE);

#ifdef TIME
  StopWatch::get(renderTime).stop();

  cout << "renderTime: " << StopWatch::get(renderTime).getTime() << endl;
#endif // TIME

  glFinish();
  this->StopTimer();
  this->PrintTime();
}


void vtkBinningFluorescenceRenderer::ComputeConvolution(ConvolutionData *cd) {
  vtkFramebufferObjectTexture *binningTexture =
    this->GetFramebufferTexture(BINNING_TEXTURE);
  vtkFramebufferObjectTexture *psfTexture =
    this->GetFramebufferTexture(PSF_RENDER_TEXTURE);

#ifdef TIME
  unsigned int realTotalTime   = StopWatch::create();
  unsigned int texReadbackTime = StopWatch::create();
  unsigned int mallocTime      = StopWatch::create();
  unsigned int copy2DeviceTime = StopWatch::create();
  unsigned int splitTime       = StopWatch::create();
  unsigned int fftTime         = StopWatch::create();
  unsigned int ifftTime        = StopWatch::create();
  unsigned int mergeTime       = StopWatch::create();
  unsigned int copy2HostTime   = StopWatch::create();
  unsigned int texDownloadTime = StopWatch::create();

  StopWatch::get(realTotalTime).reset();
  StopWatch::get(realTotalTime).start();
#endif // TIME

  float4 *binHostMem = new float4[cd->width*cd->height];
  float4 *psfHostMem = new float4[cd->width*cd->height];

#ifdef TIME
  StopWatch::get(texReadbackTime).reset();
  StopWatch::get(texReadbackTime).start();
#endif // TIME
  binningTexture->Load(this);
  glGetTexImage(vtkgl::TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, GL_FLOAT, binHostMem);
  binningTexture->PostRender();

  psfTexture->Load(this);
  glGetTexImage(vtkgl::TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE, GL_FLOAT, psfHostMem);
  psfTexture->PostRender();
#ifdef TIME
  StopWatch::get(texReadbackTime).stop();

  cout << "Tex readback time: " << 
    StopWatch::get(texReadbackTime).getTime() << endl;
#endif // TIME

#ifdef TIME
  StopWatch::get(mallocTime).reset();
  StopWatch::get(mallocTime).start();
#endif // TIME

#ifdef TIME
  StopWatch::get(mallocTime).stop();
  cout << "Malloc time: " << StopWatch::get(mallocTime).getTime() << endl;

  StopWatch::get(copy2DeviceTime).reset();
  StopWatch::get(copy2DeviceTime).start();
#endif // TIME

  // Copy over from host
  CUDA_SAFE_CALL_NO_SYNC(cudaMemcpy(cd->imagePtr, binHostMem, cd->imageSize,
    cudaMemcpyHostToDevice));

#ifdef TIME
  StopWatch::get(copy2DeviceTime).stop();

  // Split the image channels into R, G, B for separate
  // convolution.
  StopWatch::get(splitTime).reset();
  StopWatch::get(splitTime).start();
#endif // TIME

  CUDA_SAFE_CALL_NO_SYNC(splitPixels(cd->imagePtr, cd->width, cd->height,
    cd->binR, cd->binG, cd->binB));

#ifdef TIME
  StopWatch::get(splitTime).stop();

  StopWatch::get(copy2DeviceTime).start();
#endif // TIME
  CUDA_SAFE_CALL_NO_SYNC(cudaMemcpy(cd->imagePtr, psfHostMem, 
    sizeof(float)*cd->channelSize, cudaMemcpyHostToDevice));

#ifdef TIME
  StopWatch::get(copy2DeviceTime).stop();

  cout << "copy2DeviceTime: " << StopWatch::get(copy2DeviceTime).getTime() << endl;
  cout << "split time: " << StopWatch::get(splitTime).getTime() << endl;

  // Do the six forward FFTs.
  StopWatch::get(fftTime).reset();
  StopWatch::get(fftTime).start();
#endif // TIME

  CUDA_SAFE_CALL_NO_SYNC(cudaThreadSynchronize());

  forwardFFT(cd->binR, cd->binComplexR, cd->width, cd->height, cd->forwardPlan);
  forwardFFT(cd->binG, cd->binComplexG, cd->width, cd->height, cd->forwardPlan);
  forwardFFT(cd->binB, cd->binComplexB, cd->width, cd->height, cd->forwardPlan);
  forwardFFT((float*)cd->imagePtr, cd->psfComplex, cd->width, cd->height, cd->forwardPlan);

#ifdef TIME
  StopWatch::get(fftTime).stop();

  cout << "fft time: " << StopWatch::get(fftTime).getTime() << endl;
#endif // TIME

  // Do the three complex multiplies
  float scaleFactor = 1.0f / (float) (cd->width * cd->height);
  CUDA_SAFE_CALL_NO_SYNC(complexMultiply(cd->binComplexR, cd->psfComplex, 
    cd->binComplexR, cd->channelComplexSize, scaleFactor));
  CUDA_SAFE_CALL_NO_SYNC(complexMultiply(cd->binComplexG, cd->psfComplex, 
    cd->binComplexG, cd->channelComplexSize, scaleFactor));
  CUDA_SAFE_CALL_NO_SYNC(complexMultiply(cd->binComplexB, cd->psfComplex, 
    cd->binComplexB, cd->channelComplexSize, scaleFactor));

#ifdef TIME
  StopWatch::get(ifftTime).reset();
  StopWatch::get(ifftTime).start();
#endif // TIME

  inverseFFT(cd->binComplexR, cd->binR, cd->width, cd->height, cd->inversePlan);
  inverseFFT(cd->binComplexG, cd->binG, cd->width, cd->height, cd->inversePlan);
  inverseFFT(cd->binComplexB, cd->binB, cd->width, cd->height, cd->inversePlan);
  
#ifdef TIME
  StopWatch::get(ifftTime).stop();

  cout << "ifft time: " << StopWatch::get(ifftTime).getTime() << endl;

  //// Merge the results back together.
  StopWatch::get(mergeTime).reset();
  StopWatch::get(mergeTime).start();
#endif // TIME

  CUDA_SAFE_CALL_NO_SYNC(mergePixels(cd->imagePtr, cd->width, cd->height, 
    cd->binR, cd->binG, cd->binB));

#ifdef TIME
  StopWatch::get(mergeTime).stop();
  cout << "merge time: " << StopWatch::get(mergeTime).getTime() << endl;

  StopWatch::get(copy2HostTime).reset();
  StopWatch::get(copy2HostTime).start();
#endif // TIME

  CUDA_SAFE_CALL_NO_SYNC(cudaMemcpy(binHostMem, cd->imagePtr, cd->imageSize,
    cudaMemcpyDeviceToHost));

#ifdef TIME
  StopWatch::get(copy2HostTime).stop();
  cout << "copy2HostTime: " << StopWatch::get(copy2HostTime).getTime() << endl;
#endif // TIME

#ifdef TIME
  StopWatch::get(texDownloadTime).reset();
  StopWatch::get(texDownloadTime).start();
#endif // TIME

  binningTexture->Load(this);
  glTexSubImage2D(vtkgl::TEXTURE_RECTANGLE_ARB, 0, 0, 0, 
    cd->width, cd->height, GL_RGBA, GL_FLOAT, binHostMem);
  binningTexture->PostRender();

#ifdef TIME
  StopWatch::get(texDownloadTime).stop();
  cout << "tex download time: " << StopWatch::get(texDownloadTime).getTime() << endl;
#endif // TIME

  delete[] binHostMem;
  delete[] psfHostMem;

#ifdef TIME
  StopWatch::get(realTotalTime).stop();
  cout << "real totalTime: " << StopWatch::get(realTotalTime).getTime() << endl;

  StopWatch::destroy(realTotalTime);
  StopWatch::destroy(texReadbackTime);
  StopWatch::destroy(mallocTime);
  StopWatch::destroy(copy2DeviceTime);
  StopWatch::destroy(splitTime);
  StopWatch::destroy(fftTime);
  StopWatch::destroy(ifftTime);
  StopWatch::destroy(mergeTime);
  StopWatch::destroy(copy2HostTime);
  StopWatch::destroy(texDownloadTime);
#endif // TIME
}


void vtkBinningFluorescenceRenderer::CreateConvolutionData(ConvolutionData *cd, int width, int height) {
  cd->width = width;
  cd->height = height;
  cd->channelSize        = width * height;
  cd->channelComplexSize = width * ((height/2) + 1);
  cd->imageSize          = sizeof(float4) * cd->channelSize;
  CUDA_SAFE_CALL_NO_SYNC(cudaMalloc((void**)&cd->binR, sizeof(float)*cd->channelSize));
  CUDA_SAFE_CALL_NO_SYNC(cudaMalloc((void**)&cd->binG, sizeof(float)*cd->channelSize));
  CUDA_SAFE_CALL_NO_SYNC(cudaMalloc((void**)&cd->binB, sizeof(float)*cd->channelSize));
  CUDA_SAFE_CALL_NO_SYNC(cudaMalloc((void**)&cd->binComplexR, sizeof(Complex)*cd->channelComplexSize));
  CUDA_SAFE_CALL_NO_SYNC(cudaMalloc((void**)&cd->binComplexG, sizeof(Complex)*cd->channelComplexSize));
  CUDA_SAFE_CALL_NO_SYNC(cudaMalloc((void**)&cd->binComplexB, sizeof(Complex)*cd->channelComplexSize));
  CUDA_SAFE_CALL_NO_SYNC(cudaMalloc((void**)&cd->psfComplex, sizeof(Complex)*cd->channelComplexSize));
  CUDA_SAFE_CALL_NO_SYNC(cudaMalloc((void**)&cd->imagePtr, cd->imageSize));
  CUFFT_SAFE_CALL(cufftPlan2d(&cd->forwardPlan, width, height, CUFFT_R2C));
  CUFFT_SAFE_CALL(cufftPlan2d(&cd->inversePlan, width, height, CUFFT_C2R));
}


void vtkBinningFluorescenceRenderer::DestroyConvolutionData(ConvolutionData *cd) {
  CUDA_SAFE_CALL_NO_SYNC(cudaFree(cd->binR));
  CUDA_SAFE_CALL_NO_SYNC(cudaFree(cd->binG));
  CUDA_SAFE_CALL_NO_SYNC(cudaFree(cd->binB));
  CUDA_SAFE_CALL_NO_SYNC(cudaFree(cd->binComplexR));
  CUDA_SAFE_CALL_NO_SYNC(cudaFree(cd->binComplexG));
  CUDA_SAFE_CALL_NO_SYNC(cudaFree(cd->binComplexB));
  CUDA_SAFE_CALL_NO_SYNC(cudaFree(cd->psfComplex));
  CUDA_SAFE_CALL_NO_SYNC(cudaFree(cd->imagePtr));
  CUFFT_SAFE_CALL(cufftDestroy(cd->forwardPlan));
  CUFFT_SAFE_CALL(cufftDestroy(cd->inversePlan));
}


void vtkBinningFluorescenceRenderer::SetQualityTo16Bit() {
  vtkCollectionSimpleIterator sit;
  vtkFramebufferObjectTexture *currentTexture;
  this->FramebufferTextureCollection->InitTraversal(sit);
  while ((currentTexture = 
    vtkFramebufferObjectTexture::SafeDownCast(this->FramebufferTextureCollection->GetNextItemAsObject(sit)))) {
    currentTexture->SetQualityTo16Bit();
  }
}


void vtkBinningFluorescenceRenderer::SetQualityTo32Bit() {
  vtkCollectionSimpleIterator sit;
  vtkFramebufferObjectTexture *currentTexture;
  this->FramebufferTextureCollection->InitTraversal(sit);
  while ((currentTexture = 
    vtkFramebufferObjectTexture::SafeDownCast(this->FramebufferTextureCollection->GetNextItemAsObject(sit)))) {
    currentTexture->SetQualityTo32Bit();
  }
}


int vtkBinningFluorescenceRenderer::LoadExtensions() {
  if (this->ExtensionsLoaded) {
    return 1;
  }
    
  vtkDebugMacro(<< "Loading extensions");

  vtkOpenGLExtensionManager *manager = vtkOpenGLExtensionManager::New();
  manager->SetRenderWindow(this->GetRenderWindow());

  // Check for acceptable OpenGL versions
  int versionOK = 0;
  versionOK += manager->ExtensionSupported("GL_VERSION_1_2");
  manager->LoadExtension("GL_VERSION_1_2");
  versionOK += manager->ExtensionSupported("GL_VERSION_1_3");
  manager->LoadExtension("GL_VERSION_1_3");
  versionOK += manager->ExtensionSupported("GL_VERSION_1_4");
  manager->LoadExtension("GL_VERSION_1_4");
  versionOK += manager->ExtensionSupported("GL_VERSION_1_5");
  manager->LoadExtension("GL_VERSION_1_5");

  // Acceptable OpenGL version not found.
  if (!versionOK) {
    manager->Delete();
    return 0;
  }

  int supported = 1;
  supported *= manager->ExtensionSupported("GL_ARB_imaging");
  if (supported)
    manager->LoadExtension("GL_ARB_imaging");

  supported *= manager->ExtensionSupported("GL_ARB_occlusion_query");
  if (supported)
    manager->LoadExtension("GL_ARB_occlusion_query");

  // Dispose of the manager.
  manager->Delete();
  this->ExtensionsLoaded = 1;
  return supported;
}


void vtkBinningFluorescenceRenderer::RenderPSFSlice(double *psfSize, double distFromFocalPlane) {
#ifdef TIME
  unsigned int psfInternal = StopWatch::create();
  StopWatch::get(psfInternal).reset();
#endif // TIME

  this->GetFramebufferTexture(PSF_RENDER_TEXTURE)->EnableTarget(this);

  glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT);
  int width = this->GetFramebufferTexture(PSF_RENDER_TEXTURE)->GetTextureWidth();
  int height = this->GetFramebufferTexture(PSF_RENDER_TEXTURE)->GetTextureHeight();

  glViewport(0, 0, width, height);
  glScissor(0, 0, width, height);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, width-1, 0, height-1, 0, 1);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

#ifdef TIME
  StopWatch::get(psfInternal).start();
#endif // TIME

  this->PSFTexture->Render(this);

#ifdef TIME
  StopWatch::get(psfInternal).stop();
#endif // TIME

  float r = static_cast<float>((distFromFocalPlane / psfSize[2]) + 0.5);
  float wf = width - 1.0f;
  float hf = height - 1.0f;

  // Relevant spatial coordinates in the quadrant decomposition of the PSF quad
  float psfPixelsWide = psfSize[0] / this->PixelSize;
  float psfPixelsHigh = psfSize[1] / this->PixelSize;
  float x0 = 0.0f; float x1 = 0.5*psfPixelsWide; float x2 = (1.0*wf) - (0.5*psfPixelsWide); float x3 = 1.0*wf;
  float y0 = 0.0f; float y1 = 0.5*psfPixelsHigh; float y2 = (1.0*hf) - (0.5*psfPixelsHigh); float y3 = 1.0*hf;

  // Texture coordinates for the quadrant decomposition of the PSF quad.
  // Assumes that the x and y dimensions of the PSF are the same.
  float tMin = 0.0f;  float tMed = 0.5f;  float tMax = 1.0f;
  float t0[3] = {tMin, tMin, r}; float t1[3] = {tMed, tMin, r};    float t2[3] = {tMax, tMin, r};
  float t3[3] = {tMin, tMed, r}; float t4[3] = {tMed, tMed, r};    float t5[3] = {tMax, tMed, r};
  float t6[3] = {tMin, tMax, r}; float t7[3] = {tMed, tMax, r};    float t8[3] = {tMax, tMax, r};

  // Account for shift down and to the left induced somewhere above.
  float pixelShift = 0.5f / this->PixelSize;
  //float pixelShift = 0.0f;
  t0[0] -= pixelShift; t0[1] -= pixelShift;
  t1[0] -= pixelShift; t1[1] -= pixelShift;
  t2[0] -= pixelShift; t2[1] -= pixelShift;
  t3[0] -= pixelShift; t3[1] -= pixelShift;
  t4[0] -= pixelShift; t4[1] -= pixelShift;
  t5[0] -= pixelShift; t5[1] -= pixelShift;
  t6[0] -= pixelShift; t6[1] -= pixelShift;
  t7[0] -= pixelShift; t7[1] -= pixelShift;
  t8[0] -= pixelShift; t8[1] -= pixelShift;

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glBegin(GL_QUADS);
  {
    // Quadrant 1
    glTexCoord3fv(t3); glVertex2f(x2, y0);
    glTexCoord3fv(t6); glVertex2f(x2, y1);
    glTexCoord3fv(t7); glVertex2f(x3, y1);
    glTexCoord3fv(t4); glVertex2f(x3, y0);

    // Quadrant 2
    glTexCoord3fv(t4); glVertex2f(x0, y0);
    glTexCoord3fv(t7); glVertex2f(x0, y1);
    glTexCoord3fv(t8); glVertex2f(x1, y1);
    glTexCoord3fv(t5); glVertex2f(x1, y0);

    // Quadrant 3
    glTexCoord3fv(t0); glVertex2f(x2, y2);
    glTexCoord3fv(t3); glVertex2f(x2, y3);
    glTexCoord3fv(t4); glVertex2f(x3, y3);
    glTexCoord3fv(t1); glVertex2f(x3, y2);

    // Quadrant 4
    glTexCoord3fv(t1); glVertex2f(x0, y2);
    glTexCoord3fv(t4); glVertex2f(x0, y3);
    glTexCoord3fv(t5); glVertex2f(x1, y3);
    glTexCoord3fv(t2); glVertex2f(x1, y2);
  }
  glEnd();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glPopAttrib();

  // Make sure to disable the 3D texture and alpha testing
  glDisable(vtkgl::TEXTURE_3D);
  glDisable(GL_ALPHA_TEST);

  this->GetFramebufferTexture(PSF_RENDER_TEXTURE)->DisableTarget();

#ifdef TIME
  cout << "psf render internal: " << StopWatch::get(psfInternal).getTime() << endl;
#endif // TIME
}


void vtkBinningFluorescenceRenderer::LoadPBOs(int width, int height) {
  vtkgl::GLsizeiptr bufferSize = sizeof(GLfloat)*width*height*4;
  if (bufferSize == this->PBOBufferSize)
    return;

  // Need this here to get around a bug in NVIDIA driver
  GLfloat *tmpData = new GLfloat[width*height*4];

  if (this->BinningPBO == -1) {
    vtkgl::GenBuffers(1, &this->BinningPBO);
  }
  vtkgl::BindBuffer(vtkgl::ARRAY_BUFFER, this->BinningPBO);
  vtkgl::BufferData(vtkgl::ARRAY_BUFFER, bufferSize, tmpData, 
    vtkgl::DYNAMIC_DRAW);

  if (this->PSFPBO == -1) {
    vtkgl::GenBuffers(1, &this->PSFPBO);
  }
  vtkgl::BindBuffer(vtkgl::ARRAY_BUFFER, this->PSFPBO);
  vtkgl::BufferData(vtkgl::ARRAY_BUFFER, bufferSize, tmpData, 
    vtkgl::DYNAMIC_DRAW);

  delete[] tmpData;

  this->PBOBufferSize = bufferSize;
}


void vtkBinningFluorescenceRenderer::DeletePBOs() {
  vtkgl::BindBuffer(vtkgl::PIXEL_PACK_BUFFER, this->BinningPBO);
  vtkgl::DeleteBuffers(1, &this->BinningPBO);
  this->BinningPBO = 0;
  vtkgl::BindBuffer(vtkgl::PIXEL_PACK_BUFFER, this->PSFPBO);
  vtkgl::DeleteBuffers(1, &this->PSFPBO);
  this->PSFPBO = 0;
  vtkgl::BindBuffer(vtkgl::PIXEL_PACK_BUFFER, 0);
}


void vtkBinningFluorescenceRenderer::ReportOpenGLErrors() {
  GLenum glError = GL_NO_ERROR;
    
  do {
    glError = glGetError();
    if (glError == GL_NO_ERROR)
      break;

    if (glError == GL_INVALID_ENUM) {
      vtkErrorMacro(<< "OpenGL invalid enum error");
    } else if (glError == GL_INVALID_VALUE) {
      vtkErrorMacro(<< "OpenGL invalid value");
    } else if (glError == GL_INVALID_OPERATION) {
      vtkErrorMacro(<< "OpenGL invalid operation");
    } else if (glError == GL_STACK_OVERFLOW) {
      vtkErrorMacro(<< "OpenGL stack overflow");
    } else if (glError == GL_STACK_UNDERFLOW) {
      vtkErrorMacro(<< "OpenGL stack underflow");
    } else if (glError == GL_OUT_OF_MEMORY) {
      vtkErrorMacro(<< "OpenGL out of memory");
    }
  } while (1);
}


void vtkBinningFluorescenceRenderer::PrintSelf(ostream& os, vtkIndent indent) {
  this->Superclass::PrintSelf(os,indent);
}

