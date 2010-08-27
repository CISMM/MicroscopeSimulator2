#include "vtkVolumetricCylinderSource.h"

#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkUnstructuredGrid.h"

#include <math.h>

vtkCxxRevisionMacro(vtkVolumetricCylinderSource, "$Revision: 1.55 $");
vtkStandardNewMacro(vtkVolumetricCylinderSource);

vtkVolumetricCylinderSource::vtkVolumetricCylinderSource (int res)
{
  this->Resolution = res;
  this->Height = 1.0;
  this->Radius = 0.5;
  this->GenerateScalars = 1;

  this->SetNumberOfInputPorts(0);
}

void vtkVolumetricCylinderSource::ComputePoint(double t, double theta, double r, double result[3])
{
  result[0] = r*this->Radius*cos(theta);
  result[1] = (t-0.5)*this->Height;
  result[2] = r*this->Radius*sin(theta);
}

void vtkVolumetricCylinderSource::ComputeObjectCoordinates(double x[3], double result[3])
{
  result[0] = x[1] / this->Height + 0.5;
  result[1] = atan2(x[2],x[0]);
  if (result[1] < 0.0)
    result[1] += vtkMath::DoubleTwoPi();

  double r = sqrt(x[0]*x[0] + x[2]*x[2]);
  result[2] = r / this->Radius;
}

void vtkVolumetricCylinderSource::ComputeVelocityWRTHeight(double t, double theta, double r, double result[3])
{
  result[0] = 0.0;
  result[1] = t-0.5;
  result[2] = 0.0;
}

void vtkVolumetricCylinderSource::ComputeVelocityWRTRadius(double t, double theta, double r, double result[3])
{
  result[0] = r*cos(theta);
  result[1] = 0.0;
  result[2] = r*sin(theta);
}

int vtkVolumetricCylinderSource::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  double angle= vtkMath::DoubleTwoPi()/this->Resolution;
  int numCells, numPts;
  double xtop[3], xbot[3];
  int i, idx;
  vtkIdType pts[VTK_CELL_SIZE];
  vtkPoints *newPoints; 
  vtkCellArray *newCells;
  
  //
  // Set things up; allocate memory
  //

  numPts   = 2*this->Resolution + 2;
  numCells = 3*this->Resolution;

  newPoints = vtkPoints::New();
  newPoints->Allocate(numPts);

  newCells = vtkCellArray::New();
  newCells->Allocate(newCells->EstimateSize(numCells,this->Resolution));
  //
  // Generate points and point data for sides
  //
  for (i=0; i<this->Resolution; i++)
    {
    this->ComputePoint(0.0, i*angle, 1.0, xtop);
    this->ComputePoint(1.0, i*angle, 1.0, xbot);

    idx = 2*i;
    newPoints->InsertPoint(idx,xtop);
    newPoints->InsertPoint(idx+1,xbot);
    }

  //
  // Add two points at either end of the axis
  //
  this->ComputePoint(0.0, 0.0, 0.0, xtop);
  this->ComputePoint(1.0, 0.0, 0.0, xbot);
  newPoints->InsertPoint(numPts-2,xtop);
  newPoints->InsertPoint(numPts-1,xbot);
  
  //
  // Generate tetrahedral cells for volume. We (conceptually) loop over
  // wedges, then subdivide the wedges into tetrahedra.
  //
  for (i=0; i<this->Resolution; i++)
    {
    // Tetra 1
    pts[0] = 2*i;
    pts[1] = (2*(i+1)) % (2*this->Resolution);
    pts[2] = 2*i+1;
    pts[3] = numPts-2;
    newCells->InsertNextCell(4,pts);

    // Tetra 2
    pts[0] = 2*i+1;
    pts[1] = numPts-2;
    pts[2] = numPts-1;
    pts[3] = (2*(i+1)+1) % (2*this->Resolution);
    newCells->InsertNextCell(4,pts);

    // Tetra 3
    pts[0] = (2*(i+1)) % (2*this->Resolution);
    pts[1] = numPts-2;
    pts[2] = (2*(i+1)+1) % (2*this->Resolution);
    pts[3] = 2*i+1;
    newCells->InsertNextCell(4,pts);
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

void vtkVolumetricCylinderSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "Resolution: " << this->Resolution << "\n";
  os << indent << "Height: " << this->Height << "\n";
  os << indent << "Radius: " << this->Radius << "\n";
  os << indent << "GenerateScalars: " << (this->GenerateScalars ? "On\n" : "Off\n");
}
