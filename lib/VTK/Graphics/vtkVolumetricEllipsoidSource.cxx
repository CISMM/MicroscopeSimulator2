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
  int i, j, idx;
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

      x[0] = this->Radius[0]*cos(theta)*sin(phi);
      x[1] = this->Radius[1]*sin(theta)*sin(phi);
      x[2] = this->Radius[2]*cos(phi);

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
  x[0] = x[1] = 0.0;
  x[2] = -this->Radius[2];
  newPoints->InsertPoint(numPts-2,x);
  x[2] =  this->Radius[2];
  newPoints->InsertPoint(numPts-1,x);

  vtkIdType tetPtIds[VTK_CELL_SIZE];

  //
  // Start by creating the tetrahedra at the north and south polls.
  //
  for (i=0; i<this->ThetaResolution;i++)
    {
    // South pole
    tetPtIds[0] = numPts-2;
    tetPtIds[1] = (this->PhiResolution-3)*skip + i;
    tetPtIds[2] = (this->PhiResolution-3)*skip + ((i+1) % this->ThetaResolution);
    tetPtIds[3] = (this->PhiResolution-3)*skip + this->ThetaResolution;
    newCells->InsertNextCell(4, tetPtIds);

    // North pole
    tetPtIds[0] = numPts-1;
    tetPtIds[1] = i;
    tetPtIds[2] = (i+1) % this->ThetaResolution;
    tetPtIds[3] = this->ThetaResolution;
    newCells->InsertNextCell(4, tetPtIds);
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
      tetPtIds[0] = n0;
      tetPtIds[1] = n3;
      tetPtIds[2] = n5;
      tetPtIds[3] = n1;
      newCells->InsertNextCell(4, tetPtIds);

      // Tet 1
      tetPtIds[0] = n5;
      tetPtIds[1] = n3;
      tetPtIds[2] = n4;
      tetPtIds[3] = n1;
      newCells->InsertNextCell(4, tetPtIds);
      
      // Tet 2
      tetPtIds[0] = n1;
      tetPtIds[1] = n4;
      tetPtIds[2] = n2;
      tetPtIds[3] = n3;
      newCells->InsertNextCell(4, tetPtIds);

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
