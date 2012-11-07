/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkImageConstantSource.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImageConstantSource.h"

#include "vtkImageData.h"
#include "vtkImageProgressIterator.h"
#include "vtkMath.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"

vtkStandardNewMacro(vtkImageConstantSource);

//----------------------------------------------------------------------------
vtkImageConstantSource::vtkImageConstantSource()
{
  this->Constant = 0.0;
  this->Spacing[0] = 1.0; this->Spacing[1] = 1.0; this->Spacing[2] = 1.0;
  this->Origin[0] = 0.0;  this->Origin[1] = 0.0;  this->Origin[2] = 0.0;
  this->WholeExtent[0] = 0;  this->WholeExtent[1] = 255;
  this->WholeExtent[2] = 0;  this->WholeExtent[3] = 255;
  this->WholeExtent[4] = 0;  this->WholeExtent[5] = 0;
  this->SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
void vtkImageConstantSource::SetWholeExtent(int xMin, int xMax, 
                                         int yMin, int yMax,
                                         int zMin, int zMax)
{
  int modified = 0;
  
  if (this->WholeExtent[0] != xMin)
    {
    modified = 1;
    this->WholeExtent[0] = xMin ;
    }
  if (this->WholeExtent[1] != xMax)
    {
    modified = 1;
    this->WholeExtent[1] = xMax ;
    }
  if (this->WholeExtent[2] != yMin)
    {
    modified = 1;
    this->WholeExtent[2] = yMin ;
    }
  if (this->WholeExtent[3] != yMax)
    {
    modified = 1;
    this->WholeExtent[3] = yMax ;
    }
  if (this->WholeExtent[4] != zMin)
    {
    modified = 1;
    this->WholeExtent[4] = zMin ;
    }
  if (this->WholeExtent[5] != zMax)
    {
    modified = 1;
    this->WholeExtent[5] = zMax ;
    }
  if (modified)
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
int vtkImageConstantSource::RequestInformation (
  vtkInformation * vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed( inputVector ),
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  outInfo->Set(vtkDataObject::ORIGIN(), this->Origin, 3);
  outInfo->Set(vtkDataObject::SPACING(), this->Spacing, 3);
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
               this->WholeExtent,6);
  vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_DOUBLE, 1);
  return 1;
}

//----------------------------------------------------------------------------
void vtkImageConstantSource::ExecuteData(vtkDataObject *output,
                                         vtkInformation *outInfo)
{
  vtkImageData *data = this->AllocateOutputData(output, outInfo);
  
  if (data->GetScalarType() != VTK_DOUBLE)
    {
    vtkErrorMacro("Execute: This source only outputs doubles");
    }
  
  vtkImageProgressIterator<double> outIt(data, data->GetExtent(), this, 0);

  // Loop through ouput pixels
  while (!outIt.IsAtEnd())
    {
    double* outSI = outIt.BeginSpan();
    double* outSIEnd = outIt.EndSpan();
    while (outSI != outSIEnd)
      {
      // now process the components
      *outSI = this->Constant;
      outSI++;
      }
    outIt.NextSpan();
    }
}

//----------------------------------------------------------------------------
void vtkImageConstantSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Constant: " << this->Constant << "\n";
  os << indent << "Origin: (" << this->Origin[0] << ", "
                              << this->Origin[1] << ", "
                              << this->Origin[2] << ")\n";
  os << indent << "Spacing: (" << this->Spacing[0] << ", "
                               << this->Spacing[1] << ", "
                               << this->Spacing[2] << ")\n";
  os << indent << "WholeExtent: (" << WholeExtent[0];
  for (int idx = 1; idx < 6; ++idx)
    {
    os << ", " << WholeExtent[idx];
    }
  os << ")\n";
}
