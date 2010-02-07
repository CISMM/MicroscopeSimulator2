#include <ImportedPointSpreadFunction.h>

#include <vtkAlgorithmOutput.h>
#include <vtkImageChangeInformation.h>


const std::string ImportedPointSpreadFunction::FILE_NAME_ATTRIBUTE  = "FileName";
const std::string ImportedPointSpreadFunction::POINT_CENTER_ELEMENT = "PointCenter";


ImportedPointSpreadFunction
::ImportedPointSpreadFunction() : PointSpreadFunction() {
  m_ImageReader = ImageSourceType::New();

  m_ITKToVTKFilter = new ITKImageToVTKImage<ImageType>();
  m_ITKToVTKFilter->SetInput(m_ImageReader->GetOutput());

  m_ChangeInformationFilter = vtkSmartPointer<vtkImageChangeInformation>::New();
  m_ChangeInformationFilter->SetInputConnection(m_ITKToVTKFilter->GetOutputPort());

  // Set up parameter names
  m_ParameterNames.push_back("X Size (voxels)");
  m_ParameterNames.push_back("Y Size (voxels)");
  m_ParameterNames.push_back("Z Size (voxels)");
  m_ParameterNames.push_back("X Voxel Spacing (nm)");
  m_ParameterNames.push_back("Y Voxel Spacing (nm)");
  m_ParameterNames.push_back("Z Voxel Spacing (nm)");
  m_ParameterNames.push_back("X Point Center (nm)");
  m_ParameterNames.push_back("Y Point Center (nm)");
  m_ParameterNames.push_back("Z Point Center (nm)");
}


ImportedPointSpreadFunction
::~ImportedPointSpreadFunction() {
  delete m_ITKToVTKFilter;
}


void
ImportedPointSpreadFunction
::SetFileName(const std::string& fileName) {
  m_FileName = fileName;
  m_ImageReader->SetFileName(m_FileName);
  m_ImageReader->UpdateLargestPossibleRegion();
  m_ChangeInformationFilter->UpdateWholeExtent();
}


std::string
ImportedPointSpreadFunction
::GetFileName() {
  return m_FileName;
}


void
ImportedPointSpreadFunction
::GetSize(int& ix, int& iy, int& iz) {

}


vtkImageData*
ImportedPointSpreadFunction
::GetOutput() {
  m_ITKToVTKFilter->Modified();
  m_ChangeInformationFilter->UpdateWholeExtent();
  return m_ChangeInformationFilter->GetOutput();
}


vtkAlgorithmOutput*
ImportedPointSpreadFunction
::GetOutputPort() {
  m_ITKToVTKFilter->Modified();
  m_ChangeInformationFilter->UpdateWholeExtent();
  return m_ChangeInformationFilter->GetOutputPort();
}


int
ImportedPointSpreadFunction
::GetNumberOfProperties() {
  return m_ParameterNames.size();
}


std::string
ImportedPointSpreadFunction
::GetParameterName(int index) {
  try {
    return m_ParameterNames.at(index);
  } catch (...) {
  }

  return std::string("Error");
}


double
ImportedPointSpreadFunction
::GetParameterValue(int index) {
  return 0.0;
}


void
ImportedPointSpreadFunction
::SetParameterValue(int index, double value) {

}


void
ImportedPointSpreadFunction
::GetXMLConfiguration(xmlNodePtr node) {
  xmlNodePtr root = xmlNewChild(node, NULL, BAD_CAST "ImportedPointSpreadFunction", NULL);

  char doubleFormat[] = "%f";
  char buf[128];

  xmlNewProp(root, BAD_CAST NAME_ATTRIBUTE.c_str(), BAD_CAST m_Name.c_str());
  xmlNewProp(root, BAD_CAST FILE_NAME_ATTRIBUTE.c_str(), BAD_CAST m_FileName.c_str());

  vtkImageData* image = m_ITKToVTKFilter->GetOutput();

  xmlNodePtr spacingNode = xmlNewChild(root, NULL, BAD_CAST SPACING_ELEMENT.c_str(), NULL);
  sprintf(buf, doubleFormat, image->GetSpacing()[0]);
  xmlNewProp(spacingNode, BAD_CAST X_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, image->GetSpacing()[1]);
  xmlNewProp(spacingNode, BAD_CAST Y_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, image->GetSpacing()[2]);
  xmlNewProp(spacingNode, BAD_CAST Z_ATTRIBUTE.c_str(), BAD_CAST buf);

  xmlNodePtr pointCenterNode = xmlNewChild(root, NULL, BAD_CAST POINT_CENTER_ELEMENT.c_str(), NULL);
  sprintf(buf, doubleFormat, m_PointCenter[0]);
  xmlNewProp(pointCenterNode, BAD_CAST X_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_PointCenter[1]);
  xmlNewProp(pointCenterNode, BAD_CAST Y_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_PointCenter[2]);
  xmlNewProp(pointCenterNode, BAD_CAST Z_ATTRIBUTE.c_str(), BAD_CAST buf);
}


void
ImportedPointSpreadFunction
::RestoreFromXML(xmlNodePtr node) {
  const char* fileName = 
    (const char*) xmlGetProp(node, BAD_CAST FILE_NAME_ATTRIBUTE.c_str());

  m_ImageReader->SetFileName(fileName);
  m_ImageReader->Update();
}


void
ImportedPointSpreadFunction
::RecenterImage() {
  
}
