#include <ImportedPointSpreadFunction.h>

#include <XMLHelper.h>

#include <vtkAlgorithmOutput.h>
#include <vtkImageChangeInformation.h>

const std::string ImportedPointSpreadFunction::FILE_NAME_ATTRIBUTE  = "FileName";
const std::string ImportedPointSpreadFunction::POINT_CENTER_ELEMENT = "PointCenter";


ImportedPointSpreadFunction
::ImportedPointSpreadFunction() : PointSpreadFunction() {
  m_ImageReader = ImageSourceType::New();
  m_PointCenter[0] = m_PointCenter[1] = m_PointCenter[2] = 0.0;

  m_ITKToVTKFilter = new ITKImageToVTKImage<ImageType>();
  m_ITKToVTKFilter->SetInput(m_ImageReader->GetOutput());

  m_ChangeInformationFilter = vtkSmartPointer<vtkImageChangeInformation>::New();
  m_ChangeInformationFilter->SetInputConnection(m_ITKToVTKFilter->GetOutputPort());
  m_ChangeInformationFilter->SetOutputSpacing(65.0, 65.0, 200.0);
  m_ChangeInformationFilter->SetOutputOrigin(0.0, 0.0, 0.0);

  // Set up parameter names
  m_ParameterNames.push_back("X Size (voxels)");
  m_ParameterNames.push_back("Y Size (voxels)");
  m_ParameterNames.push_back("Z Size (voxels)");
  m_ParameterNames.push_back("X Voxel Spacing (nm)");
  m_ParameterNames.push_back("Y Voxel Spacing (nm)");
  m_ParameterNames.push_back("Z Voxel Spacing (nm)");
  m_ParameterNames.push_back("X Center (nm)");
  m_ParameterNames.push_back("Y Center (nm)");
  m_ParameterNames.push_back("Z Center (nm)");
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
  switch (index) {
  case 0: return m_ITKToVTKFilter->GetOutput()->GetDimensions()[0]; break;
  case 1: return m_ITKToVTKFilter->GetOutput()->GetDimensions()[1]; break;
  case 2: return m_ITKToVTKFilter->GetOutput()->GetDimensions()[2]; break;
  case 3: return m_ChangeInformationFilter->GetOutputSpacing()[0]; break;
  case 4: return m_ChangeInformationFilter->GetOutputSpacing()[1]; break;
  case 5: return m_ChangeInformationFilter->GetOutputSpacing()[2]; break;
  case 6: return m_ChangeInformationFilter->GetOutputOrigin()[0]; break;
  case 7: return m_ChangeInformationFilter->GetOutputOrigin()[1]; break;
  case 8: return m_ChangeInformationFilter->GetOutputOrigin()[2]; break;

  default: return 0.0;
  }
  
  return 0.0;
}


void
ImportedPointSpreadFunction
::SetParameterValue(int index, double value) {
  double outputSpacing[3];
  m_ChangeInformationFilter->GetOutputSpacing(outputSpacing);
  double outputOrigin[3];
  m_ChangeInformationFilter->GetOutputOrigin(outputOrigin);

  switch(index) {
  case 0:
  case 1:
  case 2: 
    break; // Not editable

  case 3: 
  case 4: 
  case 5:
    outputSpacing[index-3] = value;
    m_ChangeInformationFilter->SetOutputSpacing(outputSpacing);
    
  case 6: 
  case 7:
  case 8:
    outputOrigin[index-6] = value;
    m_ChangeInformationFilter->SetOutputOrigin(outputOrigin);
  default: break;
  }
}


void
ImportedPointSpreadFunction
::GetXMLConfiguration(xmlNodePtr node) {
  xmlNodePtr root = xmlNewChild(node, NULL, BAD_CAST "ImportedPointSpreadFunction", NULL);

  char doubleFormat[] = "%f";
  char buf[128];

  xmlNewProp(root, BAD_CAST NAME_ATTRIBUTE.c_str(), BAD_CAST m_Name.c_str());
  xmlNewProp(root, BAD_CAST FILE_NAME_ATTRIBUTE.c_str(), BAD_CAST m_FileName.c_str());

  xmlNodePtr spacingNode = xmlNewChild(root, NULL, BAD_CAST SPACING_ELEMENT.c_str(), NULL);
  sprintf(buf, doubleFormat, m_ChangeInformationFilter->GetOutputSpacing()[0]);
  xmlNewProp(spacingNode, BAD_CAST X_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_ChangeInformationFilter->GetOutputSpacing()[1]);
  xmlNewProp(spacingNode, BAD_CAST Y_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_ChangeInformationFilter->GetOutputSpacing()[2]);
  xmlNewProp(spacingNode, BAD_CAST Z_ATTRIBUTE.c_str(), BAD_CAST buf);

  xmlNodePtr pointCenterNode = xmlNewChild(root, NULL, BAD_CAST POINT_CENTER_ELEMENT.c_str(), NULL);
  sprintf(buf, doubleFormat, m_ChangeInformationFilter->GetOutputOrigin()[0]);
  xmlNewProp(pointCenterNode, BAD_CAST X_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_ChangeInformationFilter->GetOutputOrigin()[1]);
  xmlNewProp(pointCenterNode, BAD_CAST Y_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_ChangeInformationFilter->GetOutputOrigin()[2]);
  xmlNewProp(pointCenterNode, BAD_CAST Z_ATTRIBUTE.c_str(), BAD_CAST buf);
}


void
ImportedPointSpreadFunction
::RestoreFromXML(xmlNodePtr node) {
  char* name = (char*) xmlGetProp(node, BAD_CAST NAME_ATTRIBUTE.c_str());
  SetName(name);

  char* fileName = (char*) xmlGetProp(node, BAD_CAST FILE_NAME_ATTRIBUTE.c_str());
  SetFileName(std::string(fileName));

  xmlNodePtr spacingNode = xmlGetFirstElementChildWithName(node, BAD_CAST SPACING_ELEMENT.c_str());
  if (spacingNode) {
    char* x = (char*) xmlGetProp(spacingNode, BAD_CAST X_ATTRIBUTE.c_str());
    char* y = (char*) xmlGetProp(spacingNode, BAD_CAST Y_ATTRIBUTE.c_str());
    char* z = (char*) xmlGetProp(spacingNode, BAD_CAST Z_ATTRIBUTE.c_str());
    if (x && y && z) {
      double spacing[3];
      spacing[0] = atof(x);
      spacing[1] = atof(y);
      spacing[2] = atof(z);
      m_ChangeInformationFilter->SetOutputSpacing(spacing);
    }
  }

  xmlNodePtr pointCenterNode = xmlGetFirstElementChildWithName(node, BAD_CAST POINT_CENTER_ELEMENT.c_str());
  if (pointCenterNode) {
    char* x = (char*) xmlGetProp(pointCenterNode, BAD_CAST X_ATTRIBUTE.c_str());
    char* y = (char*) xmlGetProp(pointCenterNode, BAD_CAST Y_ATTRIBUTE.c_str());
    char* z = (char*) xmlGetProp(pointCenterNode, BAD_CAST Z_ATTRIBUTE.c_str());
    if (x && y && z) {
      double pointCenter[3];
      pointCenter[0] = atof(x);
      pointCenter[1] = atof(y);
      pointCenter[2] = atof(z);
      m_ChangeInformationFilter->SetOutputOrigin(pointCenter);
    }
  }
}


void
ImportedPointSpreadFunction
::RecenterImage() {
  
}
