/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkFramebufferObjectTexture.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkFramebufferObjectTexture.h"

#include "vtkImageData.h"
#include "vtkImageImport.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkLookupTable.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGLRenderer.h"
#include "vtkPointData.h"
#include "vtkRenderWindow.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkOpenGLExtensionManager.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkUnsignedShortArray.h"

#include "vtkOpenGL.h"

#include <math.h>
#include <vector>

#ifndef VTK_IMPLEMENT_MESA_CXX
vtkCxxRevisionMacro(vtkFramebufferObjectTexture, "$Revision: 1.12 $");
vtkStandardNewMacro(vtkFramebufferObjectTexture);
#endif

// Initializes an instance, generates a unique index.
vtkFramebufferObjectTexture::vtkFramebufferObjectTexture()
{
  this->Index = 0;

  // See if we need this
  this->RenderWindow = 0;
  this->AutomaticDimensions = 1;
  this->TextureWidth  = -1;
  this->TextureHeight = -1;
  this->TextureTarget = vtkgl::TEXTURE_RECTANGLE_ARB;
  this->TextureFormat = GL_RGBA;
  this->Quality = VTK_TEXTURE_QUALITY_16BIT;
  this->TextureInternalFormat = vtkgl::RGBA16F_ARB;
  this->FilterMode = GL_LINEAR;
  this->RenderBufferHandle = 0;
  this->FrameBufferHandle = 0;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}


vtkFramebufferObjectTexture::~vtkFramebufferObjectTexture()
{
  if (this->RenderWindow)
    this->ReleaseGraphicsResources(this->RenderWindow);
  this->RenderWindow = NULL;
}


void vtkFramebufferObjectTexture::SetTextureWidth(int width) {
  if (this->TextureWidth != width) {
    this->TextureWidth = width;
    this->DimensionTime.Modified();
  }
}


void vtkFramebufferObjectTexture::SetTextureHeight(int height) {
  if (this->TextureHeight != height) {
    this->TextureHeight = height;
    this->DimensionTime.Modified();
  }
}


void vtkFramebufferObjectTexture::SetTextureFormatToLuminance() {
  this->TextureFormat = GL_LUMINANCE;
}


void vtkFramebufferObjectTexture::SetTextureFormatToRGBA() {
  this->TextureFormat = GL_RGBA;
}


// Release the graphics resources used by this texture.
void vtkFramebufferObjectTexture::ReleaseGraphicsResources(vtkWindow *renWin)
{
  if (this->Index && renWin)
    {
    ((vtkRenderWindow *) renWin)->MakeCurrent();

#ifdef GL_VERSION_1_1
    // free any textures
    if (glIsTexture(this->Index))
      {
      GLuint tempIndex;
      tempIndex = this->Index;
      // NOTE: Sun's OpenGL seems to require disabling of texture before delete
      glDisable(GL_TEXTURE_2D);
      glDeleteTextures(1, &tempIndex);
      }
#else
    if (glIsList(this->Index))
      {
      glDeleteLists(this->Index,1);
      }
#endif
    }
  this->Index = 0;
  this->RenderWindow = NULL;
  this->Modified();
}

//----------------------------------------------------------------------------
int vtkFramebufferObjectTexture::LoadExtensions(vtkRenderWindow *renWin) {
  vtkOpenGLRenderWindow *oglRenWin =
    vtkOpenGLRenderWindow::SafeDownCast(renWin);
  if (oglRenWin == NULL)
    return 0;

  vtkOpenGLExtensionManager *manager = oglRenWin->GetExtensionManager();

  std::vector<std::string> extensions;
  extensions.push_back(std::string("GL_EXT_framebuffer_object"));
  extensions.push_back(std::string("GL_ARB_texture_rectangle"));
  extensions.push_back(std::string("GL_ATI_texture_float"));

  // These extensions are required for floating-point rendering.
  int supported = 1;
  for (unsigned int i = 0; i < extensions.size(); i++) {
    const char *ext = extensions[i].c_str();
    supported *= manager->ExtensionSupported(ext);
    if (supported) {
      manager->LoadExtension(ext);
    } else {
      vtkErrorMacro(<< "could not load " << ext);
      break;
    }
  }

  return supported;
}

//----------------------------------------------------------------------------
void vtkFramebufferObjectTexture::EnableTarget(vtkRenderer *renderer)
{
  int *size = renderer->GetSize();
  int needsUpdate = this->AutomaticDimensions ?
    (size[0] != this->TextureWidth || size[1] != this->TextureHeight) :
    (this->DimensionTime.GetMTime() > this->LoadTime.GetMTime());

  if (needsUpdate) {

    // Make sure all the required extensions are loaded.
    this->LoadExtensions(renderer->GetRenderWindow());

    // Clean up the previous frame buffer object.
    this->Clean();

    if (this->AutomaticDimensions) {
      this->TextureWidth  = size[0];
      this->TextureHeight = size[1];
    }

    this->MaxCoordS = (this->TextureTarget == vtkgl::TEXTURE_RECTANGLE_ARB) ? this->TextureWidth : 1;
    this->MaxCoordT = (this->TextureTarget == vtkgl::TEXTURE_RECTANGLE_ARB) ? this->TextureHeight : 1;

    if (this->Quality == VTK_TEXTURE_QUALITY_16BIT) {
      if (this->TextureFormat == GL_RGBA)
        this->TextureInternalFormat = vtkgl::RGBA16F_ARB;
      else if (this->TextureFormat == GL_LUMINANCE)
        this->TextureInternalFormat = vtkgl::LUMINANCE16F_ARB;
      else
        vtkErrorMacro(<< "Unknown TextureFormat value");
    } else {
      if (this->TextureFormat == GL_RGBA)
        this->TextureInternalFormat = vtkgl::RGBA32F_ARB;
      else if (this->TextureFormat == GL_LUMINANCE)
        this->TextureInternalFormat = vtkgl::LUMINANCE32F_ARB;
      else
        vtkErrorMacro(<< "Unknown TextureFormat value");
    }

    vtkgl::GenFramebuffersEXT(1, &this->FrameBufferHandle);
    glGenTextures(1, &this->Index);
    vtkgl::BindFramebufferEXT(vtkgl::FRAMEBUFFER_EXT, this->FrameBufferHandle);
    glBindTexture(this->TextureTarget, this->Index);
    glTexImage2D(this->TextureTarget, 0, this->TextureInternalFormat,
      this->TextureWidth, this->TextureHeight, 0, this->TextureFormat, GL_FLOAT, NULL);

    glTexParameterf(this->TextureTarget, GL_TEXTURE_MIN_FILTER, this->FilterMode);
    glTexParameterf(this->TextureTarget, GL_TEXTURE_MAG_FILTER, this->FilterMode);
    glTexParameterf(this->TextureTarget, GL_TEXTURE_WRAP_S, vtkgl::CLAMP_TO_EDGE);
    glTexParameterf(this->TextureTarget, GL_TEXTURE_WRAP_T, vtkgl::CLAMP_TO_EDGE);

    vtkgl::FramebufferTexture2DEXT(vtkgl::FRAMEBUFFER_EXT, vtkgl::COLOR_ATTACHMENT0_EXT,
                                   this->TextureTarget, this->Index, 0);

    vtkgl::GenRenderbuffersEXT(1, &this->RenderBufferHandle);
    vtkgl::BindRenderbufferEXT(vtkgl::RENDERBUFFER_EXT, this->RenderBufferHandle);
    vtkgl::RenderbufferStorageEXT(vtkgl::RENDERBUFFER_EXT, vtkgl::DEPTH_COMPONENT24,
                                  this->TextureWidth, this->TextureHeight);

    vtkgl::FramebufferRenderbufferEXT(vtkgl::FRAMEBUFFER_EXT, vtkgl::DEPTH_ATTACHMENT_EXT,
                                      vtkgl::RENDERBUFFER_EXT, this->RenderBufferHandle);

    // Probably should check for errors here
    this->CheckFrameBufferStatus();

    this->LoadTime.Modified();
  }

  // Bind this frame buffer in preparation for rendering.
  vtkgl::BindFramebufferEXT(vtkgl::FRAMEBUFFER_EXT, this->FrameBufferHandle);
}

//----------------------------------------------------------------------------
void vtkFramebufferObjectTexture::DisableTarget()
{
  // Set framebuffer target to default framebuffer (the one in the render window).
  vtkgl::BindFramebufferEXT(vtkgl::FRAMEBUFFER_EXT, 0);

  // Mark this texture as modified because we assume that something was
  // written to the target texture.
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkFramebufferObjectTexture::Render(vtkRenderer *ren)
{
  this->Load(ren);
  this->SetRenderWindow(ren->GetRenderWindow());
  this->Modified();
}

// Implement base class method.
void vtkFramebufferObjectTexture::Load(vtkRenderer *ren)
{
  glBindTexture(this->TextureTarget, this->Index);
  glEnable(this->TextureTarget);
}

//----------------------------------------------------------------------------
void vtkFramebufferObjectTexture::PostRender()
{
  glDisable(this->TextureTarget);
}

//----------------------------------------------------------------------------
int vtkFramebufferObjectTexture::RequestInformation (
  vtkInformation * vtkNotUsed(request),
  vtkInformationVector ** vtkNotUsed( inputVector ),
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // set the whole extent
  int wholeExtent[6];
  wholeExtent[0] = 0; wholeExtent[1] = this->TextureWidth - 1;
  wholeExtent[2] = 0; wholeExtent[3] = this->TextureHeight - 1;
  wholeExtent[4] = 0; wholeExtent[5] = 0;
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
               wholeExtent, 6);

  // set data type
  int components = this->TextureFormat == GL_RGBA ? 3 : 1;
  vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_FLOAT, components);

  return 1;
}

//----------------------------------------------------------------------------
int vtkFramebufferObjectTexture::RequestData(vtkInformation* vtkNotUsed(request),
                                             vtkInformationVector** vtkNotUsed(inputVector),
                                             vtkInformationVector* outputVector) {

  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkImageData *output = vtkImageData::SafeDownCast
    (outInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkImageData *data = this->AllocateOutputData(output);

  if (data->GetScalarType() != VTK_FLOAT)
    {
      vtkErrorMacro("RequestData: This source only outputs floats");
    }

  int *outExt = data->GetExtent();
  float *outPtr = static_cast<float *>(data->GetScalarPointer(outExt[0],outExt[2],outExt[4]));

    // Grab the texture data
  if (!this->RenderWindow) {
    vtkErrorMacro(<< "no RenderWindow is set. Results may be unexpected.");
  }

  this->RenderWindow->MakeCurrent();

  glBindTexture(this->TextureTarget, this->Index);

  if (this->TextureFormat == GL_RGBA)
    glGetTexImage(this->TextureTarget, 0, GL_RGB, GL_FLOAT, (GLvoid *) outPtr);
  else if (this->TextureFormat == GL_LUMINANCE)
    glGetTexImage(this->TextureTarget, 0, GL_LUMINANCE, GL_FLOAT, (GLvoid *) outPtr);
  else
    vtkErrorMacro(<< "Uknown TextureFormat encountered");

  return 1;
}


//----------------------------------------------------------------------------
void vtkFramebufferObjectTexture::CheckFrameBufferStatus() {
  GLenum status;
  status = (GLenum) vtkgl::CheckFramebufferStatusEXT(vtkgl::FRAMEBUFFER_EXT);
  switch(status) {
    case vtkgl::FRAMEBUFFER_COMPLETE_EXT:
      break;
    case vtkgl::FRAMEBUFFER_UNSUPPORTED_EXT:
      vtkErrorMacro( << "Unsupported framebuffer format");
      break;
    case vtkgl::FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
      vtkErrorMacro(<< "Framebuffer incomplete, missing attachment");
      break;
    case vtkgl::FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
      vtkErrorMacro(<< "Framebuffer incomplete, attached images must have same dimensions");
      break;
    case vtkgl::FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
      vtkErrorMacro(<< "Framebuffer incomplete, attached images must have same format");
      break;
    case vtkgl::FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
      vtkErrorMacro(<< "Framebuffer incomplete, missing draw buffer");
      break;
    case vtkgl::FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
      vtkErrorMacro(<< "Framebuffer incomplete, missing read buffer");
      break;
    default:
      break;
  }
}

//----------------------------------------------------------------------------
void vtkFramebufferObjectTexture::Clean() {
  glDeleteTextures(1, &(this->Index));
  vtkgl::DeleteFramebuffersEXT(1, &(this->FrameBufferHandle));
}

//----------------------------------------------------------------------------
void vtkFramebufferObjectTexture::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Index: " << this->Index << endl;
}
