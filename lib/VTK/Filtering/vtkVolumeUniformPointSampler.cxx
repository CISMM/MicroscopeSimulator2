/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVolumeUniformPointSampler.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkVolumeUniformPointSampler.h"

#include "vtkCellArray.h"
#include "vtkDoubleArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkMinimalStandardRandomSequence.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkTetra.h"

#include <cstdlib>
#include <ctime>

vtkStandardNewMacro(vtkVolumeUniformPointSampler);

//----------------------------------------------------------------------------
vtkVolumeUniformPointSampler::vtkVolumeUniformPointSampler()
{
  // by default assume filters have one input and one output
  // subclasses that deviate should modify this setting
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
  this->Density = 0.0;
  this->Volume = 0.0;
  this->TetrahedraVolumes = vtkDoubleArray::New();
  this->Register(this->TetrahedraVolumes);
}

//----------------------------------------------------------------------------
vtkVolumeUniformPointSampler::~vtkVolumeUniformPointSampler()
{
  this->UnRegister(this->TetrahedraVolumes);
  this->TetrahedraVolumes = NULL;
}

//----------------------------------------------------------------------------
int vtkVolumeUniformPointSampler::FillInputPortInformation(int port, vtkInformation *info) {
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkUnstructuredGrid");
  return 1;
}

//----------------------------------------------------------------------------
void vtkVolumeUniformPointSampler::PrintSelf(ostream& os, vtkIndent indent)
{
  // TODO - fill this in
}

//----------------------------------------------------------------------------
// This is the superclasses style of Execute method.  Convert it into
// an imaging style Execute method.
int vtkVolumeUniformPointSampler::RequestData(
  vtkInformation* request,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // Set the RNG seed
  this->Random->SetSeed( time( NULL ) );

  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // Clean up the weights and output object.
  this->TetrahedraVolumes->Initialize();
  output->Initialize();

  // Calculate weights for each polygon.
  this->ComputeTetrahedraVolumes(input);

  // Create the sample points
  this->CreateSamplePoints(input, output);

  this->ComputedTime = this->GetMTime();
    
  return 1;
}

//----------------------------------------------------------------------------
void vtkVolumeUniformPointSampler::ComputeTetrahedraVolumes(vtkUnstructuredGrid* input) {
  vtkIdType npts, *pts;
  vtkCellArray* tetrahedra = input->GetCells();

  // Invariant: at the completion of each iteration, the current tetrahedron
  // is associated with the total volume computed so far.
  // This will be used to randomly select a tetrahedron for point sampling using
  // an uniform distribution.
  int cellId = 0;
  this->Volume = 0.0;
  for (tetrahedra->InitTraversal(); tetrahedra->GetNextCell(npts, pts); ) {
    double p0[3], p1[3], p2[3], p3[3];
    if (npts == 4) {
      input->GetPoint(pts[0], p0);
      input->GetPoint(pts[1], p1);
      input->GetPoint(pts[2], p2);
      input->GetPoint(pts[3], p3);
      double volume = vtkTetra::ComputeVolume(p0, p1, p2, p3);
      if (volume <= 0.0) {
        std::cout << "Non-positive volume in element ID " << cellId
                  << ". Volume: " << volume << std::endl;
      }
      //this->Volume += vtkTetra::ComputeVolume(p0, p1, p2, p3);
      this->Volume += volume;
      this->TetrahedraVolumes->InsertNextValue(this->Volume);
    } else {
      vtkDebugMacro(<< "npts != 4");
    }
    cellId++;
  }
}

//----------------------------------------------------------------------------
void vtkVolumeUniformPointSampler::CreateSamplePoints(vtkUnstructuredGrid *input, 
                                                vtkPolyData *output) {
  // Set up new points.
  int numPoints = this->NumberOfSamples;
  if (!this->UseFixedNumberOfSamples)
    numPoints = this->ComputeNumberOfSamples(this->Volume, this->Density);
  if (this->Volume <= 0.0)
    numPoints = 0;

  output->Allocate(numPoints, 10);
  vtkPoints *newPoints = vtkPoints::New();
  newPoints->SetNumberOfPoints(numPoints);

  // Insert the appropriate number of points.
  for (int j = 0; j < numPoints; j++) {

    // Get a random number. Probably doesn't matter how good it is.
    this->Random->Next();
    double rand = this->Random->GetValue() * this->Volume;

    // Map this number to a polygon index via a binary search.
    vtkIdType tetraIndex = -1;    
    int start = 0;
    int finish = this->TetrahedraVolumes->GetNumberOfTuples() - 1;
    while (finish - start > 1) {
      int mid = ((finish - start + 1) / 2) + start;
      double weight = this->TetrahedraVolumes->GetTuple(mid)[0];
      rand <= weight ? finish = mid : start = mid;
    }

    // Final decision.
    if (rand <= this->TetrahedraVolumes->GetTuple(start)[0])
      tetraIndex = start;
    else
      tetraIndex = finish;

    // Get the polygon points and compute the new point to insert
    vtkTetra *tetra = vtkTetra::SafeDownCast(input->GetCell(tetraIndex));
    if (!tetra) {
      vtkErrorMacro(<< "Could not downcast to tetrahedra with " << input->GetCell(tetraIndex)->GetNumberOfPoints() << " points to vtkTetra");
      continue;
    }
    double randomPoint[3];
    this->RandomTetrahedronPoint(tetra, input, randomPoint);

    newPoints->InsertPoint(j, randomPoint);
    vtkIdType id = static_cast<vtkIdType>(j);
    output->InsertNextCell(static_cast<int>(VTK_VERTEX), 1, &id);
  }

  output->SetPoints(newPoints);
  newPoints->Delete();
}

//----------------------------------------------------------------------------
void vtkVolumeUniformPointSampler::RandomTetrahedronPoint(vtkTetra *tetra, 
                                                  vtkUnstructuredGrid *grid, 
                                                  double *pt) {
  // Map to real coordinate.
  double p1[3], p2[3], p3[3], p4[3];
  grid->GetPoint(tetra->GetPointId(0), p1);
  grid->GetPoint(tetra->GetPointId(1), p2);
  grid->GetPoint(tetra->GetPointId(2), p3);
  grid->GetPoint(tetra->GetPointId(3), p4);

  // Get coordinates that give a uniformly random
  // distribution in the space contained by the
  // tetrahedron.
  this->Random->Next();
  double s = pow(this->Random->GetValue(), 1.0/3.0);
  this->Random->Next();
  double t = sqrt(this->Random->GetValue());
  this->Random->Next();
  double r = this->Random->GetValue();

  // Get triangle
  double ux = s*(p2[0] - p1[0]) + p1[0];
  double uy = s*(p2[1] - p1[1]) + p1[1];
  double uz = s*(p2[2] - p1[2]) + p1[2];
  double vx = s*(p3[0] - p1[0]) + p1[0];
  double vy = s*(p3[1] - p1[1]) + p1[1];
  double vz = s*(p3[2] - p1[2]) + p1[2];
  double wx = s*(p4[0] - p1[0]) + p1[0];
  double wy = s*(p4[1] - p1[1]) + p1[1];
  double wz = s*(p4[2] - p1[2]) + p1[2];

  // Get points defining line
  double ax = t*(vx - ux) + ux;
  double ay = t*(vy - uy) + uy;
  double az = t*(vz - uz) + uz;
  double bx = t*(wx - ux) + ux;
  double by = t*(wy - uy) + uy;
  double bz = t*(wz - uz) + uz;
  
  // Linear interpolation on line produces final sample
  pt[0] = ((1.0 - r)*ax) + (r*bx);
  pt[1] = ((1.0 - r)*ay) + (r*by);
  pt[2] = ((1.0 - r)*az) + (r*bz);

}

//----------------------------------------------------------------------------
int vtkVolumeUniformPointSampler::ComputeNumberOfSamples(double volume, double density) {
  double doublePoints = volume * density;
  int numPoints = static_cast<int>(doublePoints + 0.5);
  return numPoints;
}
