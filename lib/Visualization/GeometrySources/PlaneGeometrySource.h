#ifndef _PLANE_GEOMETRY_SOURCE_H_
#define _PLANE_GEOMETRY_SOURCE_H_

#include <vtkSmartPointer.h>

#include <GeometrySource.h>

// Forward declarations
class vtkPlaneSource;


class PlaneGeometrySource : public GeometrySource {

 public:
  PlaneGeometrySource(ModelObjectPtr modelObject);
  virtual ~PlaneGeometrySource();

  virtual vtkAlgorithmOutput* GetPolyDataOutputPort();

  virtual void Update();

 protected:
  PlaneGeometrySource() {};

  vtkSmartPointer<vtkPlaneSource> m_PlaneSource;

};

#endif //  _PLANE_GEOMETRY_SOURCE_H_
