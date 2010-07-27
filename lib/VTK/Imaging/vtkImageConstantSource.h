/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkImageConstantSource.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkImageConstantSource - Create an image filled with a constant value.
// .SECTION Description
// vtkImageConstantSource produces images filled with a constant value.
// The default scalar type is VTK_DOUBLE.


#ifndef __vtkImageConstantSource_h
#define __vtkImageConstantSource_h

#include "vtkImageAlgorithm.h"


class VTK_IMAGING_EXPORT vtkImageConstantSource : public vtkImageAlgorithm 
{
public:
  static vtkImageConstantSource *New();
  vtkTypeRevisionMacro(vtkImageConstantSource,vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the constant value for the image.
  vtkSetMacro(Constant, double);
  vtkGetMacro(Constant, double);
  
  // Description:
  // Set/Get the spacing between image samples.
  vtkSetVector3Macro(Spacing, double);
  vtkGetVector3Macro(Spacing, double);
  
  // Description:
  // Set/Get the origin of the image.
  vtkSetVector3Macro(Origin, double);
  vtkGetVector3Macro(Origin, double);

  // Description:
  // Set how large of an image to generate.
  void SetWholeExtent(int xMinx, int xMax, int yMin, int yMax,
                      int zMin, int zMax);

protected:
  vtkImageConstantSource();
  ~vtkImageConstantSource() {};

  double Constant;
  double Origin[3];
  double Spacing[3];
  int WholeExtent[6];

  virtual int RequestInformation (vtkInformation *, vtkInformationVector**, vtkInformationVector *);
  virtual void ExecuteData(vtkDataObject *data);
private:
  vtkImageConstantSource(const vtkImageConstantSource&);  // Not implemented.
  void operator=(const vtkImageConstantSource&);  // Not implemented.
};


#endif

  
