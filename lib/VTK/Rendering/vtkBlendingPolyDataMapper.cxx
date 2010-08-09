/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkBlendingPolyDataMapper.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkBlendingPolyDataMapper.h"

#include "vtkProperty.h"

#ifndef VTK_IMPLEMENT_MESA_CXX
vtkCxxRevisionMacro(vtkBlendingPolyDataMapper, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkBlendingPolyDataMapper);
#endif

// Construct empty object.
vtkBlendingPolyDataMapper::vtkBlendingPolyDataMapper() {
  this->ExtensionsLoaded = 0;
}


// Destructor (don't call ReleaseGraphicsResources() because it is virtual)
vtkBlendingPolyDataMapper::~vtkBlendingPolyDataMapper() {
  if (this->LastWindow) {
    this->ReleaseGraphicsResources(this->LastWindow);
  }
}


void vtkBlendingPolyDataMapper::RenderPiece(vtkRenderer *ren, vtkActor *act) {
  // Make sure extensions are loaded
  this->LoadExtensions(ren);

  // Turn blending on
  glEnable(GL_BLEND);
  vtkgl::BlendEquation(vtkgl::FUNC_ADD);
  glBlendFunc(GL_ONE, GL_ONE);

  // Do the rendering for this actor.
  this->Superclass::RenderPiece(ren, act);

  // Turn blending off
  glDisable(GL_BLEND);
}


int vtkBlendingPolyDataMapper::LoadExtensions(vtkRenderer *renderer) {
  if (this->ExtensionsLoaded)
    return 1;

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

  std::string arbImaging("GL_ARB_imaging");
  if (manager->ExtensionSupported(arbImaging.c_str())) {
    manager->LoadExtension(arbImaging.c_str());
  } else {
    vtkErrorMacro(<< "Your graphics card does not support OpenGL extension '" << arbImaging.c_str() << "'");
  }

  this->ExtensionsLoaded = 1;

  // Dispose of the manager.
  manager->Delete();
  return (versionOK > 0);
}
