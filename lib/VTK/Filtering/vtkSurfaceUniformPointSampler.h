/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSurfaceUniformPointSampler.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSurfaceUniformPointSampler - Superclass for algorithms that produce only polydata as output
// .SECTION Description

// vtkSurfaceUniformPointSampler is a class that creates a uniform sampling of points on 
// a vtkPolyData object.

#ifndef __vtkSurfaceUniformPointSampler_h
#define __vtkSurfaceUniformPointSampler_h

#include "vtkUniformPointSampler.h"

class vtkDoubleArray;

class VTK_FILTERING_EXPORT vtkSurfaceUniformPointSampler : public vtkUniformPointSampler
{
public:
  static vtkSurfaceUniformPointSampler *New();
  vtkTypeRevisionMacro(vtkSurfaceUniformPointSampler,vtkUniformPointSampler);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get area of the polygonal model.
  vtkGetMacro(SurfaceArea, double);
  
  // Description:
  // Get number of sample points that would be on this geometry for given
  // surface area and density.
  int ComputeNumberOfSamples(double surfaceArea, double density);

protected:
  vtkSurfaceUniformPointSampler();
  virtual ~vtkSurfaceUniformPointSampler();

  // Description:
  // Weights assigned to each polygon in the input data based on area. Element in 
  // this array sum to 1.0.
  vtkDoubleArray *PolygonAreas;

  // Description:
  // Surface area of the polygonal data.
  double SurfaceArea;

  // Description:
  // Computes the surface weights of polygons in the input data.
  void ComputePolygonAreas(vtkPolyData* input);

  // Description:
  // Computes sample points on the surface using an area-weight uniform
  // sampling distribution.
  void CreateSamplePoints(vtkPolyData *input, vtkPolyData *output);

  // Description:
  // Given a triangle, creates a random point using simplex point picking.
  void RandomTrianglePoint(vtkTriangle *triangle, vtkPolyData *polyData, double *pt);

  // Description:
  // Usual data generation method.
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

private:
  vtkSurfaceUniformPointSampler(const vtkSurfaceUniformPointSampler&);  // Not implemented.
  void operator=(const vtkSurfaceUniformPointSampler&);  // Not implemented.
};

#endif
