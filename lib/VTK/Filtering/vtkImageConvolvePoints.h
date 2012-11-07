/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkImageConvolvePoints.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkImageConvolvePoints - Convolve points with a 3D volume into another
// 3D volume.
// .SECTION Description
// vtkImageConvolvePoints splats an arbitrary 3D kernel into a 2D or 3D image at the
// locations defined by points in an input geometry. A lot of functionality is
// borrowed/inspired by vtkGaussianSplatter.

// .SECTION Caveats
// The input to this filter is any dataset type. This filter can be used 
// to convolve any form of data, i.e., the input data need not be 
// unstructured. 
//
// Some voxels may never receive a contribution during the splatting process.
// The final value of these points can be specified with the "NullValue" 
// instance variable.

#ifndef __vtkImageConvolvePoints_h
#define __vtkImageConvolvePoints_h

#include "vtkImageAlgorithm.h"
#include "vtkDataSet.h"
#include "vtkImageData.h"

#define VTK_ACCUMULATION_MODE_MIN 0
#define VTK_ACCUMULATION_MODE_MAX 1
#define VTK_ACCUMULATION_MODE_SUM 2

class vtkDoubleArray;

class vtkImageConvolvePoints : public vtkImageAlgorithm 
{
public:
  static vtkImageConvolvePoints *New();
  vtkTypeMacro(vtkImageConvolvePoints,vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Sets / gets the input data object containing the points to convolve.
  //svoid SetInputPointData(vtkDataSet *pointData);
  vtkSetObjectMacro(InputPointData, vtkDataSet);
  vtkGetObjectMacro(InputPointData, vtkDataSet);
  
  // Description:
  // Sets / gets the convolution kernel.
  vtkSetObjectMacro(ConvolutionKernel, vtkImageData);
  vtkGetObjectMacro(ConvolutionKernel, vtkImageData);

  // Description:
  // Multiply convolution kernel by this value.
  vtkSetClampMacro(ScaleFactor, double, 0.0, VTK_DOUBLE_MAX);
  vtkGetMacro(ScaleFactor, double);

  // Description:
  // Specify the scalar accumulation mode. This mode expresses how scalar
  // values are combined when splats are overlapped.
  vtkSetClampMacro(AccumulationMode, int,
                   VTK_ACCUMULATION_MODE_MIN, VTK_ACCUMULATION_MODE_SUM);
  vtkGetMacro(AccumulationMode, int);
  void SetAccumulationModeToMin();
  void SetAccumulationModeToMax();
  void SetAccumulationModeToSum();

  const char *GetAccumulationModeAsString();

protected:
  vtkImageConvolvePoints();
  ~vtkImageConvolvePoints();

  virtual int FillInputPortInformation(int port, vtkInformation* info);
  virtual int RequestInformation (vtkInformation *, 
                                  vtkInformationVector **, 
                                  vtkInformationVector *);
  virtual int RequestData(vtkInformation *, 
                          vtkInformationVector **, 
                          vtkInformationVector *);
  
  // Description:
  // Splat size influenced by scale factor.
  double ScaleFactor;
  
  // Description:
  // How to combine scalar values.
  int AccumulationMode;

  // Description:
  void SetScalar(int idx, double value, vtkDoubleArray *newScalars);

  // Desctiption:
  // Determines interpolated data value in convolution kernel at given point.
  // Sets value of inside to false if the point is outside the bounds of the kernel, 
  // true otherwise.
  double GetKernelValue(double point[3], double kernelOrigin[3], bool &inside);

private:
  
  // Description:
  // Data object containing points to convolve.
  vtkDataSet *InputPointData;
  
  // Description:
  // Convolution kernel.
  vtkImageData *ConvolutionKernel;

private:
  vtkImageConvolvePoints(const vtkImageConvolvePoints&);  // Not implemented.
  void operator=(const vtkImageConvolvePoints&);  // Not implemented.
};

#endif


