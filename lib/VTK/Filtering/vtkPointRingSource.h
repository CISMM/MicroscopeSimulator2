/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPointRingSource.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPointRingSource - Generates a set of points in a ring.
// .SECTION Description
// vtkPointRingSource generates a set of points in the shape of a ring.
// Each vertex is spaced evenly about the circumference of the ring. The ring
// radius and the number of points on the perimeter are adjustable parameters.


#ifndef __vtkPointRingSource_h
#define __vtkPointRingSource_h

#include "vtkPolyDataAlgorithm.h"

class vtkPointRingSource : public vtkPolyDataAlgorithm {
 public:
  // Description:
  // Construct a ring about the x-axis with radius 1 and 5 vertices.
  static vtkPointRingSource* New();
  vtkTypeMacro(vtkPointRingSource,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify the radius of the ring.
  vtkSetMacro(Radius,double);
  vtkGetMacro(Radius,double);

  // Description:
  // Specify the number of points.
  virtual void SetNumberOfPoints(int points);
  vtkGetMacro(NumberOfPoints,int);

 protected:
  vtkPointRingSource();
  virtual ~vtkPointRingSource() {};

  int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector *);

  double Radius;
  int NumberOfPoints;

 private:
  vtkPointRingSource(const vtkPointRingSource&); // Not implemented
  void operator=(const vtkPointRingSource&); // Not implemented

};

#endif // __vtkPointRingSource_h
