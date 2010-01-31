#ifndef _FLEXIBLE_TUBE_MODEL_OBJECT_H_
#define _FLEXIBLE_TUBE_MODEL_OBJECT_H_

#include <vtkSmartPointer.h>

#include <PointSetModelObject.h>

class vtkParametricSpline;
class vtkParametricFunctionSource;
class vtkPoints;
class vtkTubeFilter;
class vtkTriangleFilter;


class FlexibleTubeModelObject : public PointSetModelObject {

 public:
  static const char* OBJECT_TYPE_NAME;

  static const char* RADIUS_ATT;
  static const char* RADIUS_PROP;

  static const char* SURFACE_FLUOR_ATT;
  static const char* SURFACE_FLUOR_PROP;

  static const char* VOLUME_FLUOR_ATT;
  static const char* VOLUME_FLUOR_PROP;
  
  //static const char* NUMBER_OF_POINTS_ATT;
  //static const char* NUMBER_OF_POINTS_PROP;

  FlexibleTubeModelObject(DirtyListener* dirtyListener);
  virtual ~FlexibleTubeModelObject();

  virtual void GetXMLConfiguration(xmlNodePtr node);

  virtual void Update();

 protected:
  FlexibleTubeModelObject() {};

  vtkSmartPointer<vtkPoints>                   m_Points;
  vtkSmartPointer<vtkParametricSpline>         m_Spline;
  vtkSmartPointer<vtkParametricFunctionSource> m_SplineSource;
  vtkSmartPointer<vtkTubeFilter>               m_TubeFilter;
  vtkSmartPointer<vtkTriangleFilter>           m_TubeSource;

  double GetLength();

 private:
  ModelObjectPropertyList* CreateProperties();

};

#endif // _FLEXIBLE_TUBE_MODEL_OBJECT_H_
