#ifndef _MODEL_OBJECT_H_
#define _MODEL_OBJECT_H_

#include <map>
#include <string>
#include <vector>

#include <DirtyListener.h>
#include <ModelObjectProperty.h>
#include <XMLStorable.h>

// Forward declarations
class GeometrySource;
class FluorophoreModelObjectProperty;
class ModelObjectProperty;
class ModelObjectPropertyList;
typedef ModelObjectProperty* ModelObjectPropertyPtr;

class vtkActor;
class vtkPoints;
class vtkPolyDataAlgorithm;


class ModelObject : public DirtyListener, public XMLStorable {

 public:

  static const char* OBJECT_TYPE_NAME;
  static const char* NAME_PROP;

  static const char* POSITION_ELEM;
  static const char* X_POSITION_PROP;
  static const char* Y_POSITION_PROP;
  static const char* Z_POSITION_PROP;
  
  static const char* ROTATION_ELEM;
  static const char* ROTATION_ANGLE_PROP;
  static const char* ROTATION_VECTOR_X_PROP;
  static const char* ROTATION_VECTOR_Y_PROP;
  static const char* ROTATION_VECTOR_Z_PROP;

  static const char* VISIBLE_PROP;

  static const char* SCANNABLE_PROP;

  static const char* FLUOROPHORE_MODEL_LIST_ELEM;

 public:
  ModelObject(DirtyListener* dirtyListener);
  ModelObject(DirtyListener* dirtyListener,
              ModelObjectPropertyList* properties);
  virtual ~ModelObject();

  virtual void Sully();
  virtual void SetStatusMessage(const std::string& status);

  virtual void GetXMLConfiguration(xmlNodePtr node);
  virtual void RestoreFromXML(xmlNodePtr node);

  virtual std::string& GetObjectTypeName();

  virtual void SetName(const std::string& name);
  virtual std::string& GetName();

  void SetVisible(bool visible);
  bool GetVisible();

  void SetScannable(bool scannable);
  bool GetScannable();

  void SetPickable(bool pickable);
  bool GetPickable();

  void AddProperty(ModelObjectProperty* property);
  void PopProperty();
  void DeleteAndPopProperty();
  ModelObjectProperty* GetProperty(const std::string name);
  ModelObjectProperty* GetProperty(int index);

  FluorophoreModelObjectProperty* GetFluorophoreProperty(int index);

  ModelObjectPropertyList* GetPropertyList();
  int GetNumberOfProperties();
  
  ModelObjectPropertyList* GetFluorophorePropertyList();
  int GetNumberOfFluorophoreProperties();

  virtual void SetPosition(double position[3]);
  virtual void GetPosition(double position[3]);

  virtual void SetRotation(double rotation[4]);
  virtual void GetRotation(double rotation[4]);

  void SetColor(double r, double g, double b);
  void GetColor(double color[3]);
  double* GetColor();

  vtkPolyDataAlgorithm* GetAllGeometry();
  vtkPolyDataAlgorithm* GetAllGeometryTransformed();
  vtkPolyDataAlgorithm* GetGeometrySubAssembly(const std::string& name);

  /** Gets a sampling of surface points on the geometry. */
  virtual vtkPoints* GetSurfaceSamplePoints();
  virtual int        GetNumberOfSurfaceSamplePoints();

  /** Gets a sampling of points within the volume bounded by the geometry. */
  virtual vtkPoints* GetVolumeSamplePoints();
  virtual int        GetNumberOfVolumeSamplePoints();

  /** Applies forces at sample points to geometry and/or sample points
      themselves. The size of the forces array should be three times the
      number of points returned by GetNumberOfSurfaceSamplePoints()/
      GetNumberOfVolumeSamplePoints(). */
  virtual void ApplySurfaceSampleForces(float* forces);
  virtual void ApplyVolumeSampleForces(float* forces);

  virtual void Update() = 0;


 protected:
  ModelObject() {};

  DirtyListener*           m_DirtyListener;
  std::string              m_ObjectTypeName;
  bool                     m_Pickable;
  ModelObjectPropertyList* m_Properties;
  ModelObjectPropertyList* m_FluorophoreProperties;
  double                   m_Color[3];

  typedef std::map<std::string, vtkPolyDataAlgorithm*> SubAssemblyMapType;
  SubAssemblyMapType m_SubAssemblies;

  ModelObjectPropertyList* CreateDefaultProperties();

  void SetGeometrySubAssembly(const std::string& name, vtkPolyDataAlgorithm* assembly);

 private:
  void Initialize();

};

typedef ModelObject* ModelObjectPtr;

#endif // _MODEL_OBJECT_H_
