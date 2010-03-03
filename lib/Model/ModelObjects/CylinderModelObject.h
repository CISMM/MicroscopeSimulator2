#ifndef _CYLINDER_MODEL_OBJECT_H_
#define _CYLINDER_MODEL_OBJECT_H_

#include <ModelObject.h>

#include <vtkSmartPointer.h>

class vtkCylinderSource;
class vtkTriangleFilter;


class CylinderModelObject : public ModelObject {

 public:
  static const char* OBJECT_TYPE_NAME;

  static const char* RADIUS_ATT;
  static const char* RADIUS_PROP;

  static const char* LENGTH_ATT;
  static const char* LENGTH_PROP;

  static const char* SURFACE_FLUOR_ATT;
  static const char* SURFACE_FLUOR_PROP;

  static const char* VOLUME_FLUOR_ATT;
  static const char* VOLUME_FLUOR_PROP;

  CylinderModelObject(DirtyListener* dirtyListener);
  virtual ~CylinderModelObject();

  virtual void GetXMLConfiguration(xmlNodePtr node);
  virtual void RestoreFromXML(xmlNodePtr node);

  virtual void Update();

 protected:
  CylinderModelObject() {};

  vtkSmartPointer<vtkCylinderSource> m_CylinderSource;
  vtkSmartPointer<vtkTriangleFilter> m_GeometrySource;

};

typedef CylinderModelObject* CylinderModelObjectPtr;

#endif // _CYLINDER_MODEL_OBJECT_H_
