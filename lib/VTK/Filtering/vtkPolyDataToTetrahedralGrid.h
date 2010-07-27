/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPolyDataToTetrahedralGrid.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPolyDataToTetrahedralGrid - reates a tetrahedral mesh
// from the volume defined by a vtkPolyData.
// .SECTION Description
// vtkPolyDataToTetrahedralGrid is a class that creates a tetrahedral mesh
// from a vtkPolyData consisting of one or more watertight components. This
// class wraps the TetGen library. For more details on the algorithm, see
//
// Si, H. and Gaertner, K. (2005). Meshing piecewise linear complexes by 
// constrained Delaunay tetrahedralizations. In Proceedings of the 14th 
// International Meshing Roundtable, pages 147–163.
//
// .SECTION Warning
// As implemented, this class uses TetGen's default tetrahedralizer algorithm.
// This is fast but may not produce high-quality meshes. If you need 
// high-quality meshes, modify the flags passed in to the tetrahedralize()
// function. A list of options is available at
//
// http://tetgen.berlios.de/switches.html

#ifndef __vtkPolyDataToTetrahedralGrid_h
#define __vtkPolyDataToTetrahedralGrid_h

#include "vtkUnstructuredGridAlgorithm.h"

class vtkDoubleArray;

class vtkPolyDataToTetrahedralGrid : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkPolyDataToTetrahedralGrid *New();
  vtkTypeRevisionMacro(vtkPolyDataToTetrahedralGrid,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);


protected:
  vtkPolyDataToTetrahedralGrid();
  ~vtkPolyDataToTetrahedralGrid();

  // Description:
  // Fills in input data port information.
  virtual int FillInputPortInformation(int port, vtkInformation* info);

  // Description:
  // Usual data generation method.
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

private:
  vtkPolyDataToTetrahedralGrid(const vtkPolyDataToTetrahedralGrid&);  // Not implemented.
  void operator=(const vtkPolyDataToTetrahedralGrid&);  // Not implemented.
};

#endif
