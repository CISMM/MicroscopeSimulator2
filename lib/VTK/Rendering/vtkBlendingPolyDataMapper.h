/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkBlendingPolyDataMapper.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkBlendingPolyDataMapper - a PolyDataMapper for the OpenGL library
// .SECTION Description
// vtkBlendingPolyDataMapper is a subclass of vtkOpenGLPolyDataMapper.
// vtkBlendingPolyDataMapper turns on blending prior to rendering geometry.

#ifndef __vtkBlendingPolyDataMapper_h
#define __vtkBlendingPolyDataMapper_h

#include "vtkObjectFactory.h"
#include "vtkOpenGLPolyDataMapper.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkOpenGLExtensionManager.h"
#include "vtkRenderer.h"

#ifndef VTK_IMPLEMENT_MESA_CXX
# include "vtkOpenGL.h"
#endif
#include "vtkgl.h"

class vtkBlendingPolyDataMapper : public vtkOpenGLPolyDataMapper
{
public:
  static vtkBlendingPolyDataMapper *New();
  vtkTypeRevisionMacro(vtkBlendingPolyDataMapper,vtkOpenGLPolyDataMapper);

  // Description:
  // Turns blending on prior to calling RenderPiece() from the superclass.
  virtual void RenderPiece(vtkRenderer *ren, vtkActor *act);

  // Description:
  // Loads required extensions for vtkBlendingPolyDataMappers.
  int LoadExtensions(vtkRenderer *renderer);
  
protected:
  vtkBlendingPolyDataMapper();
  ~vtkBlendingPolyDataMapper();

private:
  vtkBlendingPolyDataMapper(const vtkBlendingPolyDataMapper&);  // Not implemented.
  void operator=(const vtkBlendingPolyDataMapper&);  // Not implemented.

  // Description:
  // Flag indicating whether the extensions have been loaded yet.
  int ExtensionsLoaded;

};

#endif
