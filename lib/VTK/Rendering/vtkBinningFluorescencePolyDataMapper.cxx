/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkBinningFluorescencePolyDataMapper.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkBinningFluorescencePolyDataMapper.h"

#include "vtkFramebufferObjectRenderer.h"
#include "vtkPlaneCollection.h"
#include "vtkProperty.h"

#ifndef VTK_IMPLEMENT_MESA_CXX
vtkCxxRevisionMacro(vtkBinningFluorescencePolyDataMapper, "$Revision: 1.3 $");
vtkStandardNewMacro(vtkBinningFluorescencePolyDataMapper);
#endif


// Construct empty object.
vtkBinningFluorescencePolyDataMapper::vtkBinningFluorescencePolyDataMapper() {
  this->ListId = 0;
  this->Renderer = NULL;
  this->PSFImage = NULL;
  this->SetSimulatedRegion(0, 1, 0, 1);
  this->ExtensionsLoaded = 0;
}


// Destructor (don't call ReleaseGraphicsResources() because it is virtual)
vtkBinningFluorescencePolyDataMapper::~vtkBinningFluorescencePolyDataMapper() {
  if (this->Renderer) {
    this->UnRegister(this->Renderer);
    this->Renderer = NULL;
  }

  if (this->PSFImage) {
    this->UnRegister(this->PSFImage);
    this->PSFImage = NULL;
  }

  if (this->LastWindow) {
    this->ReleaseGraphicsResources(this->LastWindow);
  }
}


// Release the graphics resources used by this mapper.  In this case, 
// release the display list if any.
void vtkBinningFluorescencePolyDataMapper::ReleaseGraphicsResources(vtkWindow *win) {
  if (win) {
    win->MakeCurrent();
  }
  
  if (this->ListId && win) {
    glDeleteLists(this->ListId,1);
    this->ListId = 0;
  }
  this->LastWindow = NULL; 
}


// Draw method for OpenGL.
int vtkBinningFluorescencePolyDataMapper::Draw(vtkRenderer *aren, vtkActor *act) {
  if (!this->ExtensionsLoaded) {
    this->LoadExtensions(aren);
  }
  if (!this->PSFImage) {
    this->UpdateProgress(1.0);
    return 1;
  }
  
  // Turn on blending.
  glEnable(GL_BLEND);
  vtkgl::BlendEquation(vtkgl::FUNC_ADD);
  glBlendFunc(GL_ONE, GL_ONE);

  // Disable a few things.
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);

  this->Superclass::Superclass::Draw(aren, act);

  // Turn blending back off
  glDisable(GL_BLEND);

  this->UpdateProgress(1.0);
  return 1;
}


// Calls the superclass update method.
void vtkBinningFluorescencePolyDataMapper::Update() {
  this->Superclass::Update();
}


int vtkBinningFluorescencePolyDataMapper::LoadExtensions(vtkRenderer *renderer) {
  vtkOpenGLExtensionManager *manager = vtkOpenGLExtensionManager::New();
  manager->SetRenderWindow(renderer->GetRenderWindow());

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

  // Dispose of the manager.
  manager->Delete();
  this->ExtensionsLoaded = 1;
  return supported;
}


void vtkBinningFluorescencePolyDataMapper::PrintSelf(ostream& os, vtkIndent indent) {
  this->Superclass::PrintSelf(os,indent);
}