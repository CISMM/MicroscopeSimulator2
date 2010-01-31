#ifndef _POINT_SET_MODEL_OBJECT_H_
#define _POINT_SET_MODEL_OBJECT_H_

#include <ModelObject.h>

#include <vtkSmartPointer.h>

class DirtyListener;
class vtkGlyph3D;
class vtkPoints;
class vtkPolyData;
class vtkSphereSource;
class vtkVertexGlyphFilter;


class PointSetModelObject : public ModelObject {
  
 public:
  static const char* OBJECT_TYPE_NAME;

  static const char* VISIBLE_RADIUS_ATT;
  static const char* VISIBLE_RADIUS_PROP;

  static const char* NUMBER_OF_POINTS_ATT;
  static const char* NUMBER_OF_POINTS_PROP;

  static const char* VERTICES_FLUOROPHORE_ATT;
  static const char* VERTICES_FLUOROPHORE_PROP;


  PointSetModelObject(DirtyListener* dirtyListener);
  PointSetModelObject(DirtyListener* dirtyListener,
                      ModelObjectPropertyList* properties);
  virtual ~PointSetModelObject();

  virtual void GetXMLConfiguration(xmlNodePtr node);

  virtual void Update();

  virtual void Sully();

 protected:
  PointSetModelObject() {};

  int m_PointPropertyStartingIndex;

  vtkSmartPointer<vtkPoints>            m_Points;
  vtkSmartPointer<vtkPolyData>          m_PointSource;
  vtkSmartPointer<vtkSphereSource>      m_SphereSource;
  vtkSmartPointer<vtkGlyph3D>           m_GlyphSource;
  vtkSmartPointer<vtkVertexGlyphFilter> m_VertexSource;

  virtual void UpdatePointProperties();

};

typedef PointSetModelObject* PointSetModelObjectPtr;

#endif // _POINT_SET_MODEL_OBJECT_H_
