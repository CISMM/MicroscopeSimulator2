#include <ModelObject.h>
#include <ModelObjectProperty.h>
#include <ModelObjectPropertyList.h>
#include <FluorophoreModelTypes.h>

#include <vtkAppendPolyData.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>


const char* ModelObject::OBJECT_TYPE_NAME = "AbstractModelObject";
const char* ModelObject::NAME_ATT         = "name";
const char* ModelObject::NAME_PROP        = "Name";

const char* ModelObject::POSITION_ELEM    = "Position";
const char* ModelObject::X_POSITION_ATT   = "x";
const char* ModelObject::Y_POSITION_ATT   = "y";
const char* ModelObject::Z_POSITION_ATT   = "z";
const char* ModelObject::X_POSITION_PROP  = "Position X";
const char* ModelObject::Y_POSITION_PROP  = "Position Y";
const char* ModelObject::Z_POSITION_PROP  = "Position Z";

const char* ModelObject::ROTATION_ELEM          = "Rotation";
const char* ModelObject::ROTATION_ANGLE         = "angle";
const char* ModelObject::ROTATION_VECTOR_X_ATT  = "vx";
const char* ModelObject::ROTATION_VECTOR_Y_ATT  = "vy";
const char* ModelObject::ROTATION_VECTOR_Z_ATT  = "vz";
const char* ModelObject::ROTATION_ANGLE_PROP    = "Rotation Angle";
const char* ModelObject::ROTATION_VECTOR_X_PROP = "Rotation Vector X";
const char* ModelObject::ROTATION_VECTOR_Y_PROP = "Rotation Vector Y";
const char* ModelObject::ROTATION_VECTOR_Z_PROP = "Rotation Vector Z";

const char* ModelObject::COLOR_ELEM = "Color";
const char* ModelObject::RED_ATT    = "red";
const char* ModelObject::GREEN_ATT  = "green";
const char* ModelObject::BLUE_ATT   = "blue";

const char* ModelObject::VISIBLE_ATT           = "visible";
const char* ModelObject::VISIBLE_PROP          = "Visible";

const char* ModelObject::SCANNABLE_ATT         = "scannable";
const char* ModelObject::SCANNABLE_PROP        = "Scannable";

const char* ModelObject::FLUOROPHORE_MODEL_LIST_ELEM = "FluorophoreModelList";


ModelObject
::ModelObject(DirtyListener* dirtyListener) {
  m_DirtyListener = dirtyListener;
  m_Properties = CreateDefaultProperties();
  m_FluorophoreProperties = new ModelObjectPropertyList();

  Initialize();
}


ModelObject
::ModelObject(DirtyListener* dirtyListener, ModelObjectPropertyList* properties) {
  m_DirtyListener = dirtyListener;
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
  props->AddProperty(new ModelObjectProperty(NAME_PROP, ModelObjectProperty::STRING_TYPE));
  props->AddProperty(new ModelObjectProperty(VISIBLE_PROP, true));
  props->AddProperty(new ModelObjectProperty(SCANNABLE_PROP, true));
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
  m_Optimizable = true;
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
  
  // Subclasses are required to create the element for the model object
  // description prior to calling this method. The node parameter must be
  // root element for this model object. This method simply adds
  // children elements and their attributes that are common to all model
  // object types.

  // Set object attributes
  char trueString[]  = "true";
  char falseString[] = "false";
  xmlNewProp(node, BAD_CAST NAME_ATT,
             BAD_CAST GetProperty(NAME_PROP)->GetStringValue().c_str());
  xmlNewProp(node, BAD_CAST VISIBLE_ATT, 
             BAD_CAST (GetProperty(VISIBLE_PROP)->GetBoolValue() ? trueString : falseString));
  xmlNewProp(node, BAD_CAST SCANNABLE_ATT,
             BAD_CAST (GetProperty(SCANNABLE_PROP)->GetBoolValue() ? trueString : falseString));

  xmlNodePtr posElem = xmlNewChild(node, NULL, BAD_CAST POSITION_ELEM, NULL);
  char floatFormat[] = "%f";
  char buf[128];
  sprintf(buf, floatFormat, GetProperty(X_POSITION_PROP)->GetDoubleValue());
  xmlNewProp(posElem, BAD_CAST X_POSITION_ATT, BAD_CAST buf);
  sprintf(buf, floatFormat, GetProperty(Y_POSITION_PROP)->GetDoubleValue());
  xmlNewProp(posElem, BAD_CAST Y_POSITION_ATT, BAD_CAST buf);
  sprintf(buf, floatFormat, GetProperty(Z_POSITION_PROP)->GetDoubleValue());
  xmlNewProp(posElem, BAD_CAST Z_POSITION_ATT, BAD_CAST buf);

  // TODO - finish adding rotation components
  xmlNodePtr rotElem = xmlNewChild(node, NULL, BAD_CAST ROTATION_ELEM, NULL);
  sprintf(buf, floatFormat, GetProperty(ROTATION_ANGLE_PROP)->GetDoubleValue());
  xmlNewProp(rotElem, BAD_CAST ROTATION_ANGLE, BAD_CAST buf);
  sprintf(buf, floatFormat, GetProperty(ROTATION_VECTOR_X_PROP)->GetDoubleValue());
  xmlNewProp(rotElem, BAD_CAST ROTATION_VECTOR_X_ATT, BAD_CAST buf);
  sprintf(buf, floatFormat, GetProperty(ROTATION_VECTOR_Y_PROP)->GetDoubleValue());
  xmlNewProp(rotElem, BAD_CAST ROTATION_VECTOR_Y_ATT, BAD_CAST buf);
  sprintf(buf, floatFormat, GetProperty(ROTATION_VECTOR_Z_PROP)->GetDoubleValue());
  xmlNewProp(rotElem, BAD_CAST ROTATION_VECTOR_Z_ATT, BAD_CAST buf);

  xmlNodePtr colorElem = xmlNewChild(node, NULL, BAD_CAST COLOR_ELEM, NULL);
  sprintf(buf, floatFormat, m_Color[0]);
  xmlNewProp(colorElem, BAD_CAST RED_ATT, BAD_CAST buf);
  sprintf(buf, floatFormat, m_Color[1]);
  xmlNewProp(colorElem, BAD_CAST GREEN_ATT, BAD_CAST buf);
  sprintf(buf, floatFormat, m_Color[2]);
  xmlNewProp(colorElem, BAD_CAST BLUE_ATT, BAD_CAST buf);

  
  // TODO - add fluorophore model list

  
  
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
::SetOptimizable(bool optimizable) {
  m_Optimizable = optimizable;

  Sully();
}


bool
ModelObject
::GetOptimizable() {
  return m_Optimizable;
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
  GetProperty(X_POSITION_PROP)->SetDoubleValue(position[0]);
  GetProperty(Y_POSITION_PROP)->SetDoubleValue(position[1]);
  GetProperty(Z_POSITION_PROP)->SetDoubleValue(position[2]);

  Sully();
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
  GetProperty(ROTATION_ANGLE_PROP)->SetDoubleValue(rotation[0]);
  GetProperty(ROTATION_VECTOR_X_PROP)->SetDoubleValue(rotation[1]);
  GetProperty(ROTATION_VECTOR_Y_PROP)->SetDoubleValue(rotation[2]);
  GetProperty(ROTATION_VECTOR_Z_PROP)->SetDoubleValue(rotation[3]);

  Sully();
}


void
ModelObject
::GetRotation(double rotation[4]) {
  rotation[0] = GetProperty(ROTATION_ANGLE_PROP)->GetDoubleValue();
  rotation[1] = GetProperty(ROTATION_VECTOR_X_PROP)->GetDoubleValue();
  rotation[2] = GetProperty(ROTATION_VECTOR_Y_PROP)->GetDoubleValue();
  rotation[3] = GetProperty(ROTATION_VECTOR_Z_PROP)->GetDoubleValue();
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
  transform->RotateWXYZ(rotation[0], rotation[1], rotation[2], rotation[3]);
  transform->Translate(position);
  
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
