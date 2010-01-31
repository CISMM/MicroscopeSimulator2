#ifndef _SPHERE_GEOMETRY_SOURCE_H_
#define _SPHERE_GEOMETRY_SOURCE_H_

#include <vtkSmartPointer.h>

#include <GeometrySource.h>

// Forward declarations
class vtkSphereSource;


class SphereGeometrySource : public GeometrySource {

 public:
  SphereGeometrySource(ModelObjectPtr modelObject);
  virtual ~SphereGeometrySource();

  virtual vtkAlgorithmOutput* GetPolyDataOutputPort();

  virtual void Update();

 protected:
  SphereGeometrySource() {};

  vtkSmartPointer<vtkSphereSource> m_SphereSource;
  
};

typedef SphereGeometrySource* SphereGeometrySourcePtr;


#endif // _SPHERE_GEOMETRY_SOURCE_H_
