#ifndef _TORUS_MODEL_OBJECT_H_
#define _TORUS_MODEL_OBJECT_H_

#include <ModelObject.h>
#include <vtkSmartPointer.h>

class vtkParametricTorus;
class vtkParametricFunctionSource;
class vtkTriangleFilter;


class TorusModelObject : public ModelObject {

 public:
  static const char* OBJECT_TYPE_NAME;

  static const char* CROSS_SECTION_RADIUS_ATT;
  static const char* CROSS_SECTION_RADIUS_PROP;

  static const char* RING_RADIUS_ATT;
  static const char* RING_RADIUS_PROP;

  static const char* SURFACE_FLUOR_ATT;
  static const char* SURFACE_FLUOR_PROP;

  static const char* VOLUME_FLUOR_ATT;
  static const char* VOLUME_FLUOR_PROP;


  TorusModelObject(DirtyListener* dirtyListener);
  virtual ~TorusModelObject();

  virtual void Update();

  virtual void GetXMLConfiguration(xmlNodePtr node);

 protected:
  TorusModelObject() {};

  vtkSmartPointer<vtkParametricTorus>          m_Torus;
  vtkSmartPointer<vtkParametricFunctionSource> m_TorusSource;
  vtkSmartPointer<vtkTriangleFilter>           m_GeometrySource;

};

#endif // _TORUS_MODEL_OBJECT_H_
