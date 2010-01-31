#ifndef _GEOMETRY_SOURCE_H_
#define _GEOMETRY_SOURCE_H_

// Forward declarations
class vtkAlgorithmOutput;

#include "ModelObject.h"

class GeometrySource {

 public:
  GeometrySource(ModelObjectPtr modelObject);
  virtual ~GeometrySource();

  ModelObjectPtr GetModelObject();

  virtual vtkAlgorithmOutput* GetPolyDataOutputPort() = 0;

  virtual void Update() = 0;

 protected:
  GeometrySource() {};

  ModelObjectPtr m_ModelObject;

};


#endif // _GEOMETRY_SOURCE_H_
