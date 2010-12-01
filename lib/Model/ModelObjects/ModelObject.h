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
class Matrix;
class ModelObjectProperty;
class ModelObjectPropertyList;
typedef ModelObjectProperty* ModelObjectPropertyPtr;

class vtkActor;
class vtkPoints;
class vtkPolyData;
class vtkPolyDataAlgorithm;
class vtkPolyDataCollection;


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
  void RegenerateFluorophores();

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

  /** Applies point gradient at fluorophore sample points to geometry and/or 
      the sample points themselves. The size of the gradient array should be 
      three times the number of fluorophores samples in the given
      fluorophore model. It is up to subclasses to decide how to use
      this gradient. */
  virtual void ApplyPointGradients(vtkPolyDataCollection* pointGradients, double stepSize);

  virtual void Update();


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

  /** Get Jacobian matrix column entries for rotation components.
    Valid values for the component argument are:
      ROTATION_ANGLE_PROP
      ROTATION_VECTOR_X_PROP
      ROTATION_VECTOR_Y_PROP
      ROTATION_VECTOR_Z_PROP
  */
  void GetRotationJacobianMatrixColumn(vtkPolyData* points,
    const char* component, int column, Matrix* matrix);

  void GetRotationAngleJacobianMatrixColumn
  (vtkPolyData* points, int column, Matrix* matrix,
   double *currentRotation, double* newRotation);

  /** Get Jacobian matrix column entries for translation components.
      Values of the parameter axis mean: 0 - x axis; 1 - y axis; 2 - z axis. */
  void GetTranslationJacobianMatrixColumn(vtkPolyData* points, int axis,
                                          int column, Matrix* matrix);

  void NormalizeRotationVector();

 private:
  void Initialize();

};

typedef ModelObject* ModelObjectPtr;

#endif // _MODEL_OBJECT_H_
