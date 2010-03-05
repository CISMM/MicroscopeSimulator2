#include <GaussianPointSpreadFunction.h>

#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkImageData.h>

#include <XMLHelper.h>

const std::string GaussianPointSpreadFunction::MEAN_ELEMENT    = "Mean";
const std::string GaussianPointSpreadFunction::SIGMA_ELEMENT   = "Sigma";


GaussianPointSpreadFunction
::GaussianPointSpreadFunction() {
  // Set up parameter names and default parameters
  m_ParameterNames.push_back("X Size (voxels)");
  m_ParameterNames.push_back("Y Size (voxels)");
  m_ParameterNames.push_back("Z Size (voxels)");
  m_ParameterNames.push_back("X Voxel Spacing (nm)");
  m_ParameterNames.push_back("Y Voxel Spacing (nm)");
  m_ParameterNames.push_back("Z Voxel Spacing (nm)");
  m_ParameterNames.push_back("X Point Center X");
  m_ParameterNames.push_back("Y Point Center Y");
  m_ParameterNames.push_back("Z Point Center Z");
  m_ParameterNames.push_back("X Standard Deviation");
  m_ParameterNames.push_back("Y Standard Deviation");
  m_ParameterNames.push_back("Z Standard Deviation");

  m_GaussianSource = ImageSourceType::New();
  unsigned long size[] = {64, 64, 64};
  m_GaussianSource->SetSize(size);
  double spacing[] = {65.0, 65.0, 200.0};
  m_GaussianSource->SetSpacing(spacing);
  ImageSourceType::ArrayType sigma;
  sigma[0] = 100.0;
  sigma[1] = 100.0;
  sigma[2] = 200.0;
  m_GaussianSource->SetSigma(sigma);
  
  m_ITKToVTKFilter = new ITKImageToVTKImage<ImageType>();
  m_ITKToVTKFilter->SetInput(m_GaussianSource->GetOutput());

  RecenterImage();
}


GaussianPointSpreadFunction
::~GaussianPointSpreadFunction() {
  delete m_ITKToVTKFilter;
}


vtkImageData*
GaussianPointSpreadFunction
::GetOutput() {
  return m_ITKToVTKFilter->GetOutput();
}


vtkAlgorithmOutput*
GaussianPointSpreadFunction
::GetOutputPort() {
  return m_ITKToVTKFilter->GetOutputPort();
}


int
GaussianPointSpreadFunction
::GetNumberOfProperties() {
  return static_cast<int>(m_ParameterNames.size());
}


std::string
GaussianPointSpreadFunction
::GetParameterName(int index) {
  try {
    return m_ParameterNames.at(index);
  } catch (...) {
  }

  return std::string("Error");
}


double
GaussianPointSpreadFunction
::GetParameterValue(int index) {
  switch (index) {
  case 0: return m_GaussianSource->GetSize()[0]; break;
  case 1: return m_GaussianSource->GetSize()[1]; break;
  case 2: return m_GaussianSource->GetSize()[2]; break;
  case 3: return m_GaussianSource->GetSpacing()[0]; break;
  case 4: return m_GaussianSource->GetSpacing()[1]; break;
  case 5: return m_GaussianSource->GetSpacing()[2]; break;
  case 6: return m_GaussianSource->GetMean()[0]; break;
  case 7: return m_GaussianSource->GetMean()[1]; break;
  case 8: return m_GaussianSource->GetMean()[2]; break;
  case 9: return m_GaussianSource->GetSigma()[0]; break;
  case 10: return m_GaussianSource->GetSigma()[1]; break;
  case 11: return m_GaussianSource->GetSigma()[2]; break;

  default: return 0.0;
  }

  return 0.0;
}


void
GaussianPointSpreadFunction
::SetParameterValue(int index, double value) {
  const unsigned long* constSize = m_GaussianSource->GetSize();
  unsigned long size[3];
  const ImageSourceType::SpacingType constSpacing = m_GaussianSource->GetSpacing();
  ImageSourceType::SpacingType spacing;
  const ImageSourceType::ArrayType constMean = m_GaussianSource->GetMean();
  ImageSourceType::ArrayType mean;
  const ImageSourceType::ArrayType constSigma = m_GaussianSource->GetSigma();
  ImageSourceType::ArrayType sigma;
  int i;

  switch (index) {
  case 0:
  case 1:
  case 2:
    constSize = m_GaussianSource->GetSize();
    for (i = 0; i < 3; i++) {
      size[i] = constSize[i];
    }
    size[index] = static_cast<unsigned long>(value);
    m_GaussianSource->SetSize(size);
    RecenterImage();
    break;

  case 3:
  case 4:
  case 5:
    spacing = m_GaussianSource->GetSpacing();
    spacing[index-3] = value;
    m_GaussianSource->SetSpacing(spacing);
    RecenterImage();
    break;

  case 6:
  case 7:
  case 8:
    mean = m_GaussianSource->GetMean();
    mean[index-6] = value;
    m_GaussianSource->SetMean(mean);
    break;

  case 9:
  case 10:
  case 11:
    sigma = m_GaussianSource->GetSigma();
    sigma[index-9] = value;
    m_GaussianSource->SetSigma(sigma);
    break;

  default: break;
  }
}


void
GaussianPointSpreadFunction
::RecenterImage() {
  const ImageSourceType::SpacingType constSpacing = m_GaussianSource->GetSpacing();
  const unsigned long*               constSize    = m_GaussianSource->GetSize();

  double origin[3];
  for (int i = 0; i < 3; i++) {
    origin[i] = -0.5 * static_cast<double>(constSize[i]-1) * constSpacing[i];
  }

  m_GaussianSource->SetOrigin(origin);
}


void
GaussianPointSpreadFunction
::GetXMLConfiguration(xmlNodePtr node) {
  xmlNodePtr root = xmlNewChild(node, NULL, BAD_CAST "GaussianPointSpreadFunction", NULL);

  char intFormat[] = "%d";
  char doubleFormat[] = "%f";
  char buf[128];

  xmlNewProp(root, BAD_CAST NAME_ATTRIBUTE.c_str(), BAD_CAST m_Name.c_str());

  xmlNodePtr sizeNode = xmlNewChild(root, NULL, BAD_CAST SIZE_ELEMENT.c_str(), NULL);
  sprintf(buf, intFormat, m_GaussianSource->GetSize()[0]);
  xmlNewProp(sizeNode, BAD_CAST X_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, intFormat, m_GaussianSource->GetSize()[1]);
  xmlNewProp(sizeNode, BAD_CAST Y_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, intFormat, m_GaussianSource->GetSize()[2]);
  xmlNewProp(sizeNode, BAD_CAST Z_ATTRIBUTE.c_str(), BAD_CAST buf);

  xmlNodePtr spacingNode = xmlNewChild(root, NULL, BAD_CAST SPACING_ELEMENT.c_str(), NULL);
  sprintf(buf, doubleFormat, m_GaussianSource->GetSpacing()[0]);
  xmlNewProp(spacingNode, BAD_CAST X_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_GaussianSource->GetSpacing()[1]);
  xmlNewProp(spacingNode, BAD_CAST Y_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_GaussianSource->GetSpacing()[2]);
  xmlNewProp(spacingNode, BAD_CAST Z_ATTRIBUTE.c_str(), BAD_CAST buf);

  xmlNodePtr meanNode = xmlNewChild(root, NULL, BAD_CAST MEAN_ELEMENT.c_str(), NULL);
  sprintf(buf, doubleFormat, m_GaussianSource->GetMean()[0]);
  xmlNewProp(meanNode, BAD_CAST X_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_GaussianSource->GetMean()[1]);
  xmlNewProp(meanNode, BAD_CAST Y_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_GaussianSource->GetMean()[2]);
  xmlNewProp(meanNode, BAD_CAST Z_ATTRIBUTE.c_str(), BAD_CAST buf);

  xmlNodePtr sigmaNode = xmlNewChild(root, NULL, BAD_CAST SIGMA_ELEMENT.c_str(), NULL);
  sprintf(buf, doubleFormat, m_GaussianSource->GetSigma()[0]);
  xmlNewProp(sigmaNode, BAD_CAST X_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_GaussianSource->GetSigma()[1]);
  xmlNewProp(sigmaNode, BAD_CAST Y_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_GaussianSource->GetSigma()[2]);
  xmlNewProp(sigmaNode, BAD_CAST Z_ATTRIBUTE.c_str(), BAD_CAST buf);
}


void
GaussianPointSpreadFunction
::RestoreFromXML(xmlNodePtr node) {
  const char* name =
    (const char*) xmlGetProp(node, BAD_CAST NAME_ATTRIBUTE.c_str());
  SetName(name);

  ImageSourceType::SizeType    size;
  xmlNodePtr sizeNode = xmlGetFirstElementChildWithName(node, BAD_CAST SIZE_ELEMENT.c_str());
  size[0] = atoi((const char*) xmlGetProp(sizeNode, BAD_CAST X_ATTRIBUTE.c_str()));
  size[1] = atoi((const char*) xmlGetProp(sizeNode, BAD_CAST Y_ATTRIBUTE.c_str()));
  size[2] = atoi((const char*) xmlGetProp(sizeNode, BAD_CAST Z_ATTRIBUTE.c_str()));
  m_GaussianSource->SetSize(size);

  ImageSourceType::SpacingType spacing;
  xmlNodePtr spacingNode = xmlGetFirstElementChildWithName(node, BAD_CAST SPACING_ELEMENT.c_str());
  spacing[0] = atof((const char*) xmlGetProp(spacingNode, BAD_CAST X_ATTRIBUTE.c_str()));
  spacing[1] = atof((const char*) xmlGetProp(spacingNode, BAD_CAST Y_ATTRIBUTE.c_str()));
  spacing[2] = atof((const char*) xmlGetProp(spacingNode, BAD_CAST Z_ATTRIBUTE.c_str()));
  m_GaussianSource->SetSpacing(spacing);

  ImageSourceType::ArrayType   mean;
  xmlNodePtr meanNode = xmlGetFirstElementChildWithName(node, BAD_CAST MEAN_ELEMENT.c_str());
  mean[0] = atof((const char*) xmlGetProp(meanNode, BAD_CAST X_ATTRIBUTE.c_str()));
  mean[1] = atof((const char*) xmlGetProp(meanNode, BAD_CAST Y_ATTRIBUTE.c_str()));
  mean[2] = atof((const char*) xmlGetProp(meanNode, BAD_CAST Z_ATTRIBUTE.c_str()));
  m_GaussianSource->SetMean(mean);

  ImageSourceType::ArrayType   sigma;
  xmlNodePtr sigmaNode = xmlGetFirstElementChildWithName(node, BAD_CAST SIGMA_ELEMENT.c_str());
  sigma[0] = atof((const char*) xmlGetProp(sigmaNode, BAD_CAST X_ATTRIBUTE.c_str()));
  sigma[1] = atof((const char*) xmlGetProp(sigmaNode, BAD_CAST Y_ATTRIBUTE.c_str()));
  sigma[2] = atof((const char*) xmlGetProp(sigmaNode, BAD_CAST Z_ATTRIBUTE.c_str()));
  m_GaussianSource->SetSigma(sigma);

  RecenterImage();

  m_GaussianSource->Update();
}
