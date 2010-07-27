/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPointRingSource.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPointRingSource.h"

#include "vtkCellArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"

vtkCxxRevisionMacro(vtkPointRingSource, "$Revision: 1.65 $");
vtkStandardNewMacro(vtkPointRingSource);


// Construct a ring about the x-axis with radius 1 and 5 vertices.
vtkPointRingSource::vtkPointRingSource() {
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
  this->Radius = 1.0;
  this->NumberOfPoints = 5;
}


int vtkPointRingSource::RequestInformation(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector) {
  
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  outInfo->Set(vtkStreamingDemandDrivenPipeline::MAXIMUM_NUMBER_OF_PIECES(),
               -1);

  return 1;
}


int vtkPointRingSource::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector) {

  // Get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  if (outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()) > 0) {
    return 1;
  }

  // Get the ouptut
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPoints* newPoints = vtkPoints::New();
  newPoints->Allocate(this->NumberOfPoints);

  vtkCellArray* newPolys = vtkCellArray::New();
  newPolys->Allocate(newPolys->EstimateSize(this->NumberOfPoints,1));

  // Generate points
  double dTheta = (vtkMath::DoubleTwoPi()) / static_cast<double>(this->NumberOfPoints);
  for (int i = 0; i < this->NumberOfPoints; i++) {
    double pt[3];
    pt[0] = 0.0;
    pt[1] = this->Radius * cos(static_cast<double>(i)*dTheta);
    pt[2] = this->Radius * sin(static_cast<double>(i)*dTheta);

    newPoints->InsertPoint(i,pt);
    vtkIdType id = static_cast<vtkIdType>(i);
    newPolys->InsertNextCell(1, &id);
  }


  output->SetPoints(newPoints);
  newPoints->Delete();

  output->SetPolys(newPolys);
  newPolys->Delete();

  return 1;
}


void vtkPointRingSource::PrintSelf(ostream& os, vtkIndent indent) {
  // TODO
}


void vtkPointRingSource::SetNumberOfPoints(int points) {
  if (this->NumberOfPoints != points) {

    this->NumberOfPoints = points;

    if (this->NumberOfPoints < 1)
      this->NumberOfPoints = 1;

    this->Modified();
  }
}
