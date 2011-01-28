#include <itkModifiedGibsonLanniPointSpreadFunctionImageSource.txx>
#include <ITKImageToVTKImage.cxx>

// WARNING: Always include the header file for this class AFTER
// including the ITK headers. Otherwise, the ITK headers will be included
// without including the implementation files, and you will have many linker
// errors.
#include <ModifiedGibsonLanniWidefieldPointSpreadFunction.h>

#include <XMLHelper.h>

const std::string ModifiedGibsonLanniWidefieldPointSpreadFunction::PSF_ELEMENT = "ModifiedGibsonLanniWidefieldPointSpreadFunction";
const std::string ModifiedGibsonLanniWidefieldPointSpreadFunction::EMISSION_WAVELENGTH_ATTRIBUTE = "EmissionWavelength";
const std::string ModifiedGibsonLanniWidefieldPointSpreadFunction::NUMERICAL_APERTURE_ATTRIBUTE = "NumericalAperture";
const std::string ModifiedGibsonLanniWidefieldPointSpreadFunction::MAGNIFICATION_ATTRIBUTE = "Magnification";
const std::string ModifiedGibsonLanniWidefieldPointSpreadFunction::DESIGN_COVER_SLIP_REFRACTIVE_INDEX_ATTRIBUTE = "DesignCoverSlipRefractiveIndex";
const std::string ModifiedGibsonLanniWidefieldPointSpreadFunction::ACTUAL_COVER_SLIP_REFRACTIVE_INDEX_ATTRIBUTE = "ActualCoverSlipRefractiveIndex";
const std::string ModifiedGibsonLanniWidefieldPointSpreadFunction::DESIGN_COVER_SLIP_THICKNESS_ATTRIBUTE = "DesignCoverSlipThickness";
const std::string ModifiedGibsonLanniWidefieldPointSpreadFunction::ACTUAL_COVER_SLIP_THICKNESS_ATTRIBUTE = "ActualCoverSlipThickness";
const std::string ModifiedGibsonLanniWidefieldPointSpreadFunction::DESIGN_IMMERSION_OIL_REFRACTIVE_INDEX_ATTRIBUTE = "DesignImmersionOilRefractiveIndex";
const std::string ModifiedGibsonLanniWidefieldPointSpreadFunction::ACTUAL_IMMERSION_OIL_REFRACTIVE_INDEX_ATTRIBUTE = "ActualImmersionOilRefractiveIndex";
const std::string ModifiedGibsonLanniWidefieldPointSpreadFunction::DESIGN_IMMERSION_OIL_THICKNESS_ATTRIBUTE = "DesignImmersionOilThickness";
const std::string ModifiedGibsonLanniWidefieldPointSpreadFunction::DESIGN_SPECIMEN_LAYER_REFRACTIVE_INDEX_ATTRIBUTE = "DesignSpecimenLayerRefractiveIndex";
const std::string ModifiedGibsonLanniWidefieldPointSpreadFunction::ACTUAL_SPECIMEN_LAYER_REFRACTIVE_INDEX_ATTRIBUTE = "ActualSpecimenLayerRefractiveIndex";
const std::string ModifiedGibsonLanniWidefieldPointSpreadFunction::ACTUAL_POINT_SOURCE_DEPTH_IN_SPECIMEN_LAYER_ATTRIBUTE = "ActualPointSourceDepthInSpecimenLayer";
const std::string ModifiedGibsonLanniWidefieldPointSpreadFunction::GAUSSIAN_CENTER_X_ATTRIBUTE = "GaussianCenterX";
const std::string ModifiedGibsonLanniWidefieldPointSpreadFunction::GAUSSIAN_CENTER_Y_ATTRIBUTE = "GaussianCenterY";
const std::string ModifiedGibsonLanniWidefieldPointSpreadFunction::GAUSSIAN_CENTER_Z_ATTRIBUTE = "GaussianCenterZ";
const std::string ModifiedGibsonLanniWidefieldPointSpreadFunction::GAUSSIAN_SIGMA_X_ATTRIBUTE = "GaussianSigmaX";
const std::string ModifiedGibsonLanniWidefieldPointSpreadFunction::GAUSSIAN_SIGMA_Y_ATTRIBUTE = "GaussianSigmaY";
const std::string ModifiedGibsonLanniWidefieldPointSpreadFunction::GAUSSIAN_SIGMA_Z_ATTRIBUTE = "GaussianSigmaZ";
const std::string ModifiedGibsonLanniWidefieldPointSpreadFunction::GAUSSIAN_INTENSITY_SCALE_ATTRIBUTE = "GaussianIntensityScale";


ModifiedGibsonLanniWidefieldPointSpreadFunction
::ModifiedGibsonLanniWidefieldPointSpreadFunction() {
  // Set up parameter names and default parameters
  m_ParameterNames.push_back("Summed Intensity");
  m_ParameterNames.push_back("X Size (voxels)");
  m_ParameterNames.push_back("Y Size (voxels)");
  m_ParameterNames.push_back("Z Size (voxels)");
  m_ParameterNames.push_back("X Voxel Spacing (nm)");
  m_ParameterNames.push_back("Y Voxel Spacing (nm)");
  m_ParameterNames.push_back("Z Voxel Spacing (nm)");
  m_ParameterNames.push_back("Emission Wavelength (nm)");
  m_ParameterNames.push_back("Numerical Aperture");
  m_ParameterNames.push_back("Magnification");
  m_ParameterNames.push_back("Design Cover Slip Refractive Index");
  m_ParameterNames.push_back("Actual Cover Slip Refractive Index");
  m_ParameterNames.push_back("Design Cover Slip Thickness (microns)");
  m_ParameterNames.push_back("Actual Cover Slip Thickness (microns)");
  m_ParameterNames.push_back("Design Immersion Oil Refractive Index");
  m_ParameterNames.push_back("Actual Immersion Oil Refractive Index");
  m_ParameterNames.push_back("Design Immersion Oil Thickness (microns)");
  m_ParameterNames.push_back("Design Specimen Layer Refractive Index");
  m_ParameterNames.push_back("Actual Specimen Layer Refractive Index");
  m_ParameterNames.push_back("Actual Point Source Depth in Specimen Layer (microns)");
  m_ParameterNames.push_back("Gaussian Center X");
  m_ParameterNames.push_back("Gaussian Center Y");
  m_ParameterNames.push_back("Gaussian Center Z");
  m_ParameterNames.push_back("Gaussian Sigma X");
  m_ParameterNames.push_back("Gaussian Sigma Y");
  m_ParameterNames.push_back("Gaussian Sigma Z");
  m_ParameterNames.push_back("Gaussian Intensity Scale");

  m_ModifiedGibsonLanniSource = ImageSourceType::New();

  ImageSourceType::SizeType size; size.Fill(32);
  m_ModifiedGibsonLanniSource->SetSize(size);
  ImageSourceType::SpacingType spacing; spacing.Fill(65.0); spacing[2] = 100.0;
  m_ModifiedGibsonLanniSource->SetSpacing(spacing);

  m_Statistics->SetInput(m_ModifiedGibsonLanniSource->GetOutput());

  m_ScaleFilter->SetInput(m_ModifiedGibsonLanniSource->GetOutput());

  m_ITKToVTKFilter = new ITKImageToVTKImage<ImageType>();
  m_ITKToVTKFilter->SetInput(m_ScaleFilter->GetOutput());

  m_DerivativeX->SetInput(m_ScaleFilter->GetOutput());
  m_DerivativeY->SetInput(m_ScaleFilter->GetOutput());
  m_DerivativeZ->SetInput(m_ScaleFilter->GetOutput());

  RecenterImage();
}


ModifiedGibsonLanniWidefieldPointSpreadFunction
::~ModifiedGibsonLanniWidefieldPointSpreadFunction() {
  delete m_ITKToVTKFilter;
}


vtkImageData*
ModifiedGibsonLanniWidefieldPointSpreadFunction
::GetOutput() {
  return m_ITKToVTKFilter->GetOutput();
}


vtkAlgorithmOutput*
ModifiedGibsonLanniWidefieldPointSpreadFunction
::GetOutputPort() {
  return m_ITKToVTKFilter->GetOutputPort();
}


int
ModifiedGibsonLanniWidefieldPointSpreadFunction
::GetNumberOfProperties() {
  return static_cast<int>(m_ParameterNames.size());
}


std::string
ModifiedGibsonLanniWidefieldPointSpreadFunction
::GetParameterName(int index) {
  try {
    return m_ParameterNames.at(index);
  } catch (...) {
  }

  return std::string("Error");
}


double
ModifiedGibsonLanniWidefieldPointSpreadFunction
::GetParameterValue(int index) {
  switch (index) {
  case 0: return m_SummedIntensity;
  case 1:
  case 2:
  case 3:
    return m_ModifiedGibsonLanniSource->GetSize()[index-1];
    break;

  case 4:
  case 5:
  case 6:
    return m_ModifiedGibsonLanniSource->GetSpacing()[index-4];
    break;

  default: return m_ModifiedGibsonLanniSource->GetParameter(index - 7);
  }

  return 0.0;
}


void
ModifiedGibsonLanniWidefieldPointSpreadFunction
::SetParameterValue(int index, double value) {
  ImageSourceType::SizeType size = m_ModifiedGibsonLanniSource->GetSize();
  ImageSourceType::SpacingType spacing = m_ModifiedGibsonLanniSource->GetSpacing();

  switch (index) {
  case 0:
    m_SummedIntensity = value;
    break;

  case 1:
  case 2:
  case 3:
    size[index-1] = static_cast<ImageSourceType::SizeValueType>(value);
    m_ModifiedGibsonLanniSource->SetSize(size);
    RecenterImage();
    break;

  case 4:
  case 5:
  case 6:
    spacing[index-4] = static_cast<ImageSourceType::SpacingType::ValueType>(value);
    m_ModifiedGibsonLanniSource->SetSpacing(spacing);
    RecenterImage();
    break;

  default:
    m_ModifiedGibsonLanniSource->SetParameter(index - 7, value);
    break;
  }

}


void
ModifiedGibsonLanniWidefieldPointSpreadFunction
::RecenterImage() {
  ImageSourceType::SpacingType spacing = m_ModifiedGibsonLanniSource->GetSpacing();
  ImageSourceType::SizeType size = m_ModifiedGibsonLanniSource->GetSize();

  ImageSourceType::PointType origin;
  for (int i = 0; i < 3; i++) {
  origin[i] = -0.5 * static_cast<ImageSourceType::SpacingType::ValueType>
    (size[i]-1) * spacing[i];
  }

  m_ModifiedGibsonLanniSource->SetOrigin(origin);
}


void
ModifiedGibsonLanniWidefieldPointSpreadFunction
::GetXMLConfiguration(xmlNodePtr node) {
  xmlNodePtr root = xmlNewChild(node, NULL, BAD_CAST PSF_ELEMENT.c_str(), NULL);

  char intFormat[] = "%d";
  char doubleFormat[] = "%f";
  char buf[128];

  xmlNewProp(root, BAD_CAST NAME_ATTRIBUTE.c_str(), BAD_CAST m_Name.c_str());
  sprintf(buf, "%f", GetSummedIntensity());
  xmlNewProp(root, BAD_CAST SUMMED_INTENSITY_ATTRIBUTE.c_str(), BAD_CAST buf);

  xmlNodePtr sizeNode = xmlNewChild(root, NULL, BAD_CAST SIZE_ELEMENT.c_str(), NULL);
  sprintf(buf, intFormat, m_ModifiedGibsonLanniSource->GetSize()[0]);
  xmlNewProp(sizeNode, BAD_CAST X_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, intFormat, m_ModifiedGibsonLanniSource->GetSize()[1]);
  xmlNewProp(sizeNode, BAD_CAST Y_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, intFormat, m_ModifiedGibsonLanniSource->GetSize()[2]);
  xmlNewProp(sizeNode, BAD_CAST Z_ATTRIBUTE.c_str(), BAD_CAST buf);

  xmlNodePtr spacingNode = xmlNewChild(root, NULL, BAD_CAST SPACING_ELEMENT.c_str(), NULL);
  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetSpacing()[0]);
  xmlNewProp(spacingNode, BAD_CAST X_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetSpacing()[1]);
  xmlNewProp(spacingNode, BAD_CAST Y_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetSpacing()[2]);
  xmlNewProp(spacingNode, BAD_CAST Z_ATTRIBUTE.c_str(), BAD_CAST buf);

  unsigned int index = 0;
  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetParameter(index++));
  xmlNewProp(root, BAD_CAST EMISSION_WAVELENGTH_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetParameter(index++));
  xmlNewProp(root, BAD_CAST NUMERICAL_APERTURE_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetParameter(index++));
  xmlNewProp(root, BAD_CAST MAGNIFICATION_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetParameter(index++));
  xmlNewProp(root, BAD_CAST DESIGN_COVER_SLIP_REFRACTIVE_INDEX_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetParameter(index++));
  xmlNewProp(root, BAD_CAST ACTUAL_COVER_SLIP_REFRACTIVE_INDEX_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetParameter(index++));
  xmlNewProp(root, BAD_CAST DESIGN_COVER_SLIP_THICKNESS_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetParameter(index++));
  xmlNewProp(root, BAD_CAST ACTUAL_COVER_SLIP_THICKNESS_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetParameter(index++));
  xmlNewProp(root, BAD_CAST DESIGN_IMMERSION_OIL_REFRACTIVE_INDEX_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetParameter(index++));
  xmlNewProp(root, BAD_CAST ACTUAL_IMMERSION_OIL_REFRACTIVE_INDEX_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetParameter(index++));
  xmlNewProp(root, BAD_CAST DESIGN_IMMERSION_OIL_THICKNESS_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetParameter(index++));
  xmlNewProp(root, BAD_CAST DESIGN_SPECIMEN_LAYER_REFRACTIVE_INDEX_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetParameter(index++));
  xmlNewProp(root, BAD_CAST ACTUAL_SPECIMEN_LAYER_REFRACTIVE_INDEX_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetParameter(index++));
  xmlNewProp(root, BAD_CAST ACTUAL_POINT_SOURCE_DEPTH_IN_SPECIMEN_LAYER_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetParameter(index++));
  xmlNewProp(root, BAD_CAST GAUSSIAN_CENTER_X_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetParameter(index++));
  xmlNewProp(root, BAD_CAST GAUSSIAN_CENTER_Y_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetParameter(index++));
  xmlNewProp(root, BAD_CAST GAUSSIAN_CENTER_Z_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetParameter(index++));
  xmlNewProp(root, BAD_CAST GAUSSIAN_SIGMA_X_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetParameter(index++));
  xmlNewProp(root, BAD_CAST GAUSSIAN_SIGMA_Y_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetParameter(index++));
  xmlNewProp(root, BAD_CAST GAUSSIAN_SIGMA_Z_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_ModifiedGibsonLanniSource->GetParameter(index++));
  xmlNewProp(root, BAD_CAST GAUSSIAN_INTENSITY_SCALE_ATTRIBUTE.c_str(), BAD_CAST buf);
}


void
ModifiedGibsonLanniWidefieldPointSpreadFunction
::RestoreFromXML(xmlNodePtr node) {
  const char* name =
    (const char*) xmlGetProp(node, BAD_CAST NAME_ATTRIBUTE.c_str());
  SetName(name);

  char* summedIntensityStr = (char*) xmlGetProp(node, BAD_CAST SUMMED_INTENSITY_ATTRIBUTE.c_str());
  if (summedIntensityStr) {
    SetSummedIntensity(atof(summedIntensityStr));
  }

  ImageSourceType::SizeType size;
  xmlNodePtr sizeNode = xmlGetFirstElementChildWithName(node, BAD_CAST SIZE_ELEMENT.c_str());
  size[0] = atoi((const char*) xmlGetProp(sizeNode, BAD_CAST X_ATTRIBUTE.c_str()));
  size[1] = atoi((const char*) xmlGetProp(sizeNode, BAD_CAST Y_ATTRIBUTE.c_str()));
  size[2] = atoi((const char*) xmlGetProp(sizeNode, BAD_CAST Z_ATTRIBUTE.c_str()));
  m_ModifiedGibsonLanniSource->SetSize(size);

  ImageSourceType::SpacingType spacing;
  xmlNodePtr spacingNode = xmlGetFirstElementChildWithName(node, BAD_CAST SPACING_ELEMENT.c_str());
  spacing[0] = atof((const char*) xmlGetProp(spacingNode, BAD_CAST X_ATTRIBUTE.c_str()));
  spacing[1] = atof((const char*) xmlGetProp(spacingNode, BAD_CAST Y_ATTRIBUTE.c_str()));
  spacing[2] = atof((const char*) xmlGetProp(spacingNode, BAD_CAST Z_ATTRIBUTE.c_str()));
  m_ModifiedGibsonLanniSource->SetSpacing(spacing);

  const char* attribute;

  unsigned int index = 0;
  attribute = (const char*) xmlGetProp(node, BAD_CAST EMISSION_WAVELENGTH_ATTRIBUTE.c_str());
  if (attribute) {
    m_ModifiedGibsonLanniSource->SetParameter(index++, atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST NUMERICAL_APERTURE_ATTRIBUTE.c_str());
  if (attribute) {
    m_ModifiedGibsonLanniSource->SetParameter(index++, atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST MAGNIFICATION_ATTRIBUTE.c_str());
  if (attribute) {
    m_ModifiedGibsonLanniSource->SetParameter(index++, atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST DESIGN_COVER_SLIP_REFRACTIVE_INDEX_ATTRIBUTE.c_str());
  if (attribute) {
    m_ModifiedGibsonLanniSource->SetParameter(index++, atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST ACTUAL_COVER_SLIP_REFRACTIVE_INDEX_ATTRIBUTE.c_str());
  if (attribute) {
    m_ModifiedGibsonLanniSource->SetParameter(index++, atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST DESIGN_COVER_SLIP_THICKNESS_ATTRIBUTE.c_str());
  if (attribute) {
    m_ModifiedGibsonLanniSource->SetParameter(index++, atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST ACTUAL_COVER_SLIP_THICKNESS_ATTRIBUTE.c_str());
  if (attribute) {
    m_ModifiedGibsonLanniSource->SetParameter(index++, atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST DESIGN_IMMERSION_OIL_REFRACTIVE_INDEX_ATTRIBUTE.c_str());
  if (attribute) {
    m_ModifiedGibsonLanniSource->SetParameter(index++, atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST ACTUAL_IMMERSION_OIL_REFRACTIVE_INDEX_ATTRIBUTE.c_str());
  if (attribute) {
    m_ModifiedGibsonLanniSource->SetParameter(index++, atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST DESIGN_IMMERSION_OIL_THICKNESS_ATTRIBUTE.c_str());
  if (attribute) {
    m_ModifiedGibsonLanniSource->SetParameter(index++, atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST DESIGN_SPECIMEN_LAYER_REFRACTIVE_INDEX_ATTRIBUTE.c_str());
  if (attribute) {
    m_ModifiedGibsonLanniSource->SetParameter(index++, atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST ACTUAL_SPECIMEN_LAYER_REFRACTIVE_INDEX_ATTRIBUTE.c_str());
  if (attribute) {
    m_ModifiedGibsonLanniSource->SetParameter(index++, atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST ACTUAL_POINT_SOURCE_DEPTH_IN_SPECIMEN_LAYER_ATTRIBUTE.c_str());
  if (attribute) {
    m_ModifiedGibsonLanniSource->SetParameter(index++, atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST GAUSSIAN_CENTER_X_ATTRIBUTE.c_str());
  if (attribute) {
    m_ModifiedGibsonLanniSource->SetParameter(index++, atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST GAUSSIAN_CENTER_Y_ATTRIBUTE.c_str());
  if (attribute) {
    m_ModifiedGibsonLanniSource->SetParameter(index++, atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST GAUSSIAN_CENTER_Z_ATTRIBUTE.c_str());
  if (attribute) {
    m_ModifiedGibsonLanniSource->SetParameter(index++, atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST GAUSSIAN_SIGMA_X_ATTRIBUTE.c_str());
  if (attribute) {
    m_ModifiedGibsonLanniSource->SetParameter(index++, atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST GAUSSIAN_SIGMA_Y_ATTRIBUTE.c_str());
  if (attribute) {
    m_ModifiedGibsonLanniSource->SetParameter(index++, atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST GAUSSIAN_SIGMA_Z_ATTRIBUTE.c_str());
  if (attribute) {
    m_ModifiedGibsonLanniSource->SetParameter(index++, atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST GAUSSIAN_INTENSITY_SCALE_ATTRIBUTE.c_str());
  if (attribute) {
    m_ModifiedGibsonLanniSource->SetParameter(index++, atof(attribute));
  }

  RecenterImage();

  m_ModifiedGibsonLanniSource->Update();

  // It is critical to call this to ensure that the PSF is normalized after loading
  Update();
}
