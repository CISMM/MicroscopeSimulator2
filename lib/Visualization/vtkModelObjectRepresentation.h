#ifndef _VTK_MODEL_OBJECT_REPRESENTATION_H_
#define _VTK_MODEL_OBJECT_REPRESENTATION_H_

#include <vtkDataRepresentation.h>

class ModelObject;
typedef ModelObject* ModelObjectPtr;


class vtkModelObjectRepresentation : public vtkDataRepresentation {
 public:
  vtkTypeRevisionMacro(vtkModelObjectRepresentation, vtkDataRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Sets the ModelObject.
  void SetModelObject(ModelObjectPtr mo);
  ModelObjectPtr GetModelObject();

 protected:
  vtkModelObjectRepresentation();
  virtual ~vtkModelObjectRepresentation();

  ModelObjectPtr ModelObject;

  virtual void UpdateRepresentation() = 0;

 private:
  vtkModelObjectRepresentation(const vtkModelObjectRepresentation&); // Purposely not implemented
  void operator=(const vtkModelObjectRepresentation&); // Purposely not implemented

};

#endif // _VTK_MODEL_OBJECT_REPRESENTATION_H_
