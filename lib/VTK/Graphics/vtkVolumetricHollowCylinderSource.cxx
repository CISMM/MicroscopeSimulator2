#include "vtkVolumetricHollowCylinderSource.h"

#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkUnstructuredGrid.h"

#include <math.h>

vtkCxxRevisionMacro(vtkVolumetricHollowCylinderSource, "$Revision: 1.55 $");
vtkStandardNewMacro(vtkVolumetricHollowCylinderSource);

vtkVolumetricHollowCylinderSource::vtkVolumetricHollowCylinderSource (int res)
{
  this->Resolution = res;
  this->Height = 1.0;
  this->InnerRadius = 0.1;
  this->OuterRadius = 0.5;
  this->GenerateScalars = 1;
  this->Center[0] = this->Center[1] = this->Center[2] = 0.0;

  this->SetNumberOfInputPorts(0);
}

int vtkVolumetricHollowCylinderSource::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  double angle= 2.0*3.141592654/this->Resolution;
  int numCells, numPts;
  double xtopin[3], xbotin[3], xtopout[3], xbotout[3];
  int i, idx;
  vtkIdType pts[VTK_CELL_SIZE];
  vtkPoints *newPoints; 
  vtkCellArray *newCells;
  
  //
  // Set things up; allocate memory
  //

  numPts   = 4*this->Resolution;
  numCells = 5*this->Resolution;

  newPoints = vtkPoints::New();
  newPoints->Allocate(numPts);

  newCells = vtkCellArray::New();
  newCells->Allocate(newCells->EstimateSize(numCells,this->Resolution));
  //
  // Generate points and point data for sides
  //
  for (i=0; i<this->Resolution; i++)
    {
    // x coordinate
    xtopout[0] = xbotout[0] = this->OuterRadius * cos(i*angle) + this->Center[0];
    xtopin [0] = xbotin [0] = this->InnerRadius * cos(i*angle) + this->Center[0];

    // y coordinate
    xtopout[1] = xtopin[1] =  0.5 * this->Height + this->Center[1];
    xbotout[1] = xbotin[1] = -0.5 * this->Height + this->Center[1];

    // z coordinate
    xtopout[2] = xbotout[2] = this->OuterRadius * sin(i*angle) + this->Center[2];
    xtopin [2] = xbotin [2] = this->InnerRadius * sin(i*angle) + this->Center[2];

    idx = 4*i;
    newPoints->InsertPoint(idx,   xtopout);
    newPoints->InsertPoint(idx+1, xbotout);
    newPoints->InsertPoint(idx+2, xbotin );
    newPoints->InsertPoint(idx+3, xtopin );
    }

  vtkIdType tetPtIds[VTK_CELL_SIZE];

  //
  // Generate tetrahedral cells for volume.
  //
  for (i=0; i<this->Resolution; i++)
    {
    int mod = 4*this->Resolution;
    int n0 = 4*i+0, n1 = 4*i+1, n2 = 4*i+2, n3 = 4*i+3;
    int n4 = (4*(i+1)+0) % mod, n5 = (4*(i+1)+1) % mod;
    int n6 = (4*(i+1)+2) % mod, n7 = (4*(i+1)+3) % mod;

    // Tet 0
    tetPtIds[0] = n0;
    tetPtIds[1] = n3;
    tetPtIds[2] = n5;
    tetPtIds[3] = n1;
    newCells->InsertNextCell(4, tetPtIds);

    // Tet 1
    tetPtIds[0] = n3;
    tetPtIds[1] = n0;
    tetPtIds[2] = n5;
    tetPtIds[3] = n4;
    newCells->InsertNextCell(4, tetPtIds);

    // Tet 2
    tetPtIds[0] = n4;
    tetPtIds[1] = n7;
    tetPtIds[2] = n5;
    tetPtIds[3] = n3;
    newCells->InsertNextCell(4, tetPtIds);

    // Tet 3
    tetPtIds[0] = n5;
    tetPtIds[1] = n3;
    tetPtIds[2] = n7;
    tetPtIds[3] = n6;
    newCells->InsertNextCell(4, tetPtIds);

    // Tet 4
    tetPtIds[0] = n2;
    tetPtIds[1] = n5;
    tetPtIds[2] = n6;
    tetPtIds[3] = n3;
    newCells->InsertNextCell(4, tetPtIds);

    // Tet 5
    tetPtIds[0] = n1;
    tetPtIds[1] = n2;
    tetPtIds[2] = n3;
    tetPtIds[3] = n5;
    newCells->InsertNextCell(4, tetPtIds);
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

void vtkVolumetricHollowCylinderSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "Resolution: " << this->Resolution << "\n";
  os << indent << "Height: " << this->Height << "\n";
  os << indent << "Inner Radius: " << this->InnerRadius << "\n";
  os << indent << "Outer Radius: " << this->OuterRadius << "\n";
  os << indent << "Center: (" << this->Center[0] << ", "
     << this->Center[1] << ", " << this->Center[2] << " )\n";
  os << indent << "GenerateScalars: " << (this->GenerateScalars ? "On\n" : "Off\n");
}
