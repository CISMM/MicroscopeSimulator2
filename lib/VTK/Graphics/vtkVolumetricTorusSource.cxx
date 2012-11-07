#include "vtkVolumetricTorusSource.h"

#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkMath.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkUnstructuredGrid.h"

#include <math.h>

vtkStandardNewMacro(vtkVolumetricTorusSource);

vtkVolumetricTorusSource::vtkVolumetricTorusSource (int res)
{
  this->CrossSectionRadius = 0.1;
  this->RingRadius = 1.0;
  this->ThetaResolution = res;
  this->PhiResolution   = res;
  this->GenerateScalars = 1;

  this->SetNumberOfInputPorts(0);
}

void vtkVolumetricTorusSource::ComputePoint(double theta, double phi, double r, double result[3])
{
  double a = this->RingRadius + r*this->CrossSectionRadius*cos(phi);
  result[0] = a*cos(theta);
  result[1] = a*sin(theta);
  result[2] = r*this->CrossSectionRadius*sin(phi);  
}

void vtkVolumetricTorusSource::ComputeObjectCoordinates(double x[3], double result[3])
{
  double theta = atan2(x[1], x[0]);
  if (theta < 0.0)
    theta += 2.0 * vtkMath::Pi();
  
  double axisPt[3];
  this->ComputePoint(theta, 0.0, 0.0, axisPt);
  double dx = x[0] - axisPt[0];
  double dy = x[1] - axisPt[1];
  double rPlane = sqrt(x[0]*x[0] + x[1]*x[1]);

  double phi = atan2(x[2], rPlane - this->RingRadius);
  if (phi < 0.0)
    phi += 2.0 * vtkMath::Pi();

  double rFull  = sqrt(dx*dx + dy*dy + x[2]*x[2]); // This is the full hypotenuse length
  double r = rFull / this->CrossSectionRadius;

  result[0] = theta;
  result[1] = phi;
  result[2] = r;
}

void vtkVolumetricTorusSource::ComputeVelocityWRTCrossSectionRadius(double theta, double phi, double r, double result[3])
{
  result[0] = r*cos(phi)*cos(theta);
  result[1] = r*cos(phi)*sin(theta);
  result[2] = r*sin(phi);
}

void vtkVolumetricTorusSource::ComputeVelocityWRTRingRadius(double theta, double phi, double r, double result[3])
{
  result[0] = cos(theta);
  result[1] = sin(theta);
  result[2] = 0.0;
}

int vtkVolumetricTorusSource::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  double thetaAngle = 2.0 * vtkMath::Pi()/this->ThetaResolution;
  double phiAngle   = 2.0 * vtkMath::Pi()/this->PhiResolution;
  int numCells, numPts;
  double x[3];
  int i, j;
  vtkIdType pts[VTK_CELL_SIZE];
  vtkPoints *newPoints; 
  vtkCellArray *newCells;
  
  //
  // Set things up; allocate memory
  //

  numPts   = this->ThetaResolution*(this->PhiResolution+1);
  numCells = 3*this->ThetaResolution*this->PhiResolution;

  newPoints = vtkPoints::New();
  newPoints->Allocate(numPts);

  newCells = vtkCellArray::New();
  newCells->Allocate(newCells->EstimateSize(numCells,this->ThetaResolution));

  int skip = this->PhiResolution+1;

  //
  // Generate points and point data.
  //
  for (j=0; j<this->ThetaResolution; j++)
    {
    // x coordinate
    double theta = j*thetaAngle;

    for (i=0; i<this->PhiResolution; i++)
      {
      double phi   = i*phiAngle;
      this->ComputePoint(theta, phi, 1.0, x);
      newPoints->InsertPoint(j*skip+i, x);
      }

    // Point on the medial axis
    this->ComputePoint(theta, 0.0, 0.0, x);
    newPoints->InsertPoint(j*skip + this->PhiResolution,x);
    }

  //
  // Generate tetrahedral cells for volume.
  //
  for (i=0; i<this->PhiResolution; i++)
    {
    int i1Idx = i;
    int i2Idx = (i+1)%this->PhiResolution;
    int iInIdx = this->PhiResolution;

    for (j=0; j<this->ThetaResolution; j++)
      {
      int j1Idx = j;
      int j2Idx = (j+1)%this->ThetaResolution;
      int n0 = j1Idx*skip + i1Idx;
      int n1 = j2Idx*skip + i1Idx;
      int n2 = j2Idx*skip + iInIdx;
      int n3 = j1Idx*skip + iInIdx;
      int n4 = j2Idx*skip + i2Idx;
      int n5 = j1Idx*skip + i2Idx;

      // Tet 0
      pts[0] = n0;
      pts[1] = n3;
      pts[2] = n5;
      pts[3] = n1;
      newCells->InsertNextCell(4, pts);
      
      // Tet 1
      pts[0] = n5;
      pts[1] = n3;
      pts[2] = n4;
      pts[3] = n1;
      newCells->InsertNextCell(4, pts);
      
      // Tet 2
      pts[0] = n1;
      pts[1] = n4;
      pts[2] = n2;
      pts[3] = n3;
      newCells->InsertNextCell(4, pts);
      }
    }


  //
  // Update ourselves and release memory
  //
  output->SetPoints(newPoints);
  newPoints->Delete();

  newCells->Squeeze(); // since we've estimated size; reclaim some space
  output->SetCells(VTK_TETRA, newCells);
  newCells->Delete();
  
  return 1;
}

void vtkVolumetricTorusSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "CrossSectionRadius: " << this->CrossSectionRadius << "\n";
  os << indent << "RingRadius: " << this->RingRadius << "\n";
  os << indent << "ThetaResolution: " << this->ThetaResolution << "\n";
  os << indent << "PhiResolution: " << this->PhiResolution << "\n";
  os << indent << "GenerateScalars: " << (this->GenerateScalars ? "On\n" : "Off\n");
}
