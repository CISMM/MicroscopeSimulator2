/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkFluorescencePolyDataMapper.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkFluorescencePolyDataMapper.h"

#ifndef VTK_IMPLEMENT_MESA_CXX
vtkCxxRevisionMacro(vtkFluorescencePolyDataMapper, "$Revision: 1.11 $");
#endif

// Construct empty object.
vtkFluorescencePolyDataMapper::vtkFluorescencePolyDataMapper() {
  this->ListId = 0;
  this->Gain = 1.0;
  this->Renderer = NULL;
  this->SimulatedRegion[0] = 0;
  this->SimulatedRegion[1] = 1;
  this->SimulatedRegion[2] = 0;
  this->SimulatedRegion[3] = 1;
  this->PixelSize[0] = 1.0;
  this->PixelSize[1] = 1.0;
  this->Shear[0] = 0.0;
  this->Shear[1] = 0.0;
  this->FocalPlaneDepth = 0.0;
  this->PSFTexture = NULL;
}

// Destructor (don't call ReleaseGraphicsResources() because it is virtual)
vtkFluorescencePolyDataMapper::~vtkFluorescencePolyDataMapper() {
  SetPSFTexture(NULL);

  if (this->Renderer) {
    this->UnRegister(this->Renderer);
    this->Renderer = NULL;
  }
}

// Release the graphics resources used by this mapper.  In this case, release
// the display list if any.
void vtkFluorescencePolyDataMapper::ReleaseGraphicsResources(vtkWindow *win) {
  if (this->ListId && win) {
    win->MakeCurrent();
    glDeleteLists(this->ListId,1);
    this->ListId = 0;
  }
  this->LastWindow = NULL; 
}

// We need to know the simulation region so that we know how to scale from world 
// space to screen space.
void vtkFluorescencePolyDataMapper::SetSimulatedRegion(double xBegin, 
                                                       double xEnd, 
                                                       double yBegin, 
                                                       double yEnd) {
  this->SimulatedRegion[0] = xBegin;
  this->SimulatedRegion[1] = xEnd;
  this->SimulatedRegion[2] = yBegin;
  this->SimulatedRegion[3] = yEnd;
}


void vtkFluorescencePolyDataMapper::RenderPiece(vtkRenderer *ren, vtkActor *act) {

  // Make sure the texture matrix is the identity
  glMatrixMode(GL_TEXTURE);
  glPushMatrix();
  glLoadIdentity();

  // Do the rendering for this actor.
  this->Superclass::RenderPiece(ren, act);

  // Pop the texture matrix off the stack
  glMatrixMode(GL_TEXTURE);
  glPopMatrix();
}

void vtkFluorescencePolyDataMapper::SetRenderer(vtkRenderer *renderer) {
  if (this->Renderer) {
    this->UnRegister(this->Renderer);
    this->Renderer = 0;
  }
  this->Renderer = renderer;
  this->Register(renderer);
}

void vtkFluorescencePolyDataMapper::PrintSelf(ostream& os, vtkIndent indent) {
  this->Superclass::PrintSelf(os,indent);
}
