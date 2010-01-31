#ifndef _FLEXIBLE_TUBE_GEOMETRY_SOURCE_H_
#define _FLEXIBLE_TUBE_GEOMETRY_SOURCE_H_

#include <vtkSmartPointer.h>

#include <GeometrySource.h>

// Forward declarations
class vtkParametricSpline;
class vtkParametricFunctionSource;
class vtkPoints;
class vtkTubeFilter;
class vtkTriangleFilter;


class FlexibleTubeGeometrySource : public GeometrySource {

 public:
  FlexibleTubeGeometrySource(ModelObjectPtr modelObject);
  virtual ~FlexibleTubeGeometrySource();

  virtual vtkAlgorithmOutput* GetPolyDataOutputPort();

  virtual void Update();

 protected:
  FlexibleTubeGeometrySource() {};

  vtkSmartPointer<vtkPoints>                   m_Points;
  vtkSmartPointer<vtkParametricSpline>         m_Spline;
  vtkSmartPointer<vtkParametricFunctionSource> m_SplineSource;
  vtkSmartPointer<vtkTubeFilter>               m_TubeFilter;
  vtkSmartPointer<vtkTriangleFilter>           m_TubeSource;
  

};

#endif // _FLEXIBLE_TUBE_GEOMETRY_SOURCE_H_
