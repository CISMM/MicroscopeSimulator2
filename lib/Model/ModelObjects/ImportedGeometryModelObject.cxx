#include <vtkAlgorithmOutput.h>
#include <vtkCleanPolyData.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTriangleFilter.h>
#include <vtkXMLPolyDataReader.h>

#include <ImportedGeometryModelObject.h>
#include <SurfaceUniformFluorophoreProperty.h>
#include <VolumeUniformFluorophoreProperty.h>
#include <ModelObjectPropertyList.h>


const char* ImportedGeometryModelObject::OBJECT_TYPE_NAME = "ImportedGeometry";

const char* ImportedGeometryModelObject::FILE_NAME_PROP     = "File Name";
const char* ImportedGeometryModelObject::SCALE_PROP         = "Scale";
const char* ImportedGeometryModelObject::SURFACE_FLUOR_PROP = "Surface Fluorophore Model";
const char* ImportedGeometryModelObject::VOLUME_FLUOR_PROP  = "Volume Fluorophore Model";


ImportedGeometryModelObject
::ImportedGeometryModelObject(DirtyListener* dirtyListener) :
  ModelObject(dirtyListener) {
  m_ObjectTypeName = OBJECT_TYPE_NAME;
  SetName("Geometry");
  SetPickable(true);

  m_Transform = vtkSmartPointer<vtkTransform>::New();

  m_TransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  m_TransformFilter->SetTransform(m_Transform);

  SetGeometrySubAssembly("All", m_TransformFilter);

  m_TriangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
  m_TriangleFilter->PassVertsOff();
  m_TriangleFilter->PassLinesOff();
  m_TriangleFilter->SetInputConnection(m_TransformFilter->GetOutputPort());

  m_CleanPolyData = vtkSmartPointer<vtkCleanPolyData>::New();
  m_CleanPolyData->SetTolerance(0.0);
  m_CleanPolyData->ConvertLinesToPointsOff();
  m_CleanPolyData->ConvertPolysToLinesOff();
  m_CleanPolyData->ConvertStripsToPolysOff();
  m_CleanPolyData->PointMergingOn();
  m_CleanPolyData->SetInputConnection(m_TriangleFilter->GetOutputPort());

  // Set up properties
  AddProperty(new ModelObjectProperty(SCALE_PROP, 1.0, "-", true, true));
  AddProperty(new ModelObjectProperty(FILE_NAME_PROP, ModelObjectProperty::STRING_TYPE, "-", false, false));
  AddProperty(new SurfaceUniformFluorophoreProperty
              (SURFACE_FLUOR_PROP, m_CleanPolyData));
  AddProperty(new VolumeUniformFluorophoreProperty
              (VOLUME_FLUOR_PROP, m_CleanPolyData));

  // Must call this after setting up properties
  Update();
}


ImportedGeometryModelObject
::~ImportedGeometryModelObject() {

}


void
ImportedGeometryModelObject
::LoadFile(const std::string& fileName) {
  GetProperty(FILE_NAME_PROP)->SetStringValue(fileName);

  // Switch based on file extension
  size_t extensionBeginning = fileName.find_last_of(".");
  if (extensionBeginning == std::string::npos)
    // Give up and go home. Bad things will probably happen.
    return;

  extensionBeginning++;
  std::string extension = fileName.substr(extensionBeginning);
  char* charArray = new char[extension.length()+1];
  for (size_t i = 0; i < extension.length(); i++) {
    charArray[i] = tolower(extension[i]);
  }
  extension = std::string(charArray);

  if (extension == "obj") {
    vtkSmartPointer<vtkOBJReader> reader = vtkSmartPointer<vtkOBJReader>::New();
    reader->SetFileName(fileName.c_str());
    reader->Update();
    m_TransformFilter->SetInputConnection(reader->GetOutputPort());
  } else if (extension == "ply") {
    vtkSmartPointer<vtkPLYReader> reader = vtkSmartPointer<vtkPLYReader>::New();
    reader->SetFileName(fileName.c_str());
    reader->Update();
    m_TransformFilter->SetInputConnection(reader->GetOutputPort());
  } else if (extension == "vtk") {
    vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
    reader->SetFileName(fileName.c_str());
    reader->Update();
    m_TransformFilter->SetInputConnection(reader->GetOutputPort());
  } else if (extension == "vtp") {
    vtkSmartPointer<vtkXMLPolyDataReader> reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
    reader->SetFileName(fileName.c_str());
    reader->Update();
    m_TransformFilter->SetInputConnection(reader->GetOutputPort());
  }

}


void
ImportedGeometryModelObject
::RestoreFromXML(xmlNodePtr node) {
  ModelObject::RestoreFromXML(node);

  LoadFile(GetProperty(FILE_NAME_PROP)->GetStringValue());
}


void
ImportedGeometryModelObject
::Update() {
  double s = GetProperty(SCALE_PROP)->GetDoubleValue();

  if (s != m_Transform->GetScale()[0]) {
    m_Transform->Identity();
    m_Transform->Scale(s, s, s);
  }

  m_TransformFilter->Update();
}
