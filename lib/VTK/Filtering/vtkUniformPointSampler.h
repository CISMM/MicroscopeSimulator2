/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkUniformPointSampler.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkUniformPointSampler - Superclass for algorithms that produce only polydata as output
// .SECTION Description

// vtkUniformPointSampler is a class that creates a uniform sampling of points on 
// a vtkPolyData object.

#ifndef __vtkUniformPointSampler_h
#define __vtkUniformPointSampler_h

#include "vtkPolyDataAlgorithm.h"
#include "vtkSmartPointer.h"

class vtkMinimalStandardRandomSequence;


class vtkDoubleArray;

class vtkUniformPointSampler : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkUniformPointSampler,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/set density of point sampling in points per square unit.
  vtkGetMacro(Density, double);
  vtkSetClampMacro(Density, double, 0.0, VTK_DOUBLE_MAX);

  // Description:
  // Get area of the polygonal model.
  vtkGetMacro(SurfaceArea, double);

  // Description:
  // Use a fixed number of samples instead of a fixed density.
  vtkSetMacro(UseFixedNumberOfSamples,int);
  vtkGetMacro(UseFixedNumberOfSamples,int);
  vtkBooleanMacro(UseFixedNumberOfSamples,int);

  // Description:
  // If the UseFixedNumberOfSamples is true, then this gives the number of
  // samples that will be taken on the geometry.
  vtkSetMacro(NumberOfSamples,int);
  vtkGetMacro(NumberOfSamples,int);
  
  // Description:
  // Get number of sample points that would be on this geometry for given
  // surface area and density.
  virtual int ComputeNumberOfSamples(double surfaceArea, double density) = 0;

protected:
  vtkUniformPointSampler();
  virtual ~vtkUniformPointSampler();

  // Description:
  // Determines whether to use a fixed number of samples or fixed sample
  // density.
  int UseFixedNumberOfSamples;

  // Density:
  // Number of samples.
  int NumberOfSamples;

  // Description:
  // Density of points on this object's surface.
  double Density;

  // Description:
  // Surface area of the polygonal data.
  double SurfaceArea;

  // Description:
  // Time when the sampling was last computed.
  long ComputedTime;

  // Description:
  // Random number generator
  vtkSmartPointer< vtkMinimalStandardRandomSequence > Random;


private:
  vtkUniformPointSampler(const vtkUniformPointSampler&);  // Not implemented.
  void operator=(const vtkUniformPointSampler&);  // Not implemented.
};

#endif
