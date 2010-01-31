#ifndef _TORUS_GEOMETRY_SOURCE_H_
#define _TORUS_GEOMETRY_SOURCE_H_

#include <vtkSmartPointer.h>

#include "GeometrySource.h"

// Forward declarations
class vtkParametricTorus;
class vtkParametricFunctionSource;

class TorusGeometrySource : public GeometrySource {

 public:
  TorusGeometrySource(ModelObjectPtr modelObject);
  virtual ~TorusGeometrySource();

  virtual vtkAlgorithmOutput* GetPolyDataOutputPort();

  virtual void Update();

 protected:
  TorusGeometrySource() {};

  vtkSmartPointer<vtkParametricTorus> m_Torus;

  vtkSmartPointer<vtkParametricFunctionSource> m_TorusSource;
  
};

typedef TorusGeometrySource* TorusGeometrySourcePtr;


#endif // _TORUS_GEOMETRY_SOURCE_H_
