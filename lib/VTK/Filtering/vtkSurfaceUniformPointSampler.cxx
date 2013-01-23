/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSurfaceUniformPointSampler.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSurfaceUniformPointSampler.h"

#include "vtkCellArray.h"
#include "vtkDoubleArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkMinimalStandardRandomSequence.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkTriangle.h"
#include <cstdlib>

vtkStandardNewMacro(vtkSurfaceUniformPointSampler);

//----------------------------------------------------------------------------
vtkSurfaceUniformPointSampler::vtkSurfaceUniformPointSampler()
{
  // by default assume filters have one input and one output
  // subclasses that deviate should modify this setting
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
  this->Density = 0.0;
  this->SurfaceArea = 0.0;
  this->PolygonAreas = vtkDoubleArray::New();
  this->Register(this->PolygonAreas);
}

//----------------------------------------------------------------------------
vtkSurfaceUniformPointSampler::~vtkSurfaceUniformPointSampler()
{
  this->UnRegister(this->PolygonAreas);
  this->PolygonAreas = NULL;
}

//----------------------------------------------------------------------------
void vtkSurfaceUniformPointSampler::PrintSelf(ostream& os, vtkIndent indent)
{
  // TODO - fill in
}

//----------------------------------------------------------------------------
// This is the superclasses style of Execute method.  Convert it into
// an imaging style Execute method.
int vtkSurfaceUniformPointSampler::RequestData(
  vtkInformation* request,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // Set the RNG seed
  this->Random->SetSeed( time( NULL ) );

  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // Clean up the weights and output object.
  this->PolygonAreas->Initialize();
  output->Initialize();

  // Calculate weights for each polygon.
  this->ComputePolygonAreas(input);

  // Create the sample points
  this->CreateSamplePoints(input, output);

  this->ComputedTime = this->GetMTime();
    
  return 1;
}

//----------------------------------------------------------------------------
void vtkSurfaceUniformPointSampler::ComputePolygonAreas(vtkPolyData* input) {
  vtkIdType npts, *pts;
  vtkCellArray* triangles = input->GetPolys();

  // Invariant: at the completion of each iteration, the current polygon
  // is associated with the total area computed so far.
  // This will be used to randomly select a polygon for point sampling using
  // an uniform distribution.
  this->SurfaceArea = 0.0;
  for (triangles->InitTraversal(); triangles->GetNextCell(npts, pts); ) {
    double p1[3], p2[3], p3[3];
    if (npts == 3) {
      input->GetPoint(pts[0], p1);
      input->GetPoint(pts[1], p2);
      input->GetPoint(pts[2], p3);
      this->SurfaceArea += vtkTriangle::TriangleArea(p1, p2, p3);
      this->PolygonAreas->InsertNextValue(this->SurfaceArea);
    } else {
      vtkDebugMacro(<< "npts != 3");
    }
  }
}

//----------------------------------------------------------------------------
void vtkSurfaceUniformPointSampler::CreateSamplePoints(vtkPolyData *input, 
                                                vtkPolyData *output) {
  // Set up new points.
  int numPoints = this->NumberOfSamples;
  if (!this->UseFixedNumberOfSamples)
    numPoints = this->ComputeNumberOfSamples(this->SurfaceArea, this->Density);
  output->Allocate(numPoints, 10);
  vtkPoints *newPoints = vtkPoints::New();
  newPoints->SetNumberOfPoints(numPoints);

  // Insert the appropriate number of points.
  for (int j = 0; j < numPoints; j++) {

    // Get a random number. Probably doesn't matter how good it is.
    this->Random->Next();
    double rand = this->Random->GetValue() * this->SurfaceArea;

    // Map this number to a polygon index via a binary search.
    vtkIdType polygonIndex = -1;    
    int start = 0;
    int finish = this->PolygonAreas->GetNumberOfTuples() - 1;
    while (finish - start > 1) {
      int mid = ((finish - start + 1) / 2) + start;
      double weight = this->PolygonAreas->GetTuple(mid)[0];
      rand <= weight ? finish = mid : start = mid;
    }

    // Final decision.
    if (rand <= this->PolygonAreas->GetTuple(start)[0])
      polygonIndex = start;
    else
      polygonIndex = finish;

    // Get the polygon points and compute the new point to insert
    vtkTriangle *triangle = vtkTriangle::SafeDownCast(input->GetCell(polygonIndex));
    if (!triangle) {
      vtkErrorMacro(<< "Could not downcast polygon with " << input->GetCell(polygonIndex)->GetNumberOfPoints() << " points to vtkTriangle");
      continue;
    }

    double randomPoint[3];
    this->RandomTrianglePoint(triangle, input, randomPoint);

    newPoints->InsertPoint(j, randomPoint);
    //output->InsertNextCell(VTK_VERTEX, 1, &j);
    vtkIdType id = static_cast<vtkIdType>(j);
    output->InsertNextCell(static_cast<int>(VTK_VERTEX), 1, &id);
  }

  output->SetPoints(newPoints);
  newPoints->Delete();
}

//----------------------------------------------------------------------------
void vtkSurfaceUniformPointSampler::RandomTrianglePoint(vtkTriangle *triangle, 
                                                 vtkPolyData *polyData, 
                                                 double *pt) {
  // Parameterize two edges u = p3-p1, v = p2-p1 by value t in range
  // [0,1]. We compute the inverted function of the area of the triangle
  // formed by u(t), v(t) to map a uniform random sample to the CDF
  // as determined by the triangle area.
  double p1[3], p2[3], p3[3];
  polyData->GetPoint(triangle->GetPointId(0), p1);
  polyData->GetPoint(triangle->GetPointId(1), p2);
  polyData->GetPoint(triangle->GetPointId(2), p3);

  double ux = p3[0] - p1[0];
  double uy = p3[1] - p1[1];
  double uz = p3[2] - p1[2];
  double vx = p2[0] - p1[0];
  double vy = p2[1] - p1[1];
  double vz = p2[2] - p1[2];

  this->Random->Next();
  double t = sqrt( this->Random->GetValue() );
  
  // Now that we know t, get a random point on the line
  // connecting edges u and v.
  double utx = ux*t + p1[0];
  double uty = uy*t + p1[1];
  double utz = uz*t + p1[2];
  double vtx = vx*t + p1[0];
  double vty = vy*t + p1[1];
  double vtz = vz*t + p1[2];

  this->Random->Next();
  double s = this->Random->GetValue();

  pt[0] = (utx*(1.0-s)) + (vtx*s);
  pt[1] = (uty*(1.0-s)) + (vty*s);
  pt[2] = (utz*(1.0-s)) + (vtz*s);
}

//----------------------------------------------------------------------------
int vtkSurfaceUniformPointSampler::ComputeNumberOfSamples(double surfaceArea, double density) {
  double doublePoints = surfaceArea * density;
  int numPoints = static_cast<int>(doublePoints + 0.5);
  return numPoints;
}
