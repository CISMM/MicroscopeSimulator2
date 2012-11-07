/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// This software was funded by NIH grant P41 EB002025 and was created by
// Cory Quammen (cquammen@cs.unc.edu) at the University of North Carolina at
// Chapel Hill Center for Computer Integrated Systems for Microscopy and Manipulation
// (http://www.cismm.org).

#include "vtkFramebufferObjectRenderer.h"

#include <vector>

#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkOpenGLExtensionManager.h"
#include "vtkOpenGLRenderWindow.h"


#include <math.h>

vtkStandardNewMacro(vtkFramebufferObjectRenderer);


vtkFramebufferObjectRenderer::vtkFramebufferObjectRenderer() {
  this->FramebufferTextureCollection = vtkCollection::New();
  this->ActiveFramebufferTextureIndex = 0;
  this->ProgramHandle = 0;
  this->FragmentShaderHandle = 0;
  this->MapsToZero = 0.0;
  this->MapsToOne  = 1.0;

#if TIME
  this->Stopwatch = StopWatch::create();
#endif // TIME
}


vtkFramebufferObjectRenderer::~vtkFramebufferObjectRenderer() {
  if (this->FramebufferTextureCollection != NULL) {
    this->FramebufferTextureCollection->UnRegister(this);
    this->FramebufferTextureCollection = NULL;
  }

  if (this->FragmentShaderHandle) {
    vtkgl::DeleteShader(this->FragmentShaderHandle);
    vtkgl::DeleteProgram(this->ProgramHandle);
  }

#if TIME
  StopWatch::destroy(this->Stopwatch);
#endif // TIME
}


void vtkFramebufferObjectRenderer::LoadFragmentProgram() {
  if (this->FragmentShaderHandle != 0)
    return;

  // Load GLSL shader extensions
  char glVersion2_0[] = "GL_VERSION_2_0";
  char textureRect[]  = "GL_ARB_texture_rectangle";

  vtkOpenGLRenderWindow* oglRenWin =
    vtkOpenGLRenderWindow::SafeDownCast(this->GetRenderWindow());
  if (oglRenWin == NULL)
    return;

  vtkOpenGLExtensionManager *manager = oglRenWin->GetExtensionManager();
  if (manager->ExtensionSupported(glVersion2_0))
    manager->LoadExtension(glVersion2_0);
  else
    vtkErrorMacro(<< "Your graphics card does not support the extension '" << glVersion2_0 << "'");

  if (manager->ExtensionSupported(textureRect))
    manager->LoadExtension(textureRect);
  else
    vtkErrorMacro(<< "Your graphics card does not support the extension '" << textureRect << "'");

  this->ProgramHandle = vtkgl::CreateProgram();

  vtkgl::GLchar** programList = new vtkgl::GLchar*[1];
  programList[0] = new vtkgl::GLchar[1024];
  strncpy(programList[0],
    "#extension GL_ARB_texture_rectangle : enable\n"
    "#pragma optimize(on)\n"
    "\n"

    "uniform sampler2DRect framebufferTexture;\n"
    "uniform float mapsToZero;\n"
    "uniform float invDiff;\n"
    "\n"
    "void main() {\n"
    "  vec4 color = texture2DRect(framebufferTexture, gl_TexCoord[0].st);\n"
    "  color.rgb -= mapsToZero;\n"
    "  color.rgb = clamp(color.rgb * invDiff, 0.0, 1.0);\n"
    "  gl_FragColor = color;\n"
          "}\n",
          1024);

  this->FragmentShaderHandle = vtkgl::CreateShader(vtkgl::FRAGMENT_SHADER);
  vtkgl::AttachShader(this->ProgramHandle, this->FragmentShaderHandle);
  vtkgl::ShaderSource(this->FragmentShaderHandle, 1, const_cast<const vtkgl::GLchar**>(programList), NULL);
  vtkgl::CompileShader(this->FragmentShaderHandle);

  /* Check for errors. */
  GLint status;
  char infoLog[256];
  GLint length;
  vtkgl::GetShaderiv(this->FragmentShaderHandle, vtkgl::COMPILE_STATUS, &status);
  if (!status) {
    vtkErrorMacro(<< "Failed to compile fragment program");
    vtkgl::GetShaderInfoLog(this->FragmentShaderHandle, 256, &length, infoLog);
    vtkErrorMacro(<< infoLog);
  }

  vtkgl::LinkProgram(this->ProgramHandle);

  /* Check for errors. */
  vtkgl::GetProgramiv(this->ProgramHandle, vtkgl::LINK_STATUS, &status);
  if (!status) {
    vtkErrorMacro(<< "Failed to link fragment program");
    vtkgl::GetProgramInfoLog(this->ProgramHandle, 256, &length, infoLog);
    vtkErrorMacro(<< infoLog);
  }
}


void vtkFramebufferObjectRenderer::RemapAndDisplay() {
  this->RemapAndDisplay(this->ActiveFramebufferTextureIndex);
}


void vtkFramebufferObjectRenderer::RemapAndDisplay(int index) {
  this->LoadFragmentProgram();

  // Draw textured rectangle in the window frame buffer.
  // Load the framebuffer texture.
  if (this->ProgramHandle != 0) {
    vtkgl::UseProgram(this->ProgramHandle);

    // Set texture unit from which the fragment program will pull data
    GLint fbTexture = vtkgl::GetUniformLocation(this->ProgramHandle, "framebufferTexture");
    vtkgl::Uniform1i(fbTexture, 0);

    // Bind mapping values for the fragment program.
    GLfloat mapsToZero = static_cast<GLfloat>(this->MapsToZero);
    GLint mapsToZeroHandle = vtkgl::GetUniformLocation(this->ProgramHandle, "mapsToZero");
    vtkgl::Uniform1f(mapsToZeroHandle, mapsToZero);

    GLfloat invDiff = static_cast<GLfloat>(1.0 / (this->MapsToOne - this->MapsToZero));
    GLint invDiffHandle = vtkgl::GetUniformLocation(this->ProgramHandle, "invDiff");
    vtkgl::Uniform1f(invDiffHandle, invDiff);
  }

  // Clear out buffer prior to rendering.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (this->GetFramebufferTexture(index)) {
    this->GetFramebufferTexture(index)->Render(this);
    this->TextureRender(index);
    this->GetFramebufferTexture(index)->PostRender();
  }

  if (this->ProgramHandle != 0) {
    vtkgl::UseProgram(0);
  }
}


void vtkFramebufferObjectRenderer::ResetTimer() {
#if TIME
  StopWatch::get(this->Stopwatch).reset();
#endif // TIME
}


void vtkFramebufferObjectRenderer::StartTimer() {
#if TIME
  StopWatch::get(this->Stopwatch).start();
#endif // TIME
}


void vtkFramebufferObjectRenderer::StopTimer() {
#if TIME
  glFinish();
  StopWatch::get(this->Stopwatch).stop();
#endif // TIME
}


void vtkFramebufferObjectRenderer::PrintTime() {
#if TIME
  cout << StopWatch::get(this->Stopwatch).getTime() << endl;
#endif // TIME
}


int vtkFramebufferObjectRenderer::GetNumberOfFramebufferTextures() {
  return this->FramebufferTextureCollection->GetNumberOfItems();
}


void vtkFramebufferObjectRenderer::SetFramebufferTexture(vtkFramebufferObjectTexture *texture) {
  vtkFramebufferObjectTexture *currentTexture = this->GetFramebufferTexture();
  if (currentTexture != texture) {
    this->FramebufferTextureCollection->RemoveItem(currentTexture);
  }
  if (texture != NULL) {
    this->FramebufferTextureCollection->AddItem(texture);
    this->Modified();
  }
}


vtkFramebufferObjectTexture * vtkFramebufferObjectRenderer::GetFramebufferTexture() {
  return this->GetFramebufferTexture(0);
}


vtkFramebufferObjectTexture * vtkFramebufferObjectRenderer::GetFramebufferTexture(int index) {
  if (index < this->FramebufferTextureCollection->GetNumberOfItems()) {
    return vtkFramebufferObjectTexture::SafeDownCast(
      this->FramebufferTextureCollection->GetItemAsObject(index));
  }

  return NULL;
}


vtkFramebufferObjectTexture * vtkFramebufferObjectRenderer::GetActiveFramebufferTexture() {
  return this->GetFramebufferTexture(this->ActiveFramebufferTextureIndex);
}


void vtkFramebufferObjectRenderer::AddFramebufferTexture(vtkFramebufferObjectTexture *texture) {
  if (texture != NULL) {
    this->FramebufferTextureCollection->AddItem(texture);
  }
}


void vtkFramebufferObjectRenderer::SetActiveFramebufferTexture(int index) {
  if (index < 0 || index >= this->FramebufferTextureCollection->GetNumberOfItems()) {
    vtkErrorMacro(<< " Active framebuffer index is invalid");
    return;
  }
  this->ActiveFramebufferTextureIndex = index;
}


// Concrete open gl render method.
void vtkFramebufferObjectRenderer::DeviceRender(void) {
  // Do not remove this MakeCurrent! Due to Start / End methods on
  // some objects which get executed during a pipeline update,
  // other windows might get rendered since the last time
  // a MakeCurrent was called.
  this->RenderWindow->MakeCurrent();

  this->ResetTimer();
  this->StartTimer();

  // Set the framebuffer texture as the rendering target.
  if (this->GetActiveFramebufferTexture())
    this->GetActiveFramebufferTexture()->EnableTarget(this);

  // Standard render method.
  this->Superclass::DeviceRender();

  // Now that rendering is finished, disable the framebuffer texture as the rendering target.
  if (this->GetActiveFramebufferTexture())
    this->GetActiveFramebufferTexture()->DisableTarget();

  // Now remap the framebuffer texture to the display range of
  // the display.
  this->RemapAndDisplay();

  this->StopTimer();
  this->PrintTime();
}


void vtkFramebufferObjectRenderer::TextureRender() {
  this->TextureRender(this->ActiveFramebufferTextureIndex);
}


void vtkFramebufferObjectRenderer::TextureRender(int index) {

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

  glPopMatrix();
  glPopAttrib();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
}


void vtkFramebufferObjectRenderer::CheckFrameBufferStatus() {
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

void vtkFramebufferObjectRenderer::PrintSelf(ostream& os, vtkIndent indent) {
  this->Superclass::PrintSelf(os,indent);
}

