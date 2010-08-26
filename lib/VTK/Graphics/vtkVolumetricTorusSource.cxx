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

vtkCxxRevisionMacro(vtkVolumetricTorusSource, "$Revision: 1.55 $");
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

  double thetaAngle = vtkMath::DoubleTwoPi()/this->ThetaResolution;
  double phiAngle   = vtkMath::DoubleTwoPi()/this->PhiResolution;
  int numCells, numPts;
  double x[3];
  int i, j, idx;
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

      double r = this->RingRadius + this->CrossSectionRadius*cos(phi);
      x[0] = r*cos(theta);
      x[1] = r*sin(theta);
      x[2] = this->CrossSectionRadius*sin(phi);

      newPoints->InsertPoint(j*skip+i, x);
      }

    // Point on the medial axis
    x[0] = this->RingRadius*cos(theta);
    x[1] = this->RingRadius*sin(theta);
    x[2] = 0.0;
    newPoints->InsertPoint(j*skip + this->PhiResolution,x);
    }

  vtkIdType tetPtIds[VTK_CELL_SIZE];

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

void vtkVolumetricTorusSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "CrossSectionRadius: " << this->CrossSectionRadius << "\n";
  os << indent << "RingRadius: " << this->RingRadius << "\n";
  os << indent << "ThetaResolution: " << this->ThetaResolution << "\n";
  os << indent << "PhiResolution: " << this->PhiResolution << "\n";
  os << indent << "GenerateScalars: " << (this->GenerateScalars ? "On\n" : "Off\n");
}
