#ifndef _CYLINDER_GEOMETRY_SOURCE_H_
#define _CYLINDER_GEOMETRY_SOURCE_H_

#include <vtkSmartPointer.h>

#include <GeometrySource.h>

// Forward declarations
class vtkCylinderSource;

class CylinderGeometrySource : public GeometrySource {

 public:
  CylinderGeometrySource(ModelObjectPtr modelObject);
  virtual ~CylinderGeometrySource();

  virtual vtkAlgorithmOutput* GetPolyDataOutputPort();

  virtual void Update();

 protected:
  CylinderGeometrySource() {};

  vtkSmartPointer<vtkCylinderSource> m_CylinderSource;

};

typedef CylinderGeometrySource* CylinderGeometrySourcePtr;


#endif // _CYLINDER_GEOMETRY_SOURCE_H_
