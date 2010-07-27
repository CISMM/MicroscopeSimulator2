/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVolumeUniformPointSampler.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVolumeUniformPointSampler - Superclass for algorithms that produce only polydata as output
// .SECTION Description

// vtkVolumeUniformPointSampler is a class that creates a uniform sampling of points on 
// a vtkPolyData object.

#ifndef __vtkVolumeUniformPointSampler_h
#define __vtkVolumeUniformPointSampler_h

#include "vtkPolyDataAlgorithm.h"
#include "vtkUniformPointSampler.h"
#include "vtkUnstructuredGrid.h"
#include "vtkTetra.h"

class vtkDoubleArray;

class VTK_FILTERING_EXPORT vtkVolumeUniformPointSampler : public vtkUniformPointSampler
{
public:
  static vtkVolumeUniformPointSampler *New();
  vtkTypeRevisionMacro(vtkVolumeUniformPointSampler,vtkUniformPointSampler);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get volume of the tetrahedral grid.
  vtkGetMacro(Volume, double);
  
  // Description:
  // Get number of sample points that would be contained in this mesh for given
  // volume and density.
  int ComputeNumberOfSamples(double volume, double density);

protected:
  vtkVolumeUniformPointSampler();
  virtual ~vtkVolumeUniformPointSampler();

  // Description:
  // Volume of each tetrahedra.
  vtkDoubleArray *TetrahedraVolumes;

  // Description:
  // Volume of the tetrahedral grid.
  double Volume;

  // Description:
  // Overridden to specify that input is required to be vtkUnstructuredGrid
  virtual int FillInputPortInformation(int port, vtkInformation *info);

  // Description:
  // Computes the volumes of the tetrahedra in the grid.
  void ComputeTetrahedraVolumes(vtkUnstructuredGrid* input);

  // Description:
  // Computes sample points on the surface using an area-weight uniform
  // sampling distribution.
  void CreateSamplePoints(vtkUnstructuredGrid *input, vtkPolyData *output);

  // Description:
  // Given a tetrahedron, creates a random point within the tetrahedron 
  // using simplex point picking.
  void RandomTetrahedronPoint(vtkTetra *tetra, vtkUnstructuredGrid *grid, double *pt);

  // Description:
  // Usual data generation method.
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

private:
  vtkVolumeUniformPointSampler(const vtkVolumeUniformPointSampler&);  // Not implemented.
  void operator=(const vtkVolumeUniformPointSampler&);  // Not implemented.
};

#endif
