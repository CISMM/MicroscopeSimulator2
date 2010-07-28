/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkFluorescencePointsGradientRenderer.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkFluorescencePointsGradientRenderer.h"

#include <vector>

#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkOpenGLExtensionManager.h"

#ifndef VTK_IMPLEMENT_MESA_CXX
# include "vtkOpenGL.h"
#endif

#include <math.h>

#ifndef VTK_IMPLEMENT_MESA_CXX
vtkCxxRevisionMacro(vtkFluorescencePointsGradientRenderer, "$Revision: 1.2 $");
vtkStandardNewMacro(vtkFluorescencePointsGradientRenderer);
#endif


vtkFluorescencePointsGradientRenderer::vtkFluorescencePointsGradientRenderer() {
  this->ExperimentalImageTexture = NULL;
  this->ExtensionsLoaded = 0;
}


vtkFluorescencePointsGradientRenderer::~vtkFluorescencePointsGradientRenderer() {
  this->SetExperimentalImageTexture(NULL);
}


void vtkFluorescencePointsGradientRenderer::DeviceRender(void) {
  if (!this->ExperimentalImageTexture) {
    vtkErrorMacro(<< "ExperimentalImageTexture is not set");
    return;
  }

  if (!this->GetFramebufferTexture(0)) {
    vtkErrorMacro(<< "FramebufferTexture is not set");
    return;
  }

  this->LoadExtensions(this->GetRenderWindow());

  // Do not remove this MakeCurrent! Due to Start / End methods on
  // some objects which get executed during a pipeline update, 
  // other windows might get rendered since the last time
  // a MakeCurrent was called.
  this->RenderWindow->MakeCurrent();

  /////////////// SETUP ///////////////

  // TEXTURE0 reserved for mappers (PSF gradient texture);
  // TEXTURE1 reserved for mappers (point list texture)

  // Bind framebuffer texture containing synthesized image
  vtkgl::ActiveTexture(vtkgl::TEXTURE2);
  this->GetFramebufferTexture(0)->Render(this);

  // Bind ExperimentalImageTexture to TEXUNIT 3
  vtkgl::ActiveTexture(vtkgl::TEXTURE3);
  this->ExperimentalImageTexture->Render(this);

  /////////////// RENDERING ///////////////

  // Each mapper will have loaded an FBO texture to which rendering should take
  // place. All we need to do is call the standard render method
#if 0
  this->Superclass::Superclass::DeviceRender();
#else
  glMatrixMode(GL_MODELVIEW);

  //this->UpdateGeometry();
  vtkCollectionSimpleIterator sit;
  this->Props->InitTraversal(sit);
  vtkProp* prop;
  while ((prop = this->Props->GetNextProp(sit))) {
    prop->RenderOpaqueGeometry(this);
  }

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
#endif


  /////////////// CLEAN UP ///////////////
  vtkgl::ActiveTexture(vtkgl::TEXTURE3);
  this->ExperimentalImageTexture->PostRender(this);

  // Disable the framebuffer texture containing the synthesized image
  vtkgl::ActiveTexture(vtkgl::TEXTURE2);
  this->GetFramebufferTexture(0)->PostRender();

  // TEXTURE1 reserved for mappers
  // TEXTURE0 reserved for mappers
}


int vtkFluorescencePointsGradientRenderer::LoadExtensions(vtkRenderWindow* renWin) {
  if (this->ExtensionsLoaded)
    return 1;

  vtkOpenGLExtensionManager *manager = vtkOpenGLExtensionManager::New();
  manager->SetRenderWindow(renWin);

  std::vector<std::string> versions;
  versions.push_back("GL_VERSION_1_1");
  versions.push_back("GL_VERSION_1_2");
  versions.push_back("GL_VERSION_1_3");

  for (unsigned int i = 0; i < versions.size(); i++) {
    manager->LoadSupportedExtension(versions[i].c_str());
  }

  this->ExtensionsLoaded = 1;

  return 0;
}


void vtkFluorescencePointsGradientRenderer::PrintSelf(ostream& os, vtkIndent indent) {
  this->Superclass::PrintSelf(os,indent);
}
