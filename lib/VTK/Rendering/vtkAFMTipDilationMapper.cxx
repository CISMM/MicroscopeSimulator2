/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkAFMTipDilationMapper.cxx,v $
  Language:  C++
  Date:      $Date: 2007/04/17 21:26:53 $
  Version:   $Revision: 1.8 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkAFMTipDilationMapper.h"

vtkCxxRevisionMacro(vtkAFMTipDilationMapper, "$Revision: 1.8 $");
vtkStandardNewMacro(vtkAFMTipDilationMapper);

//------------------------------------------------------------------------
vtkAFMTipDilationMapper::vtkAFMTipDilationMapper()
{
  this->TipImage = NULL;
}

//------------------------------------------------------------------------
vtkAFMTipDilationMapper::~vtkAFMTipDilationMapper()
{
  if (TipImage) {
    TipImage->UnRegister(this);
  }
}

//------------------------------------------------------------------------
// Releases internal graphics resources
void vtkAFMTipDilationMapper::ReleaseGraphicsResources( vtkWindow *renWin )
{
  glDeleteLists(this->TipList, 1);
}

//------------------------------------------------------------------------
// Render the geometry
void vtkAFMTipDilationMapper::Render(vtkRenderer *ren, vtkActor *act)
{
  // Make sure that we've been properly initialized
  if (!this->GetInput())
  {
    vtkErrorMacro(<< "No input!\n");
    return;
  }

  if (!this->TipImage)
  {
    vtkErrorMacro(<< "No tip!\n");
    return;
  }

  // Make sure our window is current
  ren->GetRenderWindow()->MakeCurrent();
  if (this->TipBuildTime.GetMTime() < this->TipImage->GetMTime())
  {
    // Release old tip
    glDeleteLists(this->TipList, 1);
    this->CreateTip();
    TipBuildTime.Modified();
  }

  vtkDataSet *input = this->GetInput();

  // Loop over input points and transform tip before rendering.
  vtkImageData *depthImage = (vtkImageData *) input;
  vtkFloatArray *zbuffer = (vtkFloatArray *)depthImage->GetPointData()->GetScalars("ZBuffer");

  double *spacing = depthImage->GetSpacing();
  double *origin = depthImage->GetOrigin();
  int *dims = depthImage->GetDimensions();

  vtkIdType index[3];
  index[2] = 0;
  vtkIdType id;

  // Turn just about everything off in OpenGL
  glDisable(GL_LIGHTING);
  glDisable(GL_BLEND);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_POLYGON_SMOOTH);
  glDisable(GL_STENCIL_TEST);
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_AUTO_NORMAL);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DITHER);
  glDisable(GL_FOG);
  glDisable(GL_LINE_SMOOTH);
  glDisable(GL_NORMALIZE);
  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_COLOR_LOGIC_OP);
  glDisable(GL_LIGHT0);
  glDisable(GL_LIGHT1);
  glDisable(GL_POINT_SMOOTH);
  for (int i = 0; i < 6; i++) { 
	  glDisable(GL_CLIP_PLANE0 +i);
  }
  glEnable(GL_DEPTH_TEST);
  glClearDepth(1.0);
  glDepthFunc(GL_LESS);
  glPointSize(1);

  glMatrixMode(GL_MODELVIEW);

  float tSpaceX = this->TipImage->GetSpacing()[0];
  float tSpaceY = this->TipImage->GetSpacing()[1];
  float tipHalfX = 0.5 * (this->TipImage->GetDimensions()[0]) * tSpaceX;
  float tipHalfY = 0.5 * (this->TipImage->GetDimensions()[1]) * tSpaceY;

#define TIP
#ifndef TIP
  tipHalfX = 0.0f;
  tipHalfY = 0.0f;
#endif
  double *tipOrigin = this->TipImage->GetOrigin();
  float x, y, z;
  for (int j = 0; j < dims[1]; j++)
  {
    index[1] = j;
    y = j*spacing[1] + origin[1] - tipHalfY;

    for (int i = 0; i < dims[0]; i++)
    {
      index[0] = i;
      x = i*spacing[0] + origin[0] - tipHalfX;
      id = j*dims[0] + i;
      z  = zbuffer->GetTuple1(id);

      if (z > 0.01)
      {
        glPushMatrix();
        glTranslatef(x, y, z + tipOrigin[2]);
#ifndef TIP
        glBegin(GL_POINTS);
        glVertex3f(0, 0, 0);
        glEnd();
#else
        glCallList(this->TipList);
#endif
        glPopMatrix();
      }
    }
  }

  // Assume no error
  return;
}

//------------------------------------------------------------------------
// Create the OpenGL representation of the AFM tip
void vtkAFMTipDilationMapper::CreateTip() {
  float height;
  //double* tipOrigin = this->TipImage->GetOrigin();
  
  // tip for dilation
  this->TipList = glGenLists(1);
  int maxX = this->TipImage->GetDimensions()[0];
  int maxY = this->TipImage->GetDimensions()[1];
  double *spacing = this->TipImage->GetSpacing();
  double xSpacing = spacing[0];
  double ySpacing = spacing[1];

  // Find minimum height
  double minHeight = 0.0;
  for (int i = 0; i < maxX; i++) {
    for (int j = 0; j < maxY; j++) {
      height = this->TipImage->GetScalarComponentAsFloat(i, j, 0, 0);
      if (height < minHeight)
        minHeight = height;
    }
  }
  glNewList(this->TipList, GL_COMPILE);
  glBegin(GL_POINTS);
  for (int i = 0; i < maxX; i++) {
    for (int j = 0; j < maxY; j++) {
      height = this->TipImage->GetScalarComponentAsFloat(i, j, 0, 0);
      if (height > 1){
        glColor3f(1,0,0);
      }

      if (height > minHeight) {
        glVertex3f((i + 0.5) * xSpacing, (j + 0.5) * ySpacing, height);
      }
    }
  }
  glEnd();
  glEndList();
}

//------------------------------------------------------------------------
void vtkAFMTipDilationMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
