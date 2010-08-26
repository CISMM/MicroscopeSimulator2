#ifndef _SPHERE_MODEL_OBJECT_H_
#define _SPHERE_MODEL_OBJECT_H_

/** This class defines a sphere object that is paremeterized by radius.
 *
 * It has surface and volume fluorophore labeling models.
 */

#include <ModelObject.h>

#include <vtkSmartPointer.h>

class vtkPolyDataNormals;
class vtkVolumetricEllipsoidSource;


class SphereModelObject : public ModelObject {

 public:
  static const char* OBJECT_TYPE_NAME;

  static const char* RADIUS_ATT;
  static const char* RADIUS_PROP;

  static const char* SURFACE_FLUOR_ATT;
  static const char* SURFACE_FLUOR_PROP;

  static const char* VOLUME_FLUOR_ATT;
  static const char* VOLUME_FLUOR_PROP;


  SphereModelObject(DirtyListener* dirtyListener);
  virtual ~SphereModelObject();
  
  virtual void Update();

 protected:
  SphereModelObject() {};

  ModelObjectPropertyList* CreateProperties();

  vtkSmartPointer<vtkVolumetricEllipsoidSource> m_SphereSource;
  vtkSmartPointer<vtkPolyDataNormals>           m_GeometrySource;

};

typedef SphereModelObject* SphereModelObjectPtr;

#endif // _SPHERE_MODEL_OBJECT_H_
