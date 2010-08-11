#include <ModelObject.h>
#include <FluorophoreModelObjectProperty.h>
#include <ModelObjectProperty.h>
#include <ModelObjectPropertyList.h>
#include <XMLHelper.h>

#include <vtkAppendPolyData.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkPolyDataCollection.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>


const char* ModelObject::OBJECT_TYPE_NAME = "AbstractModelObject";
const char* ModelObject::NAME_PROP        = "Name";

const char* ModelObject::POSITION_ELEM    = "Position";
const char* ModelObject::X_POSITION_PROP  = "Position X";
const char* ModelObject::Y_POSITION_PROP  = "Position Y";
const char* ModelObject::Z_POSITION_PROP  = "Position Z";

const char* ModelObject::ROTATION_ELEM          = "Rotation";
const char* ModelObject::ROTATION_ANGLE_PROP    = "Rotation Angle";
const char* ModelObject::ROTATION_VECTOR_X_PROP = "Rotation Vector X";
const char* ModelObject::ROTATION_VECTOR_Y_PROP = "Rotation Vector Y";
const char* ModelObject::ROTATION_VECTOR_Z_PROP = "Rotation Vector Z";

const char* ModelObject::VISIBLE_PROP          = "Visible";

const char* ModelObject::SCANNABLE_PROP        = "Scannable";

const char* ModelObject::FLUOROPHORE_MODEL_LIST_ELEM = "FluorophoreModelList";


ModelObject
::ModelObject(DirtyListener* dirtyListener) {
  m_DirtyListener = dirtyListener;
  SetPickable(true);
  m_Properties = CreateDefaultProperties();
  m_FluorophoreProperties = new ModelObjectPropertyList();

  Initialize();
}


ModelObject
::ModelObject(DirtyListener* dirtyListener, ModelObjectPropertyList* properties) {
  m_DirtyListener = dirtyListener;
  SetPickable(true);
  m_Properties = properties;

  // Go through properties list and pluck out FluorophoreModelObjectProperties
  m_FluorophoreProperties = new ModelObjectPropertyList();
  for (int i = 0; i < m_Properties->GetSize(); i++) {
    ModelObjectProperty* prop = m_Properties->GetProperty(i);
    if (prop->GetType() == ModelObjectProperty::FLUOROPHORE_MODEL_TYPE) {
      m_FluorophoreProperties->AddProperty(prop);
    }
  }

  Initialize();
}


ModelObjectPropertyList*
ModelObject
::CreateDefaultProperties() {
  ModelObjectPropertyList* props = new ModelObjectPropertyList();
  props->AddProperty(new ModelObjectProperty(NAME_PROP, ModelObjectProperty::STRING_TYPE,
                                             "-", true, false));
  props->AddProperty(new ModelObjectProperty(VISIBLE_PROP, true, "-", true, false));
  //props->AddProperty(new ModelObjectProperty(SCANNABLE_PROP, true, "-", true, false));
  props->AddProperty(new ModelObjectProperty(X_POSITION_PROP, 0.0, "nanometers"));
  props->AddProperty(new ModelObjectProperty(Y_POSITION_PROP, 0.0, "nanometers"));
  props->AddProperty(new ModelObjectProperty(Z_POSITION_PROP, 0.0, "nanometers"));
  props->AddProperty(new ModelObjectProperty(ROTATION_ANGLE_PROP, 0.0, "degrees"));
  props->AddProperty(new ModelObjectProperty(ROTATION_VECTOR_X_PROP, 1.0));
  props->AddProperty(new ModelObjectProperty(ROTATION_VECTOR_Y_PROP, 0.0));
  props->AddProperty(new ModelObjectProperty(ROTATION_VECTOR_Z_PROP, 0.0));

  return props;
}


void
ModelObject
::Initialize() {
  m_ObjectTypeName = OBJECT_TYPE_NAME;
  m_Color[0] = 0.8;   m_Color[1] = 0.8;   m_Color[2] = 0.8;
}


ModelObject
::~ModelObject() {
  for (int i = 0; i < m_Properties->GetSize(); i++) {
    delete m_Properties->GetProperty(i);
  }

  delete m_Properties;
}


void
ModelObject
::Sully() {
  if (m_DirtyListener) {
    m_DirtyListener->Sully();
  }
}


void
ModelObject
::SetStatusMessage(const std::string& status) {
  if (m_DirtyListener) {
    m_DirtyListener->SetStatusMessage(status);
  }
}


void
ModelObject
::GetXMLConfiguration(xmlNodePtr node) {
  xmlNodePtr modelObjectNode = xmlNewChild(node, NULL, BAD_CAST GetObjectTypeName().c_str(), NULL);
  for (int i = 0; i < GetNumberOfProperties(); i++) {
    ModelObjectProperty* mop = GetProperty(i);
    std::string elementName = mop->GetXMLElementName();
    xmlNodePtr propertyNode = 
      xmlNewChild(modelObjectNode, NULL, BAD_CAST elementName.c_str(), NULL);
    mop->GetXMLConfiguration(propertyNode);
  }
}


void
ModelObject
::RestoreFromXML(xmlNodePtr node) {
  for (int i = 0; i < GetNumberOfProperties(); i++) {
    ModelObjectProperty* mop = GetProperty(i);
    std::string elementName = mop->GetXMLElementName();
    xmlNodePtr propertyNode =
      xmlGetFirstElementChildWithName(node, BAD_CAST elementName.c_str());
    mop->RestoreFromXML(propertyNode);
  }
}


std::string&
ModelObject
::GetObjectTypeName() {
  return m_ObjectTypeName;
}


void
ModelObject
::SetName(const std::string& name) {
  GetProperty(NAME_PROP)->SetStringValue(name);
}


std::string&
ModelObject
::GetName() {
  return GetProperty(NAME_PROP)->GetStringValue();
}


void
ModelObject
::SetVisible(bool visible) {
  GetProperty(VISIBLE_PROP)->SetBoolValue(visible);

  Sully();
}


bool
ModelObject
::GetVisible() {
  return GetProperty(VISIBLE_PROP)->GetBoolValue();
}


void
ModelObject
::SetScannable(bool scannable) {
  GetProperty(SCANNABLE_PROP)->SetBoolValue(scannable);

  Sully();
}


bool
ModelObject
::GetScannable() {
  return GetProperty(SCANNABLE_PROP)->GetBoolValue();
}


void
ModelObject
::SetPickable(bool dragable) {
  m_Pickable = dragable;
}


bool
ModelObject
::GetPickable() {
  return m_Pickable;
}


void
ModelObject
::AddProperty(ModelObjectProperty* property) {
  m_Properties->AddProperty(property);
  if (property->GetType() == ModelObjectProperty::FLUOROPHORE_MODEL_TYPE) {
    m_FluorophoreProperties->AddProperty(property);
  }
}


void
ModelObject
::PopProperty() {
  m_Properties->PopProperty();
}


void
ModelObject
::DeleteAndPopProperty() {
  m_Properties->DeleteAndPopProperty();
}


ModelObjectProperty*
ModelObject
::GetProperty(const std::string name) {
  return m_Properties->GetProperty(name);
}


ModelObjectProperty*
ModelObject
::GetProperty(int index) {
  return m_Properties->GetProperty(index);
}


FluorophoreModelObjectProperty*
ModelObject
::GetFluorophoreProperty(int index) {
  return dynamic_cast<FluorophoreModelObjectProperty*>
    (m_FluorophoreProperties->GetProperty(index));
}


ModelObjectPropertyList*
ModelObject
::GetPropertyList() {
  return m_Properties;
}


int
ModelObject
::GetNumberOfProperties() {
  if (m_Properties) {
    return m_Properties->GetSize();
  }
  return 0;
}


ModelObjectPropertyList*
ModelObject
::GetFluorophorePropertyList() {
  return m_FluorophoreProperties;
}


int
ModelObject
::GetNumberOfFluorophoreProperties() {
  if (m_FluorophoreProperties) {
    return m_FluorophoreProperties->GetSize();
  }
  return 0;
}


void
ModelObject
::SetPosition(double position[3]) {
  
  // Check that this ModelObject has position properties.
  if (GetProperty(X_POSITION_PROP)) {
    double origPosition[3];
    origPosition[0] = GetProperty(X_POSITION_PROP)->GetDoubleValue();
    origPosition[1] = GetProperty(Y_POSITION_PROP)->GetDoubleValue();
    origPosition[2] = GetProperty(Z_POSITION_PROP)->GetDoubleValue();

    bool samePosition = true;
    for (int i = 0; i < 3; i++) {
      samePosition = samePosition && (origPosition[i] == position[i]);
    }

    if (!samePosition) {
      GetProperty(X_POSITION_PROP)->SetDoubleValue(position[0]);
      GetProperty(Y_POSITION_PROP)->SetDoubleValue(position[1]);
      GetProperty(Z_POSITION_PROP)->SetDoubleValue(position[2]);
      Sully();
    }
  }
}


void
ModelObject
::GetPosition(double position[3]) {
  position[0] = m_Properties->GetProperty(X_POSITION_PROP)->GetDoubleValue();
  position[1] = m_Properties->GetProperty(Y_POSITION_PROP)->GetDoubleValue();
  position[2] = m_Properties->GetProperty(Z_POSITION_PROP)->GetDoubleValue();
}


void
ModelObject
::SetRotation(double rotation[4]) {

  // Check that this ModelObject has rotation properties.
  if (GetProperty(ROTATION_ANGLE_PROP)) {
    double origRotation[4];
    origRotation[0] = GetProperty(ROTATION_ANGLE_PROP)->GetDoubleValue();
    origRotation[1] = GetProperty(ROTATION_VECTOR_X_PROP)->GetDoubleValue();
    origRotation[2] = GetProperty(ROTATION_VECTOR_Y_PROP)->GetDoubleValue();
    origRotation[3] = GetProperty(ROTATION_VECTOR_Z_PROP)->GetDoubleValue();

    bool sameRotation = true;
    for (int i = 0; i < 4; i++) {
      sameRotation = sameRotation && (origRotation[i] == rotation[i]);
    }

    if (!sameRotation) {
      GetProperty(ROTATION_ANGLE_PROP)->SetDoubleValue(rotation[0]);
      GetProperty(ROTATION_VECTOR_X_PROP)->SetDoubleValue(rotation[1]);
      GetProperty(ROTATION_VECTOR_Y_PROP)->SetDoubleValue(rotation[2]);
      GetProperty(ROTATION_VECTOR_Z_PROP)->SetDoubleValue(rotation[3]);
      Sully();
    }
  } // else NOOP
}


void
ModelObject
::GetRotation(double rotation[4]) {
  if (GetProperty(ROTATION_ANGLE_PROP)) {
    rotation[0] = GetProperty(ROTATION_ANGLE_PROP)->GetDoubleValue();
    rotation[1] = GetProperty(ROTATION_VECTOR_X_PROP)->GetDoubleValue();
    rotation[2] = GetProperty(ROTATION_VECTOR_Y_PROP)->GetDoubleValue();
    rotation[3] = GetProperty(ROTATION_VECTOR_Z_PROP)->GetDoubleValue();
  } else {
    rotation[0] = 0.0;
    rotation[1] = 1.0;
    rotation[2] = 0.0;
    rotation[3] = 0.0;
  }
}


void
ModelObject
::SetColor(double r, double g, double b) {
  m_Color[0] = r;
  m_Color[1] = g;
  m_Color[2] = b;

  Sully();
}


void
ModelObject
::GetColor(double color[3]) {
  color[0] = m_Color[0];
  color[1] = m_Color[1];
  color[2] = m_Color[2];
}


double*
ModelObject
::GetColor() {
  return m_Color;
}


vtkPolyDataAlgorithm*
ModelObject
::GetAllGeometry() {
  vtkAppendPolyData* appender = vtkAppendPolyData::New();

  SubAssemblyMapType::iterator iter;
  for (iter = m_SubAssemblies.begin(); iter != m_SubAssemblies.end(); iter++) {
    appender->AddInputConnection((*iter).second->GetOutputPort());
  }

  return appender;
}


vtkPolyDataAlgorithm*
ModelObject
::GetAllGeometryTransformed() {
  vtkTransform* transform = vtkTransform::New();
  double rotation[4];
  GetRotation(rotation);
  double position[3];
  GetPosition(position);
  transform->Translate(position);
  transform->RotateWXYZ(rotation[0], rotation[1], rotation[2], rotation[3]);
  
  vtkTransformPolyDataFilter* transformFilter = vtkTransformPolyDataFilter::New();
  transformFilter->SetTransform(transform);
  transform->Delete();

  vtkPolyDataAlgorithm* geometry = GetAllGeometry();
  transformFilter->SetInputConnection(geometry->GetOutputPort());
  geometry->Delete();
            
  return transformFilter;
}


vtkPolyDataAlgorithm*
ModelObject
::GetGeometrySubAssembly(const std::string& name) {
  try {
    return m_SubAssemblies[name];
  } catch (...) {
  }
  return NULL;
}


void
ModelObject
::SetGeometrySubAssembly(const std::string& name, vtkPolyDataAlgorithm* assembly) {
  m_SubAssemblies[name] = assembly;
}


void
ModelObject
::ApplySampleForces(int fluorophorePropertyIndex, float* forces) {
  // Default is to optimize position and rotation.

  // Quick hack to do translation
  int numPoints = GetFluorophoreProperty(fluorophorePropertyIndex)->GetNumberOfFluorophores();
  double translation[3];
  translation[0] = translation[1] = translation[2] = 0.0;
  for (int i = 0; i < numPoints; i++) {
    for (int dim = 0; dim < 3; dim++) {
      translation[dim] += static_cast<double>(forces[i*3+dim]);
    }
  }

  std::cout << "Translation: " << translation[0] << ", " << translation[1]
            << ", " << translation[2] << std::endl;

  ModelObjectProperty* positionProperties[3];
  positionProperties[0] = GetProperty(X_POSITION_PROP);
  positionProperties[1] = GetProperty(Y_POSITION_PROP);
  positionProperties[2] = GetProperty(Z_POSITION_PROP);
  
  for (int i = 0; i < 3; i++) {
    if (positionProperties[i] && positionProperties[i]->GetOptimize()) {
      double value = positionProperties[i]->GetDoubleValue();
      positionProperties[i]->SetDoubleValue(value + translation[i]);
    }
  }

  Update();
}


void
ModelObject
::ApplyPointGradients(vtkPolyDataCollection* pointGradients, double stepSize) {
  vtkCollectionSimpleIterator iter;
  pointGradients->InitTraversal(iter);

  int numFluorophoreProperties = GetNumberOfFluorophoreProperties();
  for (int fluorIndex = 0; fluorIndex < numFluorophoreProperties; fluorIndex++) {
    vtkPolyData* gradientData = pointGradients->GetNextPolyData(iter);
    if (!GetFluorophoreProperty(fluorIndex)->GetEnabled()) {
      continue;
    }

    int numPoints = gradientData->GetNumberOfPoints();
    std::cout << "Num points: " << numPoints << std::endl;

    // Scale the gradient
    float* gradientPtr = reinterpret_cast<float*>
      (gradientData->GetPointData()->GetArray("Gradient")->GetVoidPointer(0));

    // Quick hack to do translation
    double translation[3];
    translation[0] = translation[1] = translation[2] = 0.0;
    for (int i = 0; i < numPoints; i++) {
      for (int dim = 0; dim < 3; dim++) {
        translation[dim] += stepSize * static_cast<double>(gradientPtr[i*3+dim]);
      }
    }
    for (int dim = 0; dim < 3; dim++) {
      translation[dim] /= static_cast<double>(numPoints);
    }

    std::cout << "Translation: " << translation[0] << ", " << translation[1]
              << ", " << translation[2] << std::endl;

    ModelObjectProperty* positionProperties[3];
    positionProperties[0] = GetProperty(X_POSITION_PROP);
    positionProperties[1] = GetProperty(Y_POSITION_PROP);
    positionProperties[2] = GetProperty(Z_POSITION_PROP);
  
    for (int dim = 0; dim < 3; dim++) {
      if (positionProperties[dim] && positionProperties[dim]->GetOptimize()) {
        double value = positionProperties[dim]->GetDoubleValue();
        positionProperties[dim]->SetDoubleValue(value + translation[dim]);
      }
    }

    Update();

  }
}
