#include <ModelObject.h>
#include <FluorophoreModelObjectProperty.h>
#include <Matrix.h>
#include <ModelObjectProperty.h>
#include <ModelObjectPropertyList.h>
#include <XMLHelper.h>

#include <vtkAppendPolyData.h>
#include <vtkMath.h>
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
::GetRotationJacobianMatrixColumn(vtkPolyData* points, const char* component, int column, Matrix* matrix) {
  
  if (!GetProperty(ROTATION_ANGLE_PROP)) {
    std::cout << "No rotation properties appear to be available in model "
      << "object '" << GetProperty(NAME_PROP)->GetStringValue() << "'"
      << std::endl;
    return;
  }

#if 0
  // We need the rotated point positions
  vtkSmartPointer<vtkTransform> transform = 
    vtkSmartPointer<vtkTransform>::New();
  double rotation[4];
  GetRotation(rotation);
  transform->RotateWXYZ(rotation[0], rotation[1], rotation[2], rotation[3]);

  // QUICK TEST TO CHECK IF ROTATION MATRIX IN VTK IS THE SAME AS THIS ONE
  double px = 100.0, py = 200.0, pz = 0;
  double* vtkAnswer = transform->TransformDoublePoint(px, py , pz);
  std::cout << "VTK says: " << vtkAnswer[0] << ", " << vtkAnswer[1] << ","
    << vtkAnswer[2] << std::endl;

  {
    double thetaDegrees = GetProperty(ROTATION_ANGLE_PROP)->GetDoubleValue();
    double theta = -vtkMath::RadiansFromDegrees(thetaDegrees);
    double vx    = GetProperty(ROTATION_VECTOR_X_PROP)->GetDoubleValue();
    double vy    = GetProperty(ROTATION_VECTOR_Y_PROP)->GetDoubleValue();
    double vz    = GetProperty(ROTATION_VECTOR_Z_PROP)->GetDoubleValue();

    double t      = 1.0 - cos(theta);
    double c      = cos(theta);
    double s      = sin(theta);

    double mx, my, mz;
    mx = (t*vx*vx +    c)*px + (t*vx*vy + s*vz)*py + (t*vx*vz - s*vy)*pz;
    my = (t*vx*vy - s*vz)*px + (t*vy*vy +    c)*py + (t*vy*vz + s*vx)*pz;
    mz = (t*vx*vz + s*vy)*px + (t*vy*vz - s*vx)*py + (t*vz*vz +    c)*pz;
    std::cout << "My  says: " << mx << ", " << my << ", " << mz << std::endl;
  }

  vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = 
    vtkTransformPolyDataFilter::New();
  transformFilter->SetTransform(transform);
  transformFilter->SetInput(points);
  transformFilter->Update();
  vtkPolyData* rotatedPoints = transformFilter->GetOutput();
#endif

  // Iterate over the points and compute the partial derivatives for 
  // the column.
  int numPoints = points->GetNumberOfPoints();

  double thetaDegrees = GetProperty(ROTATION_ANGLE_PROP)->GetDoubleValue();
  double theta = vtkMath::RadiansFromDegrees(thetaDegrees);
  double vx    = GetProperty(ROTATION_VECTOR_X_PROP)->GetDoubleValue();
  double vy    = GetProperty(ROTATION_VECTOR_Y_PROP)->GetDoubleValue();
  double vz    = GetProperty(ROTATION_VECTOR_Z_PROP)->GetDoubleValue();

  // In this representation, theta is the negative of the 
  // theta in VTK's representation.
  theta = -theta;
  double t      =  1.0 - cos(theta);
  double tPrime =  sin(theta);
  double cPrime = -sin(theta);
  double s      =  sin(theta);
  double sPrime =  cos(theta);

  // Switch based on the requested component.
  if (strcmp(component, ROTATION_ANGLE_PROP) == 0) {
    for (int ptId = 0; ptId < numPoints; ptId++) {
      double* pt = points->GetPoint(ptId);
      double x = pt[0], y = pt[1], z = pt[2];
      double jx[3];
      jx[0] = (tPrime*vx*vx + cPrime   )*x + 
              (tPrime*vx*vy + sPrime*vz)*y + 
              (tPrime*vx*vz - sPrime*vy)*z;
      jx[1] = (tPrime*vx*vy - sPrime*vz)*x +
              (tPrime*vy*vy + cPrime   )*y +
              (tPrime*vy*vz + sPrime*vx)*z;
      jx[2] = (tPrime*vx*vz + sPrime*vy)*x +
              (tPrime*vy*vz - sPrime*vx)*y +
              (tPrime*vz*vz + cPrime   )*z;

      matrix->SetElement(ptId*3 + 0, column, jx[0]);
      matrix->SetElement(ptId*3 + 1, column, jx[1]);
      matrix->SetElement(ptId*3 + 2, column, jx[2]);
    }
  } else if (strcmp(component, ROTATION_VECTOR_X_PROP) == 0) {
    for (int ptId = 0; ptId < numPoints; ptId++) {
      double* pt = points->GetPoint(ptId);
      double x = pt[0], y = pt[1], z = pt[2];
      matrix->SetElement(ptId*3 + 0, column, 2*t*vx*x + t*vy*y + t*vz*z);
      matrix->SetElement(ptId*3 + 1, column, t*vy*x + s*z);
      matrix->SetElement(ptId*3 + 2, column, t*vz*x - s*y);
    }
  } else if (strcmp(component, ROTATION_VECTOR_Y_PROP) == 0) {
    for (int ptId = 0; ptId < numPoints; ptId++) {
      double* pt = points->GetPoint(ptId);
      double x = pt[0], y = pt[1], z = pt[2];
      matrix->SetElement(ptId*3 + 0, column, t*vx*y - s*z);
      matrix->SetElement(ptId*3 + 1, column, t*vx*x + 2*t*vy*y + t*vz*z);
      matrix->SetElement(ptId*3 + 2, column, s*x + t*vz*y);
    }
  } else if (strcmp(component, ROTATION_VECTOR_Z_PROP) == 0) {
    for (int ptId = 0; ptId < numPoints; ptId++) {
      double* pt = points->GetPoint(ptId);
      double x = pt[0], y = pt[1], z = pt[2];
      matrix->SetElement(ptId*3 + 0, column, s*y + t*vx*z);
      matrix->SetElement(ptId*3 + 1, column, -s*x + t*vy*z);
      matrix->SetElement(ptId*3 + 2, column, t*vx*x + t*vy*y + 2*t*vz);
    }
  } else {
    std::cout 
      << "Error in ModelObject::GetRotationJacobianMatrixColumn. "
      << "Requested component '" << component << "' unknown." << std::endl;
  }
}


void
ModelObject
::GetTranslationJacobianMatrixColumn(vtkPolyData* points, int axis, 
                                     int column, Matrix* matrix) {
  double pattern[3];
  switch (axis) {
  case 0:
    pattern[0] = 1.0; // dx / dt_x
    pattern[1] = 0.0; // dy / dt_x
    pattern[2] = 0.0; // dz / dt_x
    break;

  case 1:
    pattern[0] = 0.0; // dx / dt_y
    pattern[1] = 1.0; // dy / dt_y
    pattern[2] = 0.0; // dz / dt_y
    break;

  case 2:
    pattern[0] = 0.0; // dx / dt_z
    pattern[1] = 0.0; // dy / dt_z
    pattern[2] = 1.0; // dz / dt_z
    break;

  default:
    std::cout
      << "Error in ModelObject::GetTranslationJacobianMatrixColumn. "
      << "Requested axis " << axis << "." << std::endl;
    return;
    break;
  }

  int numPoints = points->GetNumberOfPoints();
  for (int ptId = 0; ptId < numPoints; ptId++) {
    for (int j = 0; j < 3; j++) {
      matrix->SetElement(3*ptId + j, column, pattern[j]);
    }
  }
}


void
ModelObject
::NormalizeRotationVector() {
  double x = GetProperty(ROTATION_VECTOR_X_PROP)->GetDoubleValue();
  double y = GetProperty(ROTATION_VECTOR_Y_PROP)->GetDoubleValue();
  double z = GetProperty(ROTATION_VECTOR_Z_PROP)->GetDoubleValue();
  double norm = sqrt(x*x + y*y + z*z);
  if (norm != 0.0) {
    x /= norm;
    y /= norm;
    z /= norm;
  }

  GetProperty(ROTATION_VECTOR_X_PROP)->SetDoubleValue(x);
  GetProperty(ROTATION_VECTOR_Y_PROP)->SetDoubleValue(y);
  GetProperty(ROTATION_VECTOR_Z_PROP)->SetDoubleValue(z);
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

    // Scale the gradient
    float* gradientPtr = reinterpret_cast<float*>
      (gradientData->GetPointData()->GetArray("Gradient")->GetVoidPointer(0));

    int numPoints = gradientData->GetNumberOfPoints();
    Matrix* m = new Matrix(3*numPoints, 7); // Jacobian for just translation
    
    GetTranslationJacobianMatrixColumn(gradientData, 0, 0, m);
    GetTranslationJacobianMatrixColumn(gradientData, 1, 1, m);
    GetTranslationJacobianMatrixColumn(gradientData, 2, 2, m);
    
    GetRotationJacobianMatrixColumn(gradientData, ROTATION_ANGLE_PROP,    3, m);
    GetRotationJacobianMatrixColumn(gradientData, ROTATION_VECTOR_X_PROP, 4, m);
    GetRotationJacobianMatrixColumn(gradientData, ROTATION_VECTOR_Y_PROP, 5, m);
    GetRotationJacobianMatrixColumn(gradientData, ROTATION_VECTOR_Z_PROP, 6, m);

    std::cout << "Matrix m: " << std::endl;
    m->PrintSelf();

    double* theta    = new double[7]; // TODO - change this to the real number of parameters
    double* gradient = new double[3*numPoints];

    for (int i = 0; i < 3*numPoints; i++) {
      gradient[i] = stepSize * static_cast<double>(gradientPtr[i]);
    }

    m->LinearLeastSquaresSolve(theta, gradient);
    
    std::cout << "Theta: " 
              << theta[0] << ", " 
              << theta[1] << ", " 
              << theta[2] << ", "
              << theta[3] << ", "
              << theta[4] << ", "
              << theta[5] << ", "
              << theta[6] << std::endl;

    ModelObjectProperty* transformProperties[7];
    transformProperties[0] = GetProperty(X_POSITION_PROP);
    transformProperties[1] = GetProperty(Y_POSITION_PROP);
    transformProperties[2] = GetProperty(Z_POSITION_PROP);
    transformProperties[3] = GetProperty(ROTATION_ANGLE_PROP);
    transformProperties[4] = GetProperty(ROTATION_VECTOR_X_PROP);
    transformProperties[5] = GetProperty(ROTATION_VECTOR_Y_PROP);
    transformProperties[6] = GetProperty(ROTATION_VECTOR_Z_PROP);
  
    for (int paramId = 0; paramId < 7; paramId++) {
      if (transformProperties[paramId] && transformProperties[paramId]->GetOptimize()) {
        double value = transformProperties[paramId]->GetDoubleValue() + theta[paramId];
        transformProperties[paramId]->SetDoubleValue(value);
      }
    }

    delete[] theta;
    delete[] gradient;

    NormalizeRotationVector();

    Update();

  }
}
