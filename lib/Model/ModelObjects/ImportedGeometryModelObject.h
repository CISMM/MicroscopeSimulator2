#ifndef _IMPORTED_GEOMETRY_MODEL_OBJECT_H_
#define _IMPORTED_GEOMETRY_MODEL_OBJECT_H_

#include <ModelObject.h>

#include <vtkSmartPointer.h>

class vtkCleanPolyData;
class vtkTransform;
class vtkTransformPolyDataFilter;
class vtkTriangleFilter;


class ImportedGeometryModelObject : public ModelObject {

public:
  static const char* OBJECT_TYPE_NAME;

  static const char* FILE_NAME_PROP;
  static const char* SCALE_PROP;
  static const char* SURFACE_FLUOR_PROP;
  static const char* VOLUME_FLUOR_PROP;
  static const char* GRID_FLUOR_PROP;

  ImportedGeometryModelObject(DirtyListener* dirtyListener);
  virtual ~ImportedGeometryModelObject();

  void LoadFile(const std::string& fileName);

  vtkPolyData* GetPolyData();

  virtual void Update();

  virtual void RestoreFromXML(xmlNodePtr node);

protected:
  ImportedGeometryModelObject() {};

  vtkSmartPointer<vtkTransform>               m_Transform;
  vtkSmartPointer<vtkTransformPolyDataFilter> m_TransformFilter;

  vtkSmartPointer<vtkTriangleFilter>  m_TriangleFilter;
  vtkSmartPointer<vtkCleanPolyData>   m_CleanPolyData;
};

typedef ImportedGeometryModelObject* ImportedGeometryModelObjectPtr;

#endif // _IMPORTED_GEOMETRY_MODEL_OBJECT_H_
