#include "vtkModelObjectRepresentation.h"

vtkCxxRevisionMacro(vtkModelObjectRepresentation, "$Revision: 1.0 $");


vtkModelObjectRepresentation::vtkModelObjectRepresentation() {
  this->ModelObject = NULL;
}


vtkModelObjectRepresentation::~vtkModelObjectRepresentation() {

}


void vtkModelObjectRepresentation::SetModelObject(ModelObjectPtr mo) {
  this->ModelObject = mo;
}


ModelObjectPtr vtkModelObjectRepresentation::GetModelObject() {
  return this->ModelObject;
}


void vtkModelObjectRepresentation::PrintSelf(ostream& os, vtkIndent indent) {

}


