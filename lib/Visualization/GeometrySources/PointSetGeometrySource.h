#ifndef _POINT_SET_GEOMETRY_SOURCE_H_
#define _POINT_SET_GEOMETRY_SOURCE_H_

#include <vtkSmartPointer.h>

#include <GeometrySource.h>

// Forward declarations
class vtkGlyph3D;
class vtkPoints;
class vtkPolyData;
class vtkSphereSource;


class PointSetGeometrySource : public GeometrySource {

 public:
  PointSetGeometrySource(ModelObjectPtr modelObject);
  virtual ~PointSetGeometrySource();

  virtual vtkAlgorithmOutput* GetPolyDataOutputPort();

  virtual void Update();

 protected:
  PointSetGeometrySource() {};

  vtkSmartPointer<vtkPoints>       m_Points;
  vtkSmartPointer<vtkPolyData>     m_PointSource;
  vtkSmartPointer<vtkSphereSource> m_SphereSource;
  vtkSmartPointer<vtkGlyph3D>      m_GlyphSource;
  

};

#endif // _POINT_SET_GEOMETRY_SOURCE_H_
