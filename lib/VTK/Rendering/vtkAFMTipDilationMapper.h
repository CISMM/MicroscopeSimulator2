/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkAFMTipDilationMapper.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkAFMTipDilationMapper - concrete class that is used to generate an AFM scan simulation
// using gray scale morphology
// .SECTION Description

#ifndef __vtkAFMTipDilationMapper_h
#define __vtkAFMTipDilationMapper_h

#include "vtkDataSetMapper.h"
#include "vtkImageData.h"
#include "vtkFloatArray.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"

#include "vtkOpenGL.h"

class vtkAFMTipDilationMapper : public vtkDataSetMapper 
{
public:
  static vtkAFMTipDilationMapper *New();
  vtkTypeMacro(vtkAFMTipDilationMapper,vtkDataSetMapper);
  void PrintSelf(ostream& os, vtkIndent indent);
  void Render(vtkRenderer *ren, vtkActor *act);

  // Description:
  // Release any graphics resources that are being consumed by this mapper.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  vtkSetObjectMacro(TipImage, vtkImageData);

protected:
  vtkTimeStamp       TipBuildTime;
  GLuint             TipList;
  
  // Description:
  // 2D image with surface height scalar representing gray-scale morphology structuring element.
  // Assumes the two dimensions have equal logical dimensions.
  vtkImageData      *TipImage;

	vtkAFMTipDilationMapper();
	~vtkAFMTipDilationMapper();

  // Description:
  // Creates the display list for rendering the tip.
  void CreateTip();

private:
  vtkAFMTipDilationMapper(const vtkAFMTipDilationMapper&);  // Not implemented
  void operator=(const vtkAFMTipDilationMapper&);  // Not implemented
};

#endif
