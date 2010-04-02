#include <XMLHelper.h>

#include <vtkAlgorithmOutput.h>
#include <vtkImageChangeInformation.h>

#include <itkChangeInformationImageFilter.txx>
#include <itkImageFileReader.txx>
#include <ITKImageToVTKImage.cxx>

// WARNING: Always include the header file for this class AFTER
// including the ITK headers. Otherwise, the ITK headers will be included
// without including the implementation files, and you will have many linker
// errors.
#include <ImportedPointSpreadFunction.h>


const std::string ImportedPointSpreadFunction::FILE_NAME_ATTRIBUTE  = "FileName";
const std::string ImportedPointSpreadFunction::POINT_CENTER_ELEMENT = "PointCenter";


ImportedPointSpreadFunction
::ImportedPointSpreadFunction() : PointSpreadFunction() {
  m_ImageReader = ImageSourceType::New();
  m_PointCenter[0] = m_PointCenter[1] = m_PointCenter[2] = 0.0;

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

  m_ChangeInformationFilter = ChangeInfoFilterType::New();
  m_ChangeInformationFilter->ChangeDirectionOff();
  m_ChangeInformationFilter->ChangeOriginOn();
  m_ChangeInformationFilter->ChangeRegionOff();
  m_ChangeInformationFilter->ChangeSpacingOn();
  ImageType::SpacingType spacing;
  spacing[0] = 65.0;
  spacing[1] = 65.0;
  spacing[2] = 200.0;
  m_ChangeInformationFilter->SetOutputSpacing(spacing);
  ImageType::PointType origin;
  origin[0] = 0.0;
  origin[1] = 0.0;
  origin[2] = 0.0;
  m_ChangeInformationFilter->SetOutputOrigin(origin);
  m_ChangeInformationFilter->SetInput(m_ImageReader->GetOutput());

  m_ITKToVTKFilter = new ITKImageToVTKImage<ImageType>();
  m_ITKToVTKFilter->SetInput(m_ChangeInformationFilter->GetOutput());

  m_DerivativeX->SetInput(m_ChangeInformationFilter->GetOutput());
  m_DerivativeY->SetInput(m_ChangeInformationFilter->GetOutput());
  m_DerivativeZ->SetInput(m_ChangeInformationFilter->GetOutput());
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

  RecenterImage();
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
  m_ChangeInformationFilter->UpdateLargestPossibleRegion();
  m_ITKToVTKFilter->Modified();
  return m_ITKToVTKFilter->GetOutput();
}


vtkAlgorithmOutput*
ImportedPointSpreadFunction
::GetOutputPort() {
  m_ChangeInformationFilter->UpdateLargestPossibleRegion();
  m_ITKToVTKFilter->Modified();
  return m_ITKToVTKFilter->GetOutputPort();
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
  case 6: return m_PointCenter[0]; break;
  case 7: return m_PointCenter[1]; break;
  case 8: return m_PointCenter[2]; break;

  default: return 0.0;
  }
  
  return 0.0;
}


void
ImportedPointSpreadFunction
::SetParameterValue(int index, double value) {
  ImageType::SpacingType outputSpacing = 
    m_ChangeInformationFilter->GetOutputSpacing();
  ImageType::PointType outputOrigin =
    m_ChangeInformationFilter->GetOutputOrigin();

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
    RecenterImage();
    break;
    
  case 6: 
  case 7:
  case 8:
    m_PointCenter[index-6] = value;
    RecenterImage();
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
  // Change the origin here
  ImageType::RegionType region = m_ChangeInformationFilter->GetOutput()->GetLargestPossibleRegion();
  ImageType::SpacingType spacing = m_ChangeInformationFilter->GetOutputSpacing();

  ImageType::PointType origin;
  for (unsigned int i = 0; i < ImageType::GetImageDimension(); i++) {
    origin[i] = -0.5*(region.GetSize(static_cast<unsigned long>(i))-1)*spacing[i] - m_PointCenter[i];
  }
  m_ChangeInformationFilter->SetOutputOrigin(origin);

  m_ChangeInformationFilter->UpdateLargestPossibleRegion();
}
