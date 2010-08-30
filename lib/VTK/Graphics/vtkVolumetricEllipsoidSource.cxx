#include "vtkVolumetricEllipsoidSource.h"

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

vtkCxxRevisionMacro(vtkVolumetricEllipsoidSource, "$Revision: 1.55 $");
vtkStandardNewMacro(vtkVolumetricEllipsoidSource);

vtkVolumetricEllipsoidSource::vtkVolumetricEllipsoidSource (int res)
{
  this->ThetaResolution = res;
  this->PhiResolution   = res;
  this->Radius[0] = this->Radius[1] = this->Radius[2] = 1.0;
  this->GenerateScalars = 1;

  this->SetNumberOfInputPorts(0);
}

void vtkVolumetricEllipsoidSource::ComputePoint(double theta, double phi, double r, double result[3])
{
  result[0] = r*this->Radius[0]*cos(theta)*sin(phi);
  result[1] = r*this->Radius[1]*sin(theta)*sin(phi);
  result[2] = r*this->Radius[2]*cos(phi);
}

void vtkVolumetricEllipsoidSource::ComputeObjectCoordinates(double x[3], double result[3])
{
  double theta = atan2(x[1], x[0]);
  if (theta < 0.0)
    theta += vtkMath::DoubleTwoPi();
  double r = sqrt(x[0]*x[0] + x[1]*x[1] + x[2]*x[2]);
  double phi = acos(x[2] / r);

  double bdryPt[3];
  ComputePoint(result[0], result[1], 1.0, bdryPt);
  double bdryRadius = sqrt(bdryPt[0]*bdryPt[0] + bdryPt[1]*bdryPt[1] + bdryPt[2]*bdryPt[2]);
  double t = r / bdryRadius;

  result[0] = theta;
  result[1] = phi;
  result[2] = t;
}

void vtkVolumetricEllipsoidSource::ComputeVelocityWRTRadiusX(double theta, double phi, double r, double result[3])
{
  result[0] = r*cos(theta)*sin(phi);
  result[1] = 0.0;
  result[2] = 0.0;
}

void vtkVolumetricEllipsoidSource::ComputeVelocityWRTRadiusY(double theta, double phi, double r, double result[3])
{
  result[0] = 0.0;
  result[1] = r*sin(theta)*sin(phi);
  result[2] = 0.0;
}

void vtkVolumetricEllipsoidSource::ComputeVelocityWRTRadiusZ(double theta, double phi, double r, double result[3])
{
  result[0] = 0.0;
  result[1] = 0.0;
  result[2] = r*cos(phi);
}

int vtkVolumetricEllipsoidSource::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  double thetaAngle = vtkMath::DoubleTwoPi()/this->ThetaResolution;
  double phiAngle   = vtkMath::DoublePi()/(this->PhiResolution-1);
  int numCells, numPts;
  double x[3];
  int i, j;
  vtkIdType pts[VTK_CELL_SIZE];
  vtkPoints *newPoints; 
  vtkCellArray *newCells;
  
  //
  // Set things up; allocate memory
  //

  numPts   = (this->ThetaResolution+1)*(this->PhiResolution-1) + 2;
  numCells = 3*this->ThetaResolution*(this->PhiResolution-2) + 2*this->ThetaResolution;

  newPoints = vtkPoints::New();
  newPoints->Allocate(numPts);

  newCells = vtkCellArray::New();
  newCells->Allocate(newCells->EstimateSize(numCells,this->ThetaResolution));

  int skip = this->ThetaResolution+1;

  //
  // Generate points and point data for all parts of the ellipsoid except for the
  // north and south poles.
  //

  for (i=1; i<this->PhiResolution-1; i++)
    {
    double phi   = i*phiAngle;

    for (j=0; j<this->ThetaResolution; j++)
      {
      // x coordinate
      double theta = j*thetaAngle;
      this->ComputePoint(theta, phi, 1.0, x);
      newPoints->InsertPoint((i-1)*skip+j, x);
      }

    // Point on the central axis
    x[0] = x[1] = 0.0;
    x[2] = this->Radius[2]*cos(phi);
    newPoints->InsertPoint((i-1)*skip + this->ThetaResolution,x);
    }

  //
  // South and north poles
  //
  this->ComputePoint(0.0, vtkMath::DoublePi(), 1.0, x);
  newPoints->InsertPoint(numPts-2,x);

  this->ComputePoint(0.0, 0.0, 1.0, x);
  newPoints->InsertPoint(numPts-1,x);

  //
  // Start by creating the tetrahedra at the north and south polls.
  //
  for (i=0; i<this->ThetaResolution;i++)
    {
    // South pole
    pts[0] = numPts-2;
    pts[1] = (this->PhiResolution-3)*skip + i;
    pts[2] = (this->PhiResolution-3)*skip + ((i+1) % this->ThetaResolution);
    pts[3] = (this->PhiResolution-3)*skip + this->ThetaResolution;
    newCells->InsertNextCell(4, pts);

    // North pole
    pts[0] = numPts-1;
    pts[1] = (i+1) % this->ThetaResolution;
    pts[2] = i;
    pts[3] = this->ThetaResolution;
    newCells->InsertNextCell(4, pts);
    }

  //
  // Generate tetrahedral cells for volume.
  //
  for (i=0; i<this->PhiResolution-3; i++)
    {

    for (j=0; j<this->ThetaResolution; j++)
      {
      int n0 = (i  )*skip + j;
      int n1 = (i+1)*skip + j;
      int n2 = (i+1)*skip + this->ThetaResolution;
      int n3 = (i  )*skip + this->ThetaResolution;
      int n4 = (i+1)*skip + ((j+1) % this->ThetaResolution);
      int n5 = (i  )*skip + ((j+1) % this->ThetaResolution);

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

void vtkVolumetricEllipsoidSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Radius: (" << this->Radius[0] << ", " << this->Radius[1] << ", "
     << this->Radius[2] << ")\n";
  os << indent << "ThetaResolution: " << this->ThetaResolution << "\n";
  os << indent << "PhiResolution: " << this->PhiResolution << "\n";
  os << indent << "GenerateScalars: " << (this->GenerateScalars ? "On\n" : "Off\n");
}
