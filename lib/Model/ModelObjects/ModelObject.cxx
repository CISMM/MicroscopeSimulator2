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
::GetRotationAngleJacobianMatrixColumn(vtkPolyData* points, int column,
                                       Matrix* matrix, double* currentRotation,
                                       double* newRotation) {

  // Iterate over the points and compute the partial derivatives for 
  // the column.
  int numPoints = points->GetNumberOfPoints();

  double cTheta = currentRotation[0];
  double cx     = currentRotation[1];
  double cy     = currentRotation[2];
  double cz     = currentRotation[3];

  // Initialize the matrix with the current rotation.
  vtkSmartPointer<vtkTransform> partials = vtkSmartPointer<vtkTransform>::New();
  partials->Identity();
  partials->RotateWXYZ(cTheta, cx, cy, cz);
  partials->PostMultiply();

  double nTheta = newRotation[0];
  double nx     = newRotation[1];
  double ny     = newRotation[2];
  double nz     = newRotation[3];
  double nThetaRadians = -vtkMath::RadiansFromDegrees(nTheta);

  // Create the partial derivative matrix with respect to nTheta and the new
  // rotation axis.
  vtkSmartPointer<vtkMatrix4x4> dTheta = vtkSmartPointer<vtkMatrix4x4>::New();
  
  double tPrime =  sin(nThetaRadians);
  double cPrime = -sin(nThetaRadians);
  double sPrime =  cos(nThetaRadians);

  // SetElement(row, column)
  dTheta->SetElement(0, 0, tPrime*nx*nx + cPrime);
  dTheta->SetElement(0, 1, tPrime*nx*ny + sPrime*nz);
  dTheta->SetElement(0, 2, tPrime*nx*nz - sPrime*ny);
  dTheta->SetElement(0, 3, 0.0);

  dTheta->SetElement(1, 0, tPrime*nx*ny - sPrime*nz);
  dTheta->SetElement(1, 1, tPrime*ny*ny + cPrime);
  dTheta->SetElement(1, 2, tPrime*ny*nz + sPrime*nx);
  dTheta->SetElement(1, 3, 0.0);

  dTheta->SetElement(2, 0, tPrime*nx*nz + sPrime*ny);
  dTheta->SetElement(2, 1, tPrime*ny*nz - sPrime*nx);
  dTheta->SetElement(2, 2, tPrime*nz*nz + cPrime);
  dTheta->SetElement(2, 3, 0.0);

  dTheta->SetElement(3, 0, 0.0);
  dTheta->SetElement(3, 1, 0.0);
  dTheta->SetElement(3, 2, 0.0);
  dTheta->SetElement(3, 3, 1.0);

  // Create the full transform for computing the Jacobian
  partials->Concatenate(dTheta);

  // Now we can form the column for the Jacobian matrix.
  for (int ptId = 0; ptId < numPoints; ptId++) {
    double* pt = points->GetPoint(ptId);
    double* jacobianVector = partials->TransformDoublePoint(pt);
    for (int dim = 0; dim < 3; dim++) {
      matrix->SetElement(ptId*3 + dim, column, jacobianVector[dim]);
    }
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

    // Get the gradient data.
    float* gradientPtr = reinterpret_cast<float*>
      (gradientData->GetPointData()->GetArray("Gradient")->GetVoidPointer(0));
    
    int numPoints = gradientData->GetNumberOfPoints();
    double* gradient = new double[3*numPoints];
    for (int ptId = 0; ptId < 3*numPoints; ptId++) {
      gradient[ptId] = static_cast<double>(gradientPtr[ptId]);
    }

    // Decide how big the Jacobian matrix is going to be
    ModelObjectProperty** transformProps = new ModelObjectProperty*[4];
    transformProps[0] = GetProperty(X_POSITION_PROP);
    transformProps[1] = GetProperty(Y_POSITION_PROP);
    transformProps[2] = GetProperty(Z_POSITION_PROP);
    transformProps[3] = GetProperty(ROTATION_ANGLE_PROP);
    bool optimizeRotation = (transformProps[3] != NULL && transformProps[3]->GetOptimize());
    
    int numColumns = 0;
    for (int i = 0; i < 4; i++) {
      if (transformProps[i] && transformProps[i]->GetOptimize()) numColumns++;
    }

    if (numColumns == 0) return;
    
    Matrix* m = new Matrix(3*numPoints, numColumns); // Jacobian for just translation
    
    int whichColumn = 0;
    for (int paramId = 0; paramId < 3; paramId++) {
      if (transformProps[paramId] && transformProps[paramId]->GetOptimize()) {
        GetTranslationJacobianMatrixColumn(gradientData, paramId, whichColumn++, m);
      }
    }

    double currentRotation[4], newRotation[4];
    if (optimizeRotation) {
      // Check if we also want to optimize the rotation axis
      ModelObjectProperty* rotationVector = GetProperty(ROTATION_VECTOR_X_PROP);
      bool optimizeAxis = (rotationVector != NULL && rotationVector->GetOptimize());

      if (optimizeAxis) {
        currentRotation[0] = GetProperty(ROTATION_ANGLE_PROP)->GetDoubleValue();
        currentRotation[1] = GetProperty(ROTATION_VECTOR_X_PROP)->GetDoubleValue();
        currentRotation[2] = GetProperty(ROTATION_VECTOR_Y_PROP)->GetDoubleValue();
        currentRotation[3] = GetProperty(ROTATION_VECTOR_Z_PROP)->GetDoubleValue();

        vtkTransform* tform = vtkTransform::New();
        tform->Identity();
        tform->RotateWXYZ(currentRotation[0], currentRotation + 1);

        // Calculate the rotation axis as the sum of each cross-product
        // between the vector from the object-relative origin to a point
        // on the geometry and the gradient vector.
        for (int i = 0; i < 4; i++) newRotation[i] = 0.0;

        for (int ptId = 0; ptId < numPoints; ptId++) {    
          double pointVector[3], cross[3];
          gradientData->GetPoint(ptId, pointVector);
          tform->TransformPoint(pointVector, pointVector);
          vtkMath::Cross(pointVector, gradient + (3*ptId), cross);
          for (int i = 0; i < 3; i++) newRotation[i+1] += cross[i];
        }

        tform->Delete();

        // Normalize the axis vector
        vtkMath::Normalize(newRotation + 1);

        std::cout << "New axis: " << newRotation[1] << ", " << newRotation[2] <<
          ", " << newRotation[3] << std::endl;

      } else { // user has chosen not to optimize the rotation axis

        // Consider the object to be not rotated.
        currentRotation[0] = 0.0;
        currentRotation[1] = 1.0;
        currentRotation[2] = 0.0;
        currentRotation[3] = 0.0;

        // Use the user-defined rotation axis as the "new" rotation.
        newRotation[0] = GetProperty(ROTATION_ANGLE_PROP)->GetDoubleValue();
        newRotation[1] = GetProperty(ROTATION_VECTOR_X_PROP)->GetDoubleValue();
        newRotation[2] = GetProperty(ROTATION_VECTOR_Y_PROP)->GetDoubleValue();
        newRotation[3] = GetProperty(ROTATION_VECTOR_Z_PROP)->GetDoubleValue();
      }

      int paramId = 3; // The rotation angle
      if (transformProps[paramId] && transformProps[paramId]->GetOptimize()) {
        GetRotationAngleJacobianMatrixColumn(gradientData, whichColumn++, m,
                                             currentRotation, newRotation);
      }
    }

    std::cout << "Matrix m: " << std::endl;
    m->PrintSelf();

    double* dParams = new double[numColumns];
    m->LinearLeastSquaresSolve(dParams, gradient);

    printf("Rescaled gradient: ");
    for (int paramId = 0; paramId < numColumns; paramId++) {
      dParams[paramId] *= stepSize;
      printf("%8.4f, ", dParams[paramId]);
    }
    printf("\n");

    // Handle the translation here
    int whichRow = 0;
    for (int paramId = 0; paramId < 3; paramId++) {
      if (transformProps[paramId] && transformProps[paramId]->GetOptimize()) {
        double value = transformProps[paramId]->GetDoubleValue() + dParams[whichRow++];
        transformProps[paramId]->SetDoubleValue(value);
      }
    }

    if (optimizeRotation) {
      // Now handle the rotation
      vtkTransform* currentTransform = vtkTransform::New();
      currentTransform->PostMultiply();
      currentTransform->Identity();
      currentTransform->RotateWXYZ(currentRotation[0], currentRotation + 1);
      
      // Set the angle on the new rotation axis as determined by the least-squares
      // solution above.
      newRotation[0] += -vtkMath::DegreesFromRadians(dParams[whichRow++]);

      currentTransform->RotateWXYZ(newRotation[0], newRotation + 1);

      // Get the final orientation
      double finalOrientation[4];
      currentTransform->GetOrientationWXYZ(finalOrientation);
      
      GetProperty(ROTATION_ANGLE_PROP)->SetDoubleValue(finalOrientation[0]);
      GetProperty(ROTATION_VECTOR_X_PROP)->SetDoubleValue(finalOrientation[1]);
      GetProperty(ROTATION_VECTOR_Y_PROP)->SetDoubleValue(finalOrientation[2]);
      GetProperty(ROTATION_VECTOR_Z_PROP)->SetDoubleValue(finalOrientation[3]);

      currentTransform->Delete();
    }

    delete[] transformProps;
    delete[] dParams;
    delete[] gradient;
    delete m;

    NormalizeRotationVector();

    Update();

  }
}
