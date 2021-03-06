#include "vtkDiskSource2.h"

#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkMath.h"
#include "vtkPointData.h"
#include "vtkPoints.h"

#include <math.h>

vtkStandardNewMacro(vtkDiskSource2);

vtkDiskSource2::vtkDiskSource2 (int res)
{
  this->Radius = 1.0;
  this->CircumferentialResolution = res;
  this->GenerateScalars = 1;

  this->SetNumberOfInputPorts(0);
}

void vtkDiskSource2::ComputePoint(double theta, double r, double result[3])
{
  result[0] = r*this->Radius*cos(theta);
  result[1] = r*this->Radius*sin(theta);
  result[2] = 0.0;
}

void vtkDiskSource2::ComputeObjectCoordinates(double x[3], double result[2])
{
  double theta = atan2(x[1], x[0]);
  if (theta < 0.0)
    theta += 2.0 * vtkMath::Pi();

  double r = sqrt(x[0]*x[0] + x[1]*x[1]) / this->Radius;;

  result[0] = theta;
  result[1] = r;
}

void vtkDiskSource2::ComputeVelocityWRTRadius(double theta, double r, double result[3])
{
  result[0] = r*cos(theta);
  result[1] = r*sin(theta);
  result[2] = 0.0;
}

int vtkDiskSource2::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  double thetaAngle = 2.0 * vtkMath::Pi()/this->CircumferentialResolution;
  int numCells, numPts;
  double x[3];
  int i;
  vtkIdType pts[VTK_CELL_SIZE];
  vtkPoints *newPoints; 
  vtkCellArray *newCells;
  
  //
  // Set things up; allocate memory
  //
  numPts   = this->CircumferentialResolution+1;
  numCells = this->CircumferentialResolution;

  newPoints = vtkPoints::New();
  newPoints->Allocate(numPts);

  newCells = vtkCellArray::New();
  newCells->Allocate(newCells->EstimateSize(numCells,this->CircumferentialResolution));

  //
  // Generate points and point data.
  //
  for (i=0; i<this->CircumferentialResolution; i++)
    {
    // x coordinate
    double theta = i*thetaAngle;
    this->ComputePoint(theta, 1.0, x);
    newPoints->InsertPoint(i, x);
    }

  this->ComputePoint(i*thetaAngle, 0.0, x);
  newPoints->InsertPoint(this->CircumferentialResolution, x);

  //
  // Generate triangle cells for volume.
  //
  for (i=0; i<this->CircumferentialResolution; i++)
    {
    pts[0] = i;
    pts[1] = (i+1) % this->CircumferentialResolution;
    pts[2] = this->CircumferentialResolution;
    newCells->InsertNextCell(3, pts);
    }

  //
  // Update ourselves and release memory
  //
  output->SetPoints(newPoints);
  newPoints->Delete();

  newCells->Squeeze(); // since we've estimated size; reclaim some space
  output->SetPolys(newCells);
  newCells->Delete();
  
  return 1;
}

void vtkDiskSource2::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Radius: " << this->Radius << "\n";
  os << indent << "CircumferentialResolution: " << this->CircumferentialResolution << "\n";
  os << indent << "GenerateScalars: " << (this->GenerateScalars ? "On\n" : "Off\n");
}
