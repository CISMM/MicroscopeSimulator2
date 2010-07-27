/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkBinningFluorescencePolyDataMapper.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkBinningFluorescencePolyDataMapper - a PolyDataMapper for the OpenGL library
// .SECTION Description
// vtkBinningFluorescencePolyDataMapper is a subclass of vtkOpenGLPolyDataMapper.
// vtkBinningFluorescencePolyDataMapper is a geometric PolyDataMapper for the OpenGL 
// rendering library.

#ifndef __vtkBinningFluorescencePolyDataMapper_h
#define __vtkBinningFluorescencePolyDataMapper_h

#include <math.h>

#include "vtkImageData.h"
#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGL.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkOpenGLTexture.h"
#include "vtkFluorescencePolyDataMapper.h"
#include "vtkOpenGLExtensionManager.h"
#include "vtkPolyData.h"
#include "vtkRenderer.h"

#ifndef VTK_IMPLEMENT_MESA_CXX
# include "vtkOpenGL.h"
#endif
#include "vtkgl.h"

class VTK_RENDERING_EXPORT vtkBinningFluorescencePolyDataMapper : public vtkFluorescencePolyDataMapper
{
public:
  static vtkBinningFluorescencePolyDataMapper *New();
  vtkTypeRevisionMacro(vtkBinningFluorescencePolyDataMapper,vtkFluorescencePolyDataMapper);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Release any graphics resources that are being consumed by this mapper.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Draw method for OpenGL.
  virtual int Draw(vtkRenderer *ren, vtkActor *a);

  // Description:
  // Loads required extensions for vtkBinningFluorescencePolyDataMappers.
  int LoadExtensions(vtkRenderer *renderer);

  // Description:
  // Updates member variables and calls Update() method of superclass.
  virtual void Update();
  
protected:
  vtkBinningFluorescencePolyDataMapper();
  ~vtkBinningFluorescencePolyDataMapper();

private:
  vtkBinningFluorescencePolyDataMapper(const vtkBinningFluorescencePolyDataMapper&);  // Not implemented.
  void operator=(const vtkBinningFluorescencePolyDataMapper&);  // Not implemented.

  // Description:
  // Flag indicating whether the extensions have been loaded yet.
  int ExtensionsLoaded;

};

#endif
