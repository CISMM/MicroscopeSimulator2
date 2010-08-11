/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkFluorescencePointsGradientPolyDataMapper.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkFluorescencePointsGradientPolyDataMapper.h"

#include "vtkFloatArray.h"
#include "vtkFluorescencePointsGradientRenderer.h"
#include "vtkFramebufferObjectTexture.h"
#include "vtkImageData.h"
#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkOpenGL3DTexture.h"
#include "vtkOpenGLExtensionManager.h"
#include "vtkOpenGLTexture.h"
#include "vtkPointData.h"

#include <float.h>
#include <vector>


vtkCxxRevisionMacro(vtkFluorescencePointsGradientPolyDataMapper, "$Revision: 1.9 $");
vtkStandardNewMacro(vtkFluorescencePointsGradientPolyDataMapper);


// Construct empty object.
vtkFluorescencePointsGradientPolyDataMapper::vtkFluorescencePointsGradientPolyDataMapper() {
  this->ListId = 0;
  this->Renderer = NULL;

  this->SetSimulatedRegion(0, 1, 0, 1);
  this->FocalPlaneDepth = 0.0;
  this->ExtensionsLoaded = 0;
  this->PtsLastTimePointsModified = 0;

  // Set up texture memory for storing point data.
  this->PointTextureID = 0;
  this->PointTextureDimension = 0;
  this->PointTextureTarget = vtkgl::TEXTURE_RECTANGLE_ARB;

  this->ShaderProgramHandle = 0;
  this->VertexProgramHandle = 0;
  this->FragmentProgramHandle = 0;

  this->TextureTarget = vtkFramebufferObjectTexture::New();
  this->TextureTarget->AutomaticDimensionsOff();

  // Set default number of pixels to process per pass.
  this->PixelsPerPass = 3000;
  
  // By default, we'll clear the texture target
  this->ClearTextureTarget = 1;
}


// Destructor (don't call ReleaseGraphicsResources() because it is virtual)
vtkFluorescencePointsGradientPolyDataMapper::~vtkFluorescencePointsGradientPolyDataMapper() {
}


float* vtkFluorescencePointsGradientPolyDataMapper::GetPointsGradient(int& numPoints) {
  numPoints = this->GetInput()->GetPoints()->GetNumberOfPoints();
  this->TextureTarget->Update();
  this->TextureTarget->GetOutput()->Update();
  vtkFloatArray* gradientArray = vtkFloatArray::SafeDownCast
    (this->TextureTarget->GetOutput()->GetPointData()->GetScalars());

  float* gradient = gradientArray->GetPointer(0);

 
  vtkPointData* pd = this->GetInput()->GetPointData();
  std::cout << "Number of arrays: " << pd->GetNumberOfArrays() << std::endl;
  for (int i = 0; i < pd->GetNumberOfArrays(); i++) {
    std::cout << pd->GetArrayName(i) << std::endl;
  }

  return gradient;
}


vtkPolyData* vtkFluorescencePointsGradientPolyDataMapper::GetPointsGradient() {
  // Get the gradient data from the texture target
  this->TextureTarget->Update();
  this->TextureTarget->GetOutput()->Update();
  vtkSmartPointer<vtkFloatArray> gradientArray = 
    vtkSmartPointer<vtkFloatArray>::New();
  gradientArray->DeepCopy(this->TextureTarget->GetOutput()->GetPointData()->GetScalars());
  gradientArray->SetName("Gradient");

  // The variable gradientData holds the sample points used for the gradient
  // computation as well as the gradient values themselves. The gradient
  // values are stored as an array named "Gradient".
  vtkPolyData* gradientData = vtkPolyData::New();
  gradientData->DeepCopy(this->GetInput());
  vtkPointData* pointData = gradientData->GetPointData();
  pointData->AddArray(gradientArray);

  return gradientData;
}


// Release the graphics resources used by this mapper.  In this case, release
// the display list if any as well as the texture holding the point positions.
void vtkFluorescencePointsGradientPolyDataMapper::ReleaseGraphicsResources(vtkWindow *win) {
  if (win) {
    win->MakeCurrent();
  }
  
  if (this->ListId && win) {
    glDeleteLists(this->ListId,1);
    this->ListId = 0;
  }
  if (this->PointTextureID && win) {
    this->DeletePointTexture();
  }
  this->LastWindow = NULL;

  vtkgl::DeleteShader(this->VertexProgramHandle);
  this->VertexProgramHandle = 0;

  vtkgl::DeleteShader(this->FragmentProgramHandle);
  this->FragmentProgramHandle = 0;

  vtkgl::DeleteProgram(this->ShaderProgramHandle);
  this->ShaderProgramHandle = 0;
}


// Renders the points
void vtkFluorescencePointsGradientPolyDataMapper::RenderPoints(vtkActor *actor, vtkRenderer* renderer) {
  if (!PSFTexture) {
    vtkErrorMacro(<< "PSFTexture not set");
    return;
  }

  vtkFluorescencePointsGradientRenderer* gradientRenderer =
    vtkFluorescencePointsGradientRenderer::SafeDownCast(renderer);
  if (!gradientRenderer) {
    vtkErrorMacro(<< "A vtkFluorescencePointsGradientRenderer is required");
    return;
  }

  if (!this->ShaderProgramHandle) {
    vtkErrorMacro(<< "ShaderProgramHandle invalid");
    return;
  }

  vtkgl::ActiveTexture(vtkgl::TEXTURE0);
  this->PSFTexture->Render(renderer);

  // We'll use the texture matrix to store the actor transformation
  vtkgl::ActiveTexture(vtkgl::TEXTURE1);
  glMatrixMode(GL_TEXTURE);
  glPushMatrix();
  glLoadIdentity();
  vtkMatrix4x4 *matrix = actor->GetMatrix();
  this->RenderMatrix(matrix);

  // Assign the point texture to TEXUNIT1
  this->LoadPointTexture();

  glBindTexture(this->PointTextureTarget, (GLuint) this->PointTextureID);
  glEnable(this->PointTextureTarget);
  vtkgl::ActiveTexture(vtkgl::TEXTURE0);

  this->TextureTarget->SetTextureWidth(this->PointTextureDimension);
  this->TextureTarget->SetTextureHeight(this->PointTextureDimension);
  this->TextureTarget->EnableTarget(renderer);
  
  // Disable a few things.
  glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT | GL_ENABLE_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_LIGHTING);

  // Turn on blending. Hope it's floating-point. If floating-point blending
  // is not available, we'll have inaccurate gradients.
  glEnable(GL_BLEND);
  vtkgl::BlendEquation(vtkgl::FUNC_ADD);
  glBlendFunc(GL_ONE, GL_ONE);

  // Set up window
  int winW = this->PointTextureDimension;
  int winH = winW;
  glViewport(0, 0, winW, winH);

  if (this->ClearTextureTarget) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  glOrtho(0, winW, 0, winH, 0, 1);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  this->EnableShaderProgram(vtkFluorescencePointsGradientRenderer::SafeDownCast(renderer));

  vtkOpenGL3DTexture* expTexture = gradientRenderer->GetExperimentalImageTexture();

  if (this->PSFTexture && this->PSFTexture->GetInput() && expTexture) {

    // Outer loop required because fragment programs are limited to 
    // 2^16 instructions on old hardware. We'll generally have far more pixels
    // than 2^16, so we need to draw in several passes, gathering from a 
    // different set of pixels in each one.
    int* psfDimensions = this->PSFTexture->GetInput()->GetDimensions();
    int rowsPerPass = this->PixelsPerPass / psfDimensions[0];
    for (int startRow = 0; startRow < psfDimensions[1]; startRow += rowsPerPass) {
      int endRow = startRow + rowsPerPass;
      if (endRow > psfDimensions[1])
        endRow = psfDimensions[1];
      
      this->SetUniform1i("startRow", startRow);
      this->SetUniform1i("endRow", endRow);

      GLfloat maxCoord = (GLfloat) this->PointTextureDimension;
      glBegin(GL_QUADS);
      glVertex2f(0.0, 0.0);
      glVertex2f(0.0, maxCoord);
      glVertex2f(maxCoord, maxCoord);
      glVertex2f(maxCoord, 0.0);
      glEnd();
    }
  }

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  vtkgl::ActiveTexture(vtkgl::TEXTURE1);
  glMatrixMode(GL_TEXTURE);
  glPopMatrix();
  vtkgl::ActiveTexture(vtkgl::TEXTURE0);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  // Turn blending back off
  glPopAttrib();

  vtkgl::ActiveTexture(vtkgl::TEXTURE1);
  glDisable(this->PointTextureTarget);

  // Switch back to texture unit 0
  vtkgl::ActiveTexture(vtkgl::TEXTURE0);

  this->PSFTexture->PostRender(renderer);

  this->DisableShaderProgram();

  this->TextureTarget->DisableTarget();
  this->TextureTarget->Modified();
  this->TextureTarget->SetRenderWindow(renderer->GetRenderWindow());
  this->TextureTarget->UpdateWholeExtent();


#if 0 // THIS CODE IS TEMPORARY TO DISPLAY THE GRADIENT TEXTURE
  this->TextureTarget->Render(renderer);

  glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT | GL_ENABLE_BIT);
  int width = this->TextureTarget->GetTextureWidth();
  int height = this->TextureTarget->GetTextureHeight();

  glDisable(GL_ALPHA_TEST);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_LIGHTING);

  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, width, 0, height, 0, 1);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  int s = this->TextureTarget->GetMaxCoordS();
  int t = this->TextureTarget->GetMaxCoordT();
  int w = this->TextureTarget->GetTextureWidth();
  int h = this->TextureTarget->GetTextureHeight();

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glBegin(GL_QUADS);
  {
    glTexCoord2f(0, 0);   glVertex2f(0, 0);
    glTexCoord2f(s, 0);   glVertex2f(w, 0);
    glTexCoord2f(s, t);   glVertex2f(w, h);
    glTexCoord2f(0, t);   glVertex2f(0, h);
  }
  glEnd();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glPopAttrib();

  this->TextureTarget->PostRender();
#endif

#if 0
  for (int j = 0; j < this->PointTextureDimension; j++) {
    for (int i = 0; i < this->PointTextureDimension; i++) {
      std::cout << "Gradient value: (" << 
        i << ", " << j << ") " <<
        this->TextureTarget->GetOutput()->GetScalarComponentAsDouble(i, j, 0, 0) << ", " <<
        this->TextureTarget->GetOutput()->GetScalarComponentAsDouble(i, j, 0, 1) << ", " <<
        this->TextureTarget->GetOutput()->GetScalarComponentAsDouble(i, j, 0, 2) << std::endl;
    }
  }
#endif


}

// Send matrix to OpenGL.
void vtkFluorescencePointsGradientPolyDataMapper::RenderMatrix(vtkMatrix4x4 *matrix) {
  double *mat = matrix->Element[0];
  double mat2[16];
  // Transpose the matrix to the format expected by OpenGL.
  mat2[ 0] = mat[0];  mat2[ 1] = mat[4];  mat2[ 2] = mat[ 8];  mat2[ 3] = mat[12];  
  mat2[ 4] = mat[1];  mat2[ 5] = mat[5];  mat2[ 6] = mat[ 9];  mat2[ 7] = mat[13];
  mat2[ 8] = mat[2];  mat2[ 9] = mat[6];  mat2[10] = mat[10];  mat2[11] = mat[14];
  mat2[12] = mat[3];  mat2[13] = mat[7];  mat2[14] = mat[11];  mat2[15] = mat[15];
  glMultMatrixd(mat2);
}

// Draw method for OpenGL.
int vtkFluorescencePointsGradientPolyDataMapper::Draw(vtkRenderer *aren, vtkActor *act) {
  this->LoadExtensions(aren->GetRenderWindow());
  this->LoadShaderProgram(aren->GetRenderWindow());

  this->RenderPoints(act, aren);
  this->UpdateProgress(1.0);
  return 1;
}

// Loads the point texture.
void vtkFluorescencePointsGradientPolyDataMapper::LoadPointTexture() {
  if (!this->ExtensionsLoaded) {
    return;
  }

  int ptsModifiedTime = this->GetInput()->GetPoints()->GetMTime();
  if (ptsModifiedTime != this->PtsLastTimePointsModified ||
    this->PointTextureID == 0) {
    this->DeletePointTexture();

    vtkPoints *points = this->GetInput()->GetPoints();
    int numPoints = points->GetNumberOfPoints();
    this->PointTextureDimension = ceil(sqrt((double) numPoints));

    // Copy and cast point locations to texture memory.
    int pointDataLength = this->PointTextureDimension * this->PointTextureDimension;
    GLfloat *pointsData = new GLfloat[pointDataLength * 3];
    for (int i = 0; i < numPoints; i++) {
      double *tmp = points->GetPoint(i);
      pointsData[i*3 + 0] = (GLfloat) tmp[0];
      pointsData[i*3 + 1] = (GLfloat) tmp[1];
      pointsData[i*3 + 2] = (GLfloat) tmp[2];
    }
    for (int i = numPoints; i < pointDataLength; i++) {
      pointsData[i*3 + 0] = 1.0f;
      pointsData[i*3 + 1] = 0.0f;
      pointsData[i*3 + 2] = 0.0f;
    }

    this->PtsLastTimePointsModified = ptsModifiedTime;

    // Update point data in texture map
    glGenTextures(1, (GLuint *) &this->PointTextureID);

    glBindTexture(this->PointTextureTarget, (GLuint) this->PointTextureID);
    glTexParameteri(this->PointTextureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(this->PointTextureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(this->PointTextureTarget, GL_TEXTURE_WRAP_S, vtkgl::CLAMP_TO_EDGE);
    glTexParameteri(this->PointTextureTarget, GL_TEXTURE_WRAP_T, vtkgl::CLAMP_TO_EDGE);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexImage2D(this->PointTextureTarget, 0, vtkgl::RGB_FLOAT32_ATI, this->PointTextureDimension, 
      this->PointTextureDimension, 0, GL_RGB, GL_FLOAT, pointsData);

    delete[] pointsData;

    // Now set up the texture target
    // TODO - may need to change scissor buffer extent
    this->TextureTarget->SetTextureWidth(this->PointTextureDimension);
    this->TextureTarget->SetTextureHeight(this->PointTextureDimension);
  }
}

// Deletes the point texture.
void vtkFluorescencePointsGradientPolyDataMapper::DeletePointTexture() {
  if (this->PointTextureID != 0) {
    glDeleteTextures(1, (GLuint *) &this->PointTextureID);
    this->PointTextureID = 0;
  }
}


int vtkFluorescencePointsGradientPolyDataMapper::EnableShaderProgram(vtkFluorescencePointsGradientRenderer* renderer) {  
  if (renderer && this->PSFTexture && this->PSFTexture->GetInput()) {
    vtkImageData* psfImage = this->PSFTexture->GetInput();
    int *psfDims = psfImage->GetDimensions();
    double psfSpacing[3], psfOrigin[3], psfMaxTexCoords[3], psfScale[3];
    psfImage->GetSpacing(psfSpacing);
    psfImage->GetOrigin(psfOrigin);
    this->PSFTexture->GetMaxTextureCoordinates(psfMaxTexCoords);
    for (int i = 0; i < 3; i++) {
      // I have defined the origin in VTK so that it falls in the center of
      // the voxel at index (0,0,0). To translate into texture coordinates,
      // it is more convenient to have the origin defined at the corner of the
      // voxel at (0,0,0). Do this by translating the origin by minus 
      // one-half pixel.
      psfOrigin[i] -= 0.5 * psfSpacing[i];
      psfScale[i] = psfMaxTexCoords[i] / (psfDims[i] * psfSpacing[i]);
    }

    vtkImageData* experimentalImage = renderer->GetExperimentalImageTexture()->GetInput();
    int *expDims = experimentalImage->GetDimensions();
    double expSpacing[3], expOrigin[3], expMaxTexCoords[3], expScale[3];
    experimentalImage->GetSpacing(expSpacing);
    experimentalImage->GetOrigin(expOrigin);
    renderer->GetExperimentalImageTexture()->
      GetMaxTextureCoordinates(expMaxTexCoords);
    for (int i = 0; i < 3; i++) {
      // See the comment above for psfOrigin
      expOrigin[i] -= 0.5 * expSpacing[i];
      expScale[i] = expMaxTexCoords[i] / (expDims[i] * expSpacing[i]);
    }

    double synthOrigin[3] = {0.0, 0.0, 0.0};
    double synthScale[3];
    synthScale[0] = 1.0 / this->GetPixelSize()[0];
    synthScale[1] = 1.0 / this->GetPixelSize()[1];
    synthScale[2] = 1.0;
    double synthMaxTexCoords[3];
    synthMaxTexCoords[0] = renderer->GetSize()[0];
    synthMaxTexCoords[1] = renderer->GetSize()[1];
    synthMaxTexCoords[2] = 1.0;

    vtkgl::UseProgram(this->ShaderProgramHandle);

    // Set shader parameters.      
    this->SetUniform1i("psfRowLength", psfDims[0]);
    this->SetUniform1i("psfSampler", 0);
    this->SetUniform1i("ptsSampler", 1);
    this->SetUniform1i("syntheticImageSampler", 2);
    this->SetUniform1i("experimentalImageSampler", 3);
    this->SetUniform1i ("pointTexDim", this->PointTextureDimension);
    this->SetUniform3dv("psfOrigin", psfOrigin);
    this->SetUniform3dv("psfScale", psfScale);
    this->SetUniform3dv("psfSpacing", psfSpacing);
    this->SetUniform3dv("psfMaxTexCoords", psfMaxTexCoords);
    this->SetUniform3dv("expOrigin", expOrigin);
    this->SetUniform3dv("expScale", expScale);
    this->SetUniform3dv("expMaxTexCoords", expMaxTexCoords);
    this->SetUniform3dv("synthOrigin", synthOrigin);
    this->SetUniform3dv("synthScale", synthScale);
    this->SetUniform3dv("synthMaxTexCoords", synthMaxTexCoords);
    this->SetUniform1f("focalDepth", this->FocalPlaneDepth);
    this->SetUniform1f("gain", this->Gain);
    this->SetUniform1f("shearInX", this->Shear[0]);
    this->SetUniform1f("shearInY", this->Shear[1]);
    
    return 1;
  }

  return 0;
}


void vtkFluorescencePointsGradientPolyDataMapper::DisableShaderProgram() {
  vtkgl::UseProgram(0);
}


void vtkFluorescencePointsGradientPolyDataMapper::SetUniform1i(const char* name, int value) {
  GLint handle = vtkgl::GetUniformLocation(this->ShaderProgramHandle, name);
  vtkgl::Uniform1i(handle, value);
}


void vtkFluorescencePointsGradientPolyDataMapper::SetUniform1f(const char* name, float value) {
  GLint handle = vtkgl::GetUniformLocation(this->ShaderProgramHandle, name);
  vtkgl::Uniform1f(handle, value);
}


void vtkFluorescencePointsGradientPolyDataMapper::SetUniform3dv(const char* name, double* value) {
  GLint handle = vtkgl::GetUniformLocation(this->ShaderProgramHandle, name);
  vtkgl::Uniform3f(handle, (float) value[0], (float) value[1], (float) value[2]);
}


// Calls the superclass update method.
void vtkFluorescencePointsGradientPolyDataMapper::Update() {
  this->Superclass::Update();
}


int vtkFluorescencePointsGradientPolyDataMapper::LoadExtensions(vtkRenderWindow *renWin) {
  if (this->ExtensionsLoaded)
    return 1;

  vtkOpenGLExtensionManager *manager = vtkOpenGLExtensionManager::New();
  manager->SetRenderWindow(renWin);

  std::vector<std::string> versions;
  versions.push_back("GL_VERSION_1_1");
  versions.push_back("GL_VERSION_1_2");
  versions.push_back("GL_VERSION_1_3");
  versions.push_back("GL_VERSION_1_4");
  versions.push_back("GL_VERSION_2_0");
  
  for (unsigned int i = 0; i < versions.size(); i++) {
    manager->LoadSupportedExtension(versions[i].c_str());
  }

  std::vector<std::string> extensions;
  extensions.push_back("GL_ARB_texture_rectangle");
  extensions.push_back("GL_ARB_imaging");

  for (unsigned int i = 0; i < extensions.size(); i++) {
    int supported = manager->LoadSupportedExtension(extensions[i].c_str());
    if (!supported) {
      vtkErrorMacro(<< extensions[i] << " not supported on your system!");
      return 0;
    }
  }

  this->ExtensionsLoaded = 1;

  // Dispose of the manager.
  manager->Delete();
  return 1;
}


int vtkFluorescencePointsGradientPolyDataMapper::LoadShaderProgram(vtkRenderWindow *renWin) {
  if (this->ShaderProgramHandle)
    return 1;

  if (!this->LoadExtensions(renWin))
    return 0;

  std::string gradientVertexProgram =
#include "vtkFluorescencePointsGradientPolyDataMapper_vertex.glsl"
	  ;

  std::string gradientFragmentProgram =
#include "vtkFluorescencePointsGradientPolyDataMapper_fragment.glsl"
	  ;

  this->VertexProgramHandle = vtkgl::CreateShader(vtkgl::VERTEX_SHADER);
  const vtkgl::GLchar* vertexProgram = gradientVertexProgram.c_str();
  vtkgl::ShaderSource(this->VertexProgramHandle, 1, &vertexProgram, NULL);
  vtkgl::CompileShader(this->VertexProgramHandle);

  this->FragmentProgramHandle = vtkgl::CreateShader(vtkgl::FRAGMENT_SHADER);
  const vtkgl::GLchar* fragmentProgram = gradientFragmentProgram.c_str();
  vtkgl::ShaderSource(this->FragmentProgramHandle, 1, &fragmentProgram, NULL);
  vtkgl::CompileShader(this->FragmentProgramHandle);

  // Check for errors
  GLint status;
  char infoLog[256];
  GLint length;
  vtkgl::GetShaderiv(this->VertexProgramHandle, vtkgl::COMPILE_STATUS, &status);
  if (!status) {
    vtkErrorMacro(<< "Failed to compile vertex program");
    vtkgl::GetShaderInfoLog(this->VertexProgramHandle, 256, &length, infoLog);
    vtkErrorMacro(<< infoLog);
    return 0;
  }
  vtkgl::GetShaderiv(this->FragmentProgramHandle, vtkgl::COMPILE_STATUS, &status);
  if (!status) {
    vtkErrorMacro(<< "Failed to compile fragment program");
    vtkgl::GetShaderInfoLog(this->FragmentProgramHandle, 256, &length, infoLog);
    vtkErrorMacro(<< infoLog);
    return 0;
  }

  this->ShaderProgramHandle = vtkgl::CreateProgram();
  vtkgl::AttachShader(this->ShaderProgramHandle, this->VertexProgramHandle);
  vtkgl::AttachShader(this->ShaderProgramHandle, this->FragmentProgramHandle);

  vtkgl::LinkProgram(this->ShaderProgramHandle);

  vtkgl::GetProgramiv(this->ShaderProgramHandle, vtkgl::LINK_STATUS, &status);
  if (!status) {
    vtkGraphicErrorMacro(renWin, "Failed to link shader program");
    vtkgl::GetProgramInfoLog(this->ShaderProgramHandle, 256, &length, infoLog);
    vtkGraphicErrorMacro(renWin, infoLog);
  }

  return 1;
}


void vtkFluorescencePointsGradientPolyDataMapper::PrintSelf(ostream& os, vtkIndent indent) {
  this->Superclass::PrintSelf(os,indent);
}
