/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkImageConvolvePoints.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImageConvolvePoints.h"

#include "vtkDoubleArray.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkMath.h"

#include <cmath>

vtkCxxRevisionMacro(vtkImageConvolvePoints, "$Revision: 1.4 $");
vtkStandardNewMacro(vtkImageConvolvePoints);

//----------------------------------------------------------------------------
vtkImageConvolvePoints::vtkImageConvolvePoints()
{
  this->SetNumberOfInputConnections(0, 1);

  this->InputPointData = NULL;
  this->ConvolutionKernel = NULL;
  this->ScaleFactor = 1.0;
  this->AccumulationMode = VTK_ACCUMULATION_MODE_SUM;
}

//----------------------------------------------------------------------------
vtkImageConvolvePoints::~vtkImageConvolvePoints()
{
  if (this->InputPointData != NULL)
  {
    this->InputPointData->Delete();
  }
  if (this->ConvolutionKernel != NULL)
  {
    this->ConvolutionKernel->Delete();
  }
}

//----------------------------------------------------------------------------
int vtkImageConvolvePoints::RequestInformation (
  vtkInformation * vtkNotUsed(request),
  vtkInformationVector ** vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{

  // Get the info objects.
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_DOUBLE, 1);
  
  return 1;
}

//----------------------------------------------------------------------------
int vtkImageConvolvePoints::RequestData(
  vtkInformation* vtkNotUsed( request ),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{ 
  // Get the data object.
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkImageData *output = vtkImageData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkImageData *input = vtkImageData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  // This initializes all field data and passes on existing cell data.
  output->CopyStructure( input );
  output->GetCellData()->PassData( input->GetCellData() );
  output->SetNumberOfScalarComponents(1);
  output->AllocateScalars();

  if (!input) {
    vtkErrorMacro(<<"No input!");
    return 1;
  }

  if (!this->InputPointData) {
    vtkErrorMacro(<<"No points to convolve!");
    return 1;
  }
  
  if (!this->ConvolutionKernel) {
    vtkErrorMacro(<<"No convolution kernel!");
    return 1;
  }

  vtkIdType numInputPts = this->InputPointData->GetNumberOfPoints();

    // Set up new double array to store convolution results.
  vtkDoubleArray *newScalars = 
    vtkDoubleArray::SafeDownCast(output->GetPointData()->GetScalars());
  
  // Copy point scalars from the input.
  newScalars->DeepCopy(input->GetPointData()->GetScalars());
  
  int *dims = input->GetDimensions();
  int sliceSize = dims[0] * dims[1];
  
  vtkDebugMacro(<< "Convolving data");

  //  Make sure points are available
  if (numInputPts < 1)
  {
    vtkDebugMacro(<<"No points to convolve!");
    return 1;
  }
  
  vtkIdType numImagePts = dims[0] * dims[1] * dims[2];
  vtkIdType i;
  
  // Traverse all points - splatting each into the volume.
  // For each point, determine which voxel it is in.  Then determine
  // the subvolume that the splat is contained in, and process that.
  int abortExecute = 0;
  vtkIdType progressInterval = numInputPts/20 + 1;
  for (vtkIdType ptId = 0; ptId < numInputPts && !abortExecute; ptId++)
  {
    
    if (!(ptId % progressInterval))
    {
      vtkDebugMacro(<<"Inserting point #" << ptId);
      this->UpdateProgress ((double)ptId/numInputPts);
      abortExecute = this->GetAbortExecute();
    }

    // Determine kernel footprint within image
    int imageMin[3], imageMax[3];
    double halfKernelSize, kernelOrig[3];
    double *point = this->InputPointData->GetPoint(ptId);
    for (int dim = 0; dim < 3; dim++)
    {
      halfKernelSize = 0.5 * this->ConvolutionKernel->GetSpacing()[dim] * 
             (this->ConvolutionKernel->GetDimensions()[dim] - 1.0);
      double orig = input->GetOrigin()[dim];
      kernelOrig[dim] = point[dim] - halfKernelSize;
      imageMin[dim] = (int) (floor((point[dim] - halfKernelSize - orig) /
      								input->GetSpacing()[dim]));
      imageMax[dim] = (int) (ceil((point[dim] + halfKernelSize - orig) /
      								input->GetSpacing()[dim]));
      if ( imageMin[dim] < 0 )
      { 
        imageMin[dim] = 0; 
      }
      if ( imageMax[dim] >= input->GetDimensions()[dim] )
      {
        imageMax[dim] = input->GetDimensions()[dim] - 1;
      }
    }

    // Loop over all sample points in image within kernel and
    // contribute interpolated kernel values to the image.
    double imagePt[3];
    for (int k = imageMin[2]; k <= imageMax[2]; k++)
    {
      imagePt[2] = input->GetOrigin()[2] + input->GetSpacing()[2]*k;
      for (int j = imageMin[1]; j <= imageMax[1]; j++)
      {
        imagePt[1] = input->GetOrigin()[1] + input->GetSpacing()[1]*j;
        for (i = imageMin[0]; i <= imageMax[0]; i++)
        {
          imagePt[0] = input->GetOrigin()[0] + input->GetSpacing()[0]*i;
          
          // Get interpolated value for this point from the kernel.
          bool success;
          double kernelValue = this->GetKernelValue(imagePt, kernelOrig, success);

          if (success)
          {
            vtkIdType idx = i + j*input->GetDimensions()[0] + k*sliceSize;
            this->SetScalar(idx, kernelValue * this->ScaleFactor, newScalars);
          }
        }
      }
    } // Within kernel footprint.
  } // Loop over all input points.
  
  return 1;
}
    
//----------------------------------------------------------------------------
void vtkImageConvolvePoints::SetScalar(int idx, double value, vtkDoubleArray *newScalars)
{
  double s = newScalars->GetValue(idx);
  switch (this->AccumulationMode)
    {
    case VTK_ACCUMULATION_MODE_MIN:
      newScalars->SetTuple(idx,(s < value ? &s : &value));
      break;
    case VTK_ACCUMULATION_MODE_MAX:
      newScalars->SetTuple(idx,(s > value ? &s : &value));
      break;
    case VTK_ACCUMULATION_MODE_SUM:
      s += value;
      newScalars->SetTuple(idx, &s);
      break;
    }
}

//----------------------------------------------------------------------------
double vtkImageConvolvePoints::GetKernelValue(double point[3], double kernelOrigin[3], bool &success) {

  if (this->ConvolutionKernel == NULL)
  {
    success = false;
    return 0.0;
  }

  // Get kernel bounds offset by origin
  double bounds[6], orig[3];
  vtkImageData *kernel = this->ConvolutionKernel;
  kernel->GetBounds(bounds);
  kernel->GetOrigin(orig);

  for (int dim = 0; dim < 3; dim++)
  {
    bounds[dim*2 + 0] = bounds[dim*2 + 0] - orig[dim] + kernelOrigin[dim];
    bounds[dim*2 + 1] = bounds[dim*2 + 1] - orig[dim] + kernelOrigin[dim];
  }

  // If the point is not within the kernel bounds, return early.
  for (int dim = 0; dim < 3; dim++)
  {
    if (point[dim] < bounds[dim*2 + 0] || point[dim] > bounds[dim*2 + 1])
    {
      success = false;
      return 0.0;
    }
  }

  // Okay, we are in the kernel. Now look up nearest neighbors to given point
  // in preparation for interpolation.
  int ijkOrig[3], ijkInc[3];
  double weights[3];
  for (int dim = 0; dim < 3; dim++)
  {
    double dimSpacing = kernel->GetSpacing()[dim];
    double dimLocation = (point[dim] - bounds[dim*2 + 0]) / dimSpacing;
    ijkOrig[dim] = (int) vtkMath::Floor(dimLocation);
    int kDim = kernel->GetDimensions()[dim]-1;
    ijkInc[dim] = (ijkOrig[dim] == kDim) ? ijkOrig[dim] : ijkOrig[dim] + 1;

    // Weight
    weights[dim] = dimLocation - vtkMath::Floor(dimLocation);
  }

  // Trilinear interpolation
  int kDim[3];
  kernel->GetDimensions(kDim);
  int kSlice = kDim[0] * kDim[1];

  vtkIdType ids[8];
  ids[0] = ijkOrig[0] + ijkOrig[1]*kDim[0] + ijkOrig[2]*kSlice;
  ids[1] = ijkInc [0] + ijkOrig[1]*kDim[0] + ijkOrig[2]*kSlice;
  ids[2] = ijkInc [0] + ijkInc [1]*kDim[0] + ijkOrig[2]*kSlice;
  ids[3] = ijkOrig[0] + ijkInc [1]*kDim[0] + ijkOrig[2]*kSlice;
  ids[4] = ijkOrig[0] + ijkOrig[1]*kDim[0] + ijkInc [2]*kSlice;
  ids[5] = ijkInc [0] + ijkOrig[1]*kDim[0] + ijkInc [2]*kSlice;
  ids[6] = ijkInc [0] + ijkInc [1]*kDim[0] + ijkInc [2]*kSlice;
  ids[7] = ijkOrig[0] + ijkInc [1]*kDim[0] + ijkInc [2]*kSlice;

  // Data values
  vtkDoubleArray *kernelValues = vtkDoubleArray::SafeDownCast(kernel->GetPointData()->GetScalars());
  double *kernelArray = static_cast<double *>(kernelValues->GetVoidPointer(0));
  double vals[8];
  for (int i = 0; i < 8; i++)
  {
    //vals[i] = kernelValues->GetComponent(ids[i], 0);
    vals[i] = kernelArray[ids[i]];
  }

  // Z-dimension first (0-4, 1-5, 2-6, 3-7)
  double zVals[4];
  zVals[0] = vals[0]*(1.0-weights[2]) + vals[4]*weights[2];
  zVals[1] = vals[1]*(1.0-weights[2]) + vals[5]*weights[2];
  zVals[2] = vals[2]*(1.0-weights[2]) + vals[6]*weights[2];
  zVals[3] = vals[3]*(1.0-weights[2]) + vals[7]*weights[2];

  // Y-dimension next (0-3, 1-2)
  double yVals[2];
  yVals[0] = zVals[0]*(1.0-weights[1]) + zVals[3]*weights[1];
  yVals[1] = zVals[1]*(1.0-weights[1]) + zVals[2]*weights[1];

  // X-dimension last
  double xVal = yVals[0]*(1.0-weights[0]) + yVals[1]*weights[0];

  success = true;
  return xVal;
}

//----------------------------------------------------------------------------
void vtkImageConvolvePoints::SetAccumulationModeToMin()
{
  this->SetAccumulationMode(VTK_ACCUMULATION_MODE_MIN);
}

//----------------------------------------------------------------------------
void vtkImageConvolvePoints::SetAccumulationModeToMax()
{
  this->SetAccumulationMode(VTK_ACCUMULATION_MODE_MAX);
}

//----------------------------------------------------------------------------
void vtkImageConvolvePoints::SetAccumulationModeToSum()
{
  this->SetAccumulationMode(VTK_ACCUMULATION_MODE_SUM);
}

//----------------------------------------------------------------------------
const char *vtkImageConvolvePoints::GetAccumulationModeAsString()
{
  if ( this->AccumulationMode == VTK_ACCUMULATION_MODE_MIN )
    {
    return "Minimum";
    }
  else if ( this->AccumulationMode == VTK_ACCUMULATION_MODE_MAX )
    {
    return "Maximum";
    }
  else
    {
    return "Sum";
    }
}

//----------------------------------------------------------------------------
void vtkImageConvolvePoints::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Input Point Data: " << this->InputPointData << "\n";
  os << indent << "Convolution Kernel: " << this->ConvolutionKernel << "\n";

  os << indent << "Accumulation Mode: " 
     << this->GetAccumulationModeAsString() << "\n";
}

//----------------------------------------------------------------------------
int vtkImageConvolvePoints::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
  return 1;
}
