#ifndef _POINT_RING_MODEL_OBJECT_H_
#define _POINT_RING_MODEL_OBJECT_H_

#include <ModelObject.h>

#include <vtkSmartPointer.h>

class DirtyListener;
class vtkGlyph3D;
class vtkPoints;
class vtkPointRingSource;
class vtkPolyData;
class vtkSphereSource;
class vtkVertexGlyphFilter;


class PointRingModelObject : public ModelObject {

 public:
  static const char* OBJECT_TYPE_NAME;

  static const char* VISIBLE_RADIUS_PROP;
  static const char* RING_RADIUS_PROP;
  static const char* NUMBER_OF_POINTS_PROP;
  static const char* VERTICES_FLUOROPHORE_PROP;

  
  PointRingModelObject(DirtyListener* dirtyListener);
  PointRingModelObject(DirtyListener* dirtyListener,
                       ModelObjectPropertyList* properties);
  virtual ~PointRingModelObject();

  virtual void Update();

 protected:
  PointRingModelObject() {};

  vtkSmartPointer<vtkPointRingSource>   m_PointRingSource;
  vtkSmartPointer<vtkSphereSource>      m_SphereSource;
  vtkSmartPointer<vtkGlyph3D>           m_GlyphSource;
  vtkSmartPointer<vtkVertexGlyphFilter> m_VertexSource;

};

#endif //  _POINT_RING_MODEL_OBJECT_H_
