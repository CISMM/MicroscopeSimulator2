#ifndef _ELLIPSOID_MODEL_OBJECT_H_
#define _ELLIPSOID_MODEL_OBJECT_H_

/** This class defines a sphere object that is paremeterized by radius.
 *
 * It has surface and volume fluorophore labeling models.
 */

#include <ModelObject.h>

#include <vtkSmartPointer.h>

class vtkPolyDataNormals;
class vtkVolumetricEllipsoidSource;


class EllipsoidModelObject : public ModelObject {

 public:
  static const char* OBJECT_TYPE_NAME;
  static const char* RADIUS_X_PROP;
  static const char* RADIUS_Y_PROP;
  static const char* RADIUS_Z_PROP;
  static const char* SURFACE_FLUOR_PROP;
  static const char* VOLUME_FLUOR_PROP;
  static const char* GRID_FLUOR_PROP;


  EllipsoidModelObject(DirtyListener* dirtyListener);
  virtual ~EllipsoidModelObject();
  
  virtual void Update();

 protected:
  EllipsoidModelObject() {};

  vtkSmartPointer<vtkVolumetricEllipsoidSource> m_EllipsoidSource;
  vtkSmartPointer<vtkPolyDataNormals>           m_GeometrySource;

};

typedef EllipsoidModelObject* EllipsoidModelObjectPtr;

#endif // _ELLIPSOID_MODEL_OBJECT_H_
