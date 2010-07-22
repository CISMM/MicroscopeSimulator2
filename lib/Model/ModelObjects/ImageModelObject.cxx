#include <ImageModelObject.h>

#include <ModelObjectPropertyList.h>

#include <vtkContourFilter.h>


const char* ImageModelObject::OBJECT_TYPE_NAME = "ImageModel";

const char* ImageModelObject::FILE_NAME_PROP = "File Name";
const char* ImageModelObject::X_SPACING_PROP = "X Spacing";
const char* ImageModelObject::Y_SPACING_PROP = "Y Spacing";
const char* ImageModelObject::Z_SPACING_PROP = "Z Spacing";
const char* ImageModelObject::ISO_VALUE_PROP = "Isovalue";


ImageModelObject
::ImageModelObject(DirtyListener* dirtyListener) :
  ModelObject(dirtyListener, CreateProperties()) {
  m_ObjectTypeName = OBJECT_TYPE_NAME;
  SetName("Image");
  SetPickable(false);

  m_ImageReader = new ImageReader();

  // Image changer info object
  m_InfoChanger = vtkSmartPointer<vtkImageChangeInformation>::New();

  // Set up isosurface object
  m_IsosurfaceSource = vtkSmartPointer<vtkContourFilter>::New();
  m_IsosurfaceSource->SetNumberOfContours(1);  
  m_IsosurfaceSource->SetInputConnection(m_InfoChanger->GetOutputPort());

  SetGeometrySubAssembly("All", m_IsosurfaceSource);

  Update();
}


ImageModelObject
::~ImageModelObject() {
  delete m_ImageReader;
}


void
ImageModelObject
::LoadFile(const std::string& fileName) {
  GetProperty(FILE_NAME_PROP)->SetStringValue(fileName);

  m_InfoChanger->SetInputConnection(m_ImageReader->GetImageOutputPort(fileName));
}


vtkImageData*
ImageModelObject
::GetImageData() {
  return m_InfoChanger->GetOutput();
}


ImageReader::ImageType*
ImageModelObject
::GetITKImage() {
  return m_ImageReader->GetITKImage();
}


void
ImageModelObject
::GetDimensions(int dim[3]) {
  m_InfoChanger->GetOutput()->GetDimensions(dim);
}


void
ImageModelObject
::GetSpacing(double spacing[3]) {
  spacing[0] = GetProperty(X_SPACING_PROP)->GetDoubleValue();
  spacing[1] = GetProperty(Y_SPACING_PROP)->GetDoubleValue();
  spacing[2] = GetProperty(Z_SPACING_PROP)->GetDoubleValue();
}


void
ImageModelObject
::SetPosition(double position[3]) {
  // NOOP
}


void
ImageModelObject
::GetPosition(double position[3]) {
  // Always set the image origin (0,0,0)
  position[0] = 0.0;
  position[1] = 0.0;
  position[2] = 0.0;
}


void
ImageModelObject
::SetRotation(double rotation[4]) {
  // NOOP
}


void
ImageModelObject
::GetRotation(double rotation[4]) {
  // No rotation for images
  rotation[0] = 1.0;
  rotation[1] = 0.0;
  rotation[2] = 0.0;
  rotation[3] = 0.0;
}


void
ImageModelObject
::RestoreFromXML(xmlNodePtr node) {
  ModelObject::RestoreFromXML(node);

  LoadFile(GetProperty(FILE_NAME_PROP)->GetStringValue());
}


void
ImageModelObject
::Update() {
  double xSpacing = GetProperty(X_SPACING_PROP)->GetDoubleValue();
  double ySpacing = GetProperty(Y_SPACING_PROP)->GetDoubleValue();
  double zSpacing = GetProperty(Z_SPACING_PROP)->GetDoubleValue();
  m_InfoChanger->SetOutputSpacing(xSpacing, ySpacing, zSpacing);
  m_InfoChanger->SetOutputOrigin(0.5*xSpacing, 0.5*ySpacing, 0.0);

  m_IsosurfaceSource->SetValue(0, GetProperty(ISO_VALUE_PROP)->GetDoubleValue());
}


ModelObjectPropertyList*
ImageModelObject
::CreateProperties() {
  ModelObjectPropertyList* list = new ModelObjectPropertyList();

  // Set up properties
  list->AddProperty(new ModelObjectProperty(NAME_PROP, ModelObjectProperty::STRING_TYPE, "-", true, false));
  list->AddProperty(new ModelObjectProperty(VISIBLE_PROP, true, "-", true, false));
  list->AddProperty(new ModelObjectProperty(FILE_NAME_PROP, ModelObjectProperty::STRING_TYPE, "-", false, false));
  list->AddProperty(new ModelObjectProperty(X_SPACING_PROP, 100.0, "nanometers", true, false));
  list->AddProperty(new ModelObjectProperty(Y_SPACING_PROP, 100.0, "nanometers", true, false));
  list->AddProperty(new ModelObjectProperty(Z_SPACING_PROP, 100.0, "nanometers", true, false));
  list->AddProperty(new ModelObjectProperty(ISO_VALUE_PROP, 100.0, "intensity", true, false));

  return list;
}
