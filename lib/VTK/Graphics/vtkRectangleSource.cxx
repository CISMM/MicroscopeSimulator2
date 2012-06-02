#include "vtkRectangleSource.h"

#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkMath.h"
#include "vtkPointData.h"
#include "vtkPoints.h"

#include <math.h>

vtkCxxRevisionMacro(vtkRectangleSource, "$Revision: 1.55 $");
vtkStandardNewMacro(vtkRectangleSource);

vtkRectangleSource::vtkRectangleSource (int res)
{
  this->Width = 1.0;
  this->Height = 1.0;
  this->WidthResolution = 1;
  this->HeightResolution = 1;
  this->GenerateScalars = 1;

  this->SetNumberOfInputPorts(0);
}

void vtkRectangleSource::ComputePoint(double u, double v, double result[3])
{
  result[0] = (u-0.5)*this->Width;
  result[1] = (v-0.5)*this->Height;
  result[2] = 0.0;
}

void vtkRectangleSource::ComputeObjectCoordinates(double x[3], double result[2])
{
  double u = x[0] / this->Width  + 0.5;
  double v = x[1] / this->Height + 0.5;

  result[0] = u;
  result[1] = v;
}

void vtkRectangleSource::ComputeVelocityWRTWidth(double u, double v, double result[3])
{
  result[0] = u-0.5;
  result[1] = 0.0;
  result[2] = 0.0;
}

void vtkRectangleSource::ComputeVelocityWRTHeight(double u, double v, double result[3])
{
  result[0] = 0.0;
  result[1] = v-0.5;
  result[2] = 0.0;
}

int vtkRectangleSource::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  int numCells, numPts;
  double x[3];
  int i, j;
  vtkIdType pts[VTK_CELL_SIZE];
  vtkPoints *newPoints; 
  vtkCellArray *newCells;
  
  //
  // Set things up; allocate memory
  //
  numPts   = (this->WidthResolution+1)*(this->HeightResolution+1);
  numCells = this->WidthResolution*this->HeightResolution;

  newPoints = vtkPoints::New();
  newPoints->Allocate(numPts);

  newCells = vtkCellArray::New();
  newCells->Allocate(newCells->EstimateSize(numCells,this->WidthResolution));

  //
  // Generate points and point data.
  //
  int skip = this->WidthResolution+1;
  double uIncr = 1.0 / static_cast<double>(this->WidthResolution);
  double vIncr = 1.0 / static_cast<double>(this->HeightResolution);
  for (j=0; j<this->HeightResolution+1; j++)
    {
    double v = j*vIncr;
    for (i=0; i<this->WidthResolution+1; i++)
      {
      double u = i*uIncr;
      this->ComputePoint(u, v, x);
      newPoints->InsertPoint(j*skip+i, x);
      }
    }

  //
  // Generate triangle cells for volume.
  //
  for (j=0; j<this->HeightResolution; j++)
    {
    for (i=0; i<this->WidthResolution; i++)
      {
      int n0 = j    *skip + i;
      int n1 = j    *skip + (i+1);
      int n2 = (j+1)*skip + (i+1);
      int n3 = (j+1)*skip + i;

      pts[0] = n0;
      pts[1] = n1;
      pts[2] = n2;
      newCells->InsertNextCell(3, pts);

      pts[0] = n2;
      pts[1] = n3;
      pts[2] = n0;
      newCells->InsertNextCell(3, pts);
      }
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

void vtkRectangleSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Width: " << this->Width << "\n";
  os << indent << "Height: " << this->Height << "\n";
  os << indent << "WidthResolution: " << this->WidthResolution << "\n";
  os << indent << "HeightResolution: " << this->HeightResolution << "\n";
  os << indent << "GenerateScalars: " << (this->GenerateScalars ? "On\n" : "Off\n");
}
