/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkFluorescenceRenderer.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkFluorescenceRenderer.h"

#include <vector>

#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkOpenGLExtensionManager.h"

#ifndef VTK_IMPLEMENT_MESA_CXX
# include "vtkOpenGL.h"
#endif

#include <math.h>

#ifndef VTK_IMPLEMENT_MESA_CXX
vtkCxxRevisionMacro(vtkFluorescenceRenderer, "$Revision: 1.2 $");
vtkStandardNewMacro(vtkFluorescenceRenderer);
#endif


vtkFluorescenceRenderer::vtkFluorescenceRenderer() {
  this->GenerateNoise = 0;
  this->ScrambleKey   = 0;
  this->NoiseStdDev   = 0.0;
  this->NoiseMean     = 0.0;
  this->NoiseProgramHandle = 0;
  this->NoiseFragmentShaderHandle = 0;
  this->BackgroundIntensityProgramHandle = 0;
  this->BackgroundIntensityFragmentShaderHandle = 0;
  this->BackgroundIntensity = 0.0;
}


vtkFluorescenceRenderer::~vtkFluorescenceRenderer() {
  vtkgl::DeleteShader(this->NoiseFragmentShaderHandle);
  vtkgl::DeleteProgram(this->NoiseProgramHandle);
  vtkgl::DeleteShader(this->BackgroundIntensityFragmentShaderHandle);
  vtkgl::DeleteShader(this->BackgroundIntensityProgramHandle);
}


void vtkFluorescenceRenderer::DeviceRender(void) {
  // Do not remove this MakeCurrent! Due to Start / End methods on
  // some objects which get executed during a pipeline update,
  // other windows might get rendered since the last time
  // a MakeCurrent was called.
  this->RenderWindow->MakeCurrent();

  this->ResetTimer();
  this->StartTimer();

  // Set the framebuffer texture as the rendering target.
  this->GetActiveFramebufferTexture()->EnableTarget(this);

  // Standard render method.
  this->Superclass::Superclass::DeviceRender();

  if (this->GenerateNoise) {
    // Turn on blending and add noise.
    glEnable(GL_BLEND);
    vtkgl::BlendEquation(vtkgl::FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    this->RenderNoise();

    glDisable(GL_BLEND);
  }

  // Add the background intensity through floating-point blending
  RenderBackgroundIntensity();

  // Now that rendering is finished, disable the framebuffer texture as the rendering target.
  this->GetActiveFramebufferTexture()->DisableTarget();

  // Now remap the framebuffer texture to the display range of
  // the display.
  this->RemapAndDisplay();

  this->StopTimer();
  this->PrintTime();
}


void vtkFluorescenceRenderer::LoadNoiseFragmentProgram() {
  if (this->NoiseProgramHandle)
    return;

  // Load GLSL shader extensions
  char glVersion2_0[] = "GL_VERSION_2_0";
  char textureRect[]  = "GL_ARB_texture_rectangle";
  vtkOpenGLExtensionManager *manager = vtkOpenGLExtensionManager::New();
  if (manager->ExtensionSupported(glVersion2_0))
    manager->LoadExtension(glVersion2_0);
  else
    vtkErrorMacro(<< "Your graphics card does not support the extension '" << glVersion2_0 << "'");

  if (manager->ExtensionSupported(textureRect))
    manager->LoadExtension(textureRect);
  else
    vtkErrorMacro(<< "Your graphics card does not support the extension '" << textureRect << "'");

  this->NoiseProgramHandle = vtkgl::CreateProgram();

  std::string programString =
#include "GaussianNoise_fragment.glsl"
	  ;
  const vtkgl::GLchar *programPointer = programString.c_str();

  this->NoiseFragmentShaderHandle = vtkgl::CreateShader(vtkgl::FRAGMENT_SHADER);
  vtkgl::AttachShader(this->NoiseProgramHandle, this->NoiseFragmentShaderHandle);
  vtkgl::ShaderSource(this->NoiseFragmentShaderHandle, 1, &programPointer, NULL);
  vtkgl::CompileShader(this->NoiseFragmentShaderHandle);

  /* Check for errors. */
  GLint status;
  char infoLog[256];
  GLint length;
  vtkgl::GetShaderiv(this->NoiseFragmentShaderHandle, vtkgl::COMPILE_STATUS, &status);
  if (!status) {
    vtkErrorMacro(<< "Failed to compile fragment program");
    vtkgl::GetShaderInfoLog(this->NoiseFragmentShaderHandle, 256, &length, infoLog);
    vtkErrorMacro(<< infoLog);
  }

  vtkgl::LinkProgram(this->NoiseProgramHandle);

  /* Check for errors. */
  vtkgl::GetProgramiv(this->NoiseProgramHandle, vtkgl::LINK_STATUS, &status);
  if (!status) {
    vtkErrorMacro(<< "Failed to link fragment program");
    vtkgl::GetProgramInfoLog(this->NoiseProgramHandle, 256, &length, infoLog);
    vtkErrorMacro(<< infoLog);
  }
}


void vtkFluorescenceRenderer::LoadBackgroundIntensityProgram(void) {
  if (this->BackgroundIntensityProgramHandle)
    return;

  // Load GLSL shader extensions
  char glVersion2_0[] = "GL_VERSION_2_0";
  char textureRect[]  = "GL_ARB_texture_rectangle";
  vtkOpenGLExtensionManager *manager = vtkOpenGLExtensionManager::New();
  manager->SetRenderWindow(this->RenderWindow);
  if (manager->ExtensionSupported(glVersion2_0))
    manager->LoadExtension(glVersion2_0);
  else
    vtkErrorMacro(<< "Your graphics card does not support the extension '" << glVersion2_0 << "'");

  if (manager->ExtensionSupported(textureRect))
    manager->LoadExtension(textureRect);
  else
    vtkErrorMacro(<< "Your graphics card does not support the extension '" << textureRect << "'");

  this->BackgroundIntensityProgramHandle = vtkgl::CreateProgram();

  std::string programString =
#include "BackgroundIntensity_fragment.glsl"
	  ;
  const vtkgl::GLchar *programPointer = programString.c_str();

  this->BackgroundIntensityFragmentShaderHandle = vtkgl::CreateShader(vtkgl::FRAGMENT_SHADER);
  vtkgl::AttachShader(this->BackgroundIntensityProgramHandle, this->BackgroundIntensityFragmentShaderHandle);
  vtkgl::ShaderSource(this->BackgroundIntensityFragmentShaderHandle, 1, &programPointer, NULL);
  vtkgl::CompileShader(this->BackgroundIntensityFragmentShaderHandle);

  /* Check for errors. */
  GLint status;
  char infoLog[256];
  GLint length;
  vtkgl::GetShaderiv(this->BackgroundIntensityFragmentShaderHandle, vtkgl::COMPILE_STATUS, &status);
  if (!status) {
    vtkErrorMacro(<< "Failed to compile fragment program");
    vtkgl::GetShaderInfoLog(this->BackgroundIntensityFragmentShaderHandle, 256, &length, infoLog);
    vtkErrorMacro(<< infoLog);
  }

  vtkgl::LinkProgram(this->BackgroundIntensityProgramHandle);

  /* Check for errors. */
  vtkgl::GetProgramiv(this->BackgroundIntensityProgramHandle, vtkgl::LINK_STATUS, &status);
  if (!status) {
    vtkErrorMacro(<< "Failed to link fragment program");
    vtkgl::GetProgramInfoLog(this->BackgroundIntensityProgramHandle, 256, &length, infoLog);
    vtkErrorMacro(<< infoLog);
  }
}


void vtkFluorescenceRenderer::RenderNoise() {
  this->RenderNoise(this->ActiveFramebufferTextureIndex);
}


void vtkFluorescenceRenderer::RenderNoise(int index) {
  this->LoadNoiseFragmentProgram();

  glPushAttrib(GL_VIEWPORT_BIT);
  int width = this->GetFramebufferTexture(index)->GetTextureWidth();
  int height = this->GetFramebufferTexture(index)->GetTextureHeight();

  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, width, 0, height, 0, 1);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // Bind noise fragment program
  vtkgl::UseProgram(this->NoiseProgramHandle);

  // Send settings over to fragment program
  GLint scrambleKey = static_cast<GLint>(this->ScrambleKey++);
  GLint scrambleKeyHandle = vtkgl::GetUniformLocation(this->NoiseProgramHandle, "scrambleKey");
  vtkgl::Uniform1i(scrambleKeyHandle, scrambleKey);

  GLfloat stdDev = static_cast<float>(this->NoiseStdDev);
  GLint stdDevHandle = vtkgl::GetUniformLocation(this->NoiseProgramHandle, "stdDev");
  vtkgl::Uniform1f(stdDevHandle, stdDev);

  GLfloat mean = static_cast<float>(this->NoiseMean);
  GLint meanHandle = vtkgl::GetUniformLocation(this->NoiseProgramHandle, "mean");
  vtkgl::Uniform1f(meanHandle, mean);

  // Make sure to disable any 3D textures
  glDisable(vtkgl::TEXTURE_3D);

  int s = this->GetFramebufferTexture(index)->GetMaxCoordS();
  int t = this->GetFramebufferTexture(index)->GetMaxCoordT();
  int w = this->GetFramebufferTexture(index)->GetTextureWidth();
  int h = this->GetFramebufferTexture(index)->GetTextureHeight();

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glBegin(GL_QUADS);
  {
    glTexCoord2f(0, 0);   glVertex2f(0, 0);
    glTexCoord2f(s, 0);   glVertex2f(w, 0);
    glTexCoord2f(s, t);   glVertex2f(w, h);
    glTexCoord2f(0, t);   glVertex2f(0, h);
  }
  glEnd();

  // Disable noise fragment program
  vtkgl::UseProgram(0);

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glPopAttrib();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
}


void vtkFluorescenceRenderer::RenderBackgroundIntensity() {
  this->LoadBackgroundIntensityProgram();

  glEnable(GL_BLEND);
  vtkgl::BlendEquation(vtkgl::FUNC_ADD);
  glBlendFunc(GL_ONE, GL_ONE);

  glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT);

  glDisable(GL_LIGHTING);

  int width  = this->GetFramebufferTexture()->GetTextureWidth();
  int height = this->GetFramebufferTexture()->GetTextureHeight();
  int w = this->GetFramebufferTexture()->GetTextureWidth();
  int h = this->GetFramebufferTexture()->GetTextureHeight();

  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, width, 0, height, 0, 1);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // Bind noise fragment program
  vtkgl::UseProgram(this->BackgroundIntensityProgramHandle);

  // Send background intensity to the fragment program
  GLint intensityHandle = vtkgl::GetUniformLocation
    (this->BackgroundIntensityProgramHandle, "intensity");
  vtkgl::Uniform1f(intensityHandle, this->BackgroundIntensity);

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glBegin(GL_QUADS);
  {
    double bg = this->BackgroundIntensity;
    glColor3d(bg, bg, bg);
    glVertex2f(0, 0);
    glVertex2f(w, 0);
    glVertex2f(w, h);
    glVertex2f(0, h);
  }
  glEnd();

  // Disable the background intensity program
  vtkgl::UseProgram(0);

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glPopAttrib();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glDisable(GL_BLEND);
}


void vtkFluorescenceRenderer::PrintSelf(ostream& os, vtkIndent indent) {
  this->Superclass::PrintSelf(os,indent);
}

