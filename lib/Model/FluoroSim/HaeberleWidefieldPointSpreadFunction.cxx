#include <itkHaeberlePointSpreadFunctionImageSource.txx>
#include <ITKImageToVTKImage.cxx>

// WARNING: Always include the header file for this class AFTER
// including the ITK headers. Otherwise, the ITK headers will be included
// without including the implementation files, and you will have many linker
// errors.
#include <HaeberleWidefieldPointSpreadFunction.h>

#include <XMLHelper.h>

const std::string HaeberleWidefieldPointSpreadFunction::PSF_ELEMENT = "HaeberleWidefieldPointSpreadFunction";
const std::string HaeberleWidefieldPointSpreadFunction::EMISSION_WAVELENGTH_ATTRIBUTE = "EmissionWavelength";
const std::string HaeberleWidefieldPointSpreadFunction::NUMERICAL_APERTURE_ATTRIBUTE = "NumericalAperture";
const std::string HaeberleWidefieldPointSpreadFunction::MAGNIFICATION_ATTRIBUTE = "Magnification";
const std::string HaeberleWidefieldPointSpreadFunction::DESIGN_COVER_SLIP_REFRACTIVE_INDEX_ATTRIBUTE = "DesignCoverSlipRefractiveIndex";
const std::string HaeberleWidefieldPointSpreadFunction::ACTUAL_COVER_SLIP_REFRACTIVE_INDEX_ATTRIBUTE = "ActualCoverSlipRefractiveIndex";
const std::string HaeberleWidefieldPointSpreadFunction::DESIGN_COVER_SLIP_THICKNESS_ATTRIBUTE = "DesignCoverSlipThickness";
const std::string HaeberleWidefieldPointSpreadFunction::ACTUAL_COVER_SLIP_THICKNESS_ATTRIBUTE = "ActualCoverSlipThickness";
const std::string HaeberleWidefieldPointSpreadFunction::DESIGN_IMMERSION_OIL_REFRACTIVE_INDEX_ATTRIBUTE = "DesignImmersionOilRefractiveIndex";
const std::string HaeberleWidefieldPointSpreadFunction::ACTUAL_IMMERSION_OIL_REFRACTIVE_INDEX_ATTRIBUTE = "ActualImmersionOilRefractiveIndex";
const std::string HaeberleWidefieldPointSpreadFunction::DESIGN_IMMERSION_OIL_THICKNESS_ATTRIBUTE = "DesignImmersionOilThickness";
const std::string HaeberleWidefieldPointSpreadFunction::DESIGN_SPECIMEN_LAYER_REFRACTIVE_INDEX_ATTRIBUTE = "DesignSpecimenLayerRefractiveIndex";
const std::string HaeberleWidefieldPointSpreadFunction::ACTUAL_SPECIMEN_LAYER_REFRACTIVE_INDEX_ATTRIBUTE = "ActualSpecimenLayerRefractiveIndex";
const std::string HaeberleWidefieldPointSpreadFunction::ACTUAL_POINT_SOURCE_DEPTH_IN_SPECIMEN_LAYER_ATTRIBUTE = "ActualPointSourceDepthInSpecimenLayer";



HaeberleWidefieldPointSpreadFunction
::HaeberleWidefieldPointSpreadFunction() {
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

  m_HaeberleSource = ImageSourceType::New();

  m_Statistics->SetInput(m_HaeberleSource->GetOutput());

  m_ScaleFilter->SetInput(m_HaeberleSource->GetOutput());

  m_ITKToVTKFilter = new ITKImageToVTKImage<ImageType>();
  m_ITKToVTKFilter->SetInput(m_ScaleFilter->GetOutput());

  m_DerivativeX->SetInput(m_ScaleFilter->GetOutput());
  m_DerivativeY->SetInput(m_ScaleFilter->GetOutput());
  m_DerivativeZ->SetInput(m_ScaleFilter->GetOutput());

  RecenterImage();
}


HaeberleWidefieldPointSpreadFunction
::~HaeberleWidefieldPointSpreadFunction() {
  delete m_ITKToVTKFilter;
}


vtkImageData*
HaeberleWidefieldPointSpreadFunction
::GetOutput() {
  return m_ITKToVTKFilter->GetOutput();
}


vtkAlgorithmOutput*
HaeberleWidefieldPointSpreadFunction
::GetOutputPort() {
  return m_ITKToVTKFilter->GetOutputPort();
}


int
HaeberleWidefieldPointSpreadFunction
::GetNumberOfProperties() {
  return static_cast<int>(m_ParameterNames.size());
}


std::string
HaeberleWidefieldPointSpreadFunction
::GetParameterName(int index) {
  try {
    return m_ParameterNames.at(index);
  } catch (...) {
  }

  return std::string("Error");
}


double
HaeberleWidefieldPointSpreadFunction
::GetParameterValue(int index) {
  switch (index) {
  case  0: return m_SummedIntensity;
  case  1: return m_HaeberleSource->GetSize()[0]; break;
  case  2: return m_HaeberleSource->GetSize()[1]; break;
  case  3: return m_HaeberleSource->GetSize()[2]; break;
  case  4: return m_HaeberleSource->GetSpacing()[0]; break;
  case  5: return m_HaeberleSource->GetSpacing()[1]; break;
  case  6: return m_HaeberleSource->GetSpacing()[2]; break;
  case  7: return m_HaeberleSource->GetEmissionWavelength(); break;
  case  8: return m_HaeberleSource->GetNumericalAperture(); break;
  case  9: return m_HaeberleSource->GetMagnification(); break;
  case 10: return m_HaeberleSource->GetDesignCoverSlipRefractiveIndex(); break;
  case 11: return m_HaeberleSource->GetActualCoverSlipRefractiveIndex(); break;
  case 12: return m_HaeberleSource->GetDesignCoverSlipThickness(); break;
  case 13: return m_HaeberleSource->GetActualCoverSlipThickness(); break;
  case 14: return m_HaeberleSource->GetDesignImmersionOilRefractiveIndex(); break;
  case 15: return m_HaeberleSource->GetActualImmersionOilRefractiveIndex(); break;
  case 16: return m_HaeberleSource->GetDesignImmersionOilThickness(); break;
  case 17: return m_HaeberleSource->GetDesignSpecimenLayerRefractiveIndex(); break;
  case 18: return m_HaeberleSource->GetActualSpecimenLayerRefractiveIndex(); break;
  case 19: return m_HaeberleSource->GetActualPointSourceDepthInSpecimenLayer(); break;


  default: return 0.0;
  }

  return 0.0;
}


void
HaeberleWidefieldPointSpreadFunction
::SetParameterValue(int index, double value) {
  ImageSourceType::SizeType size = m_HaeberleSource->GetSize();
  ImageSourceType::SpacingType spacing = m_HaeberleSource->GetSpacing();

  switch (index) {
  case 0:
    m_SummedIntensity = value;
    break;

  case 1: case 2: case 3:
    size[index-1] = static_cast<ImageSourceType::SizeValueType>(value);
    m_HaeberleSource->SetSize(size);
    RecenterImage();
    break;

  case 4: case 5: case 6:
    spacing[index-4] = static_cast<ImageSourceType::SpacingType::ValueType>(value);
    m_HaeberleSource->SetSpacing(spacing);
    RecenterImage();
    break;

  case 7: m_HaeberleSource->SetEmissionWavelength(value); break;
  case 8: m_HaeberleSource->SetNumericalAperture(value); break;
  case 9: m_HaeberleSource->SetMagnification(value); break;
  case 10: m_HaeberleSource->SetDesignCoverSlipRefractiveIndex(value); break;
  case 11: m_HaeberleSource->SetActualCoverSlipRefractiveIndex(value); break;
  case 12: m_HaeberleSource->SetDesignCoverSlipThickness(value); break;
  case 13: m_HaeberleSource->SetActualCoverSlipThickness(value); break;
  case 14: m_HaeberleSource->SetDesignImmersionOilRefractiveIndex(value); break;
  case 15: m_HaeberleSource->SetActualImmersionOilRefractiveIndex(value); break;
  case 16: m_HaeberleSource->SetDesignImmersionOilThickness(value); break;
  case 17: m_HaeberleSource->SetDesignSpecimenLayerRefractiveIndex(value); break;
  case 18: m_HaeberleSource->SetActualSpecimenLayerRefractiveIndex(value); break;
  case 19: m_HaeberleSource->SetActualPointSourceDepthInSpecimenLayer(value); break;

  default:
    break;
  }

}


void
HaeberleWidefieldPointSpreadFunction
::RecenterImage() {
  ImageSourceType::SpacingType spacing = m_HaeberleSource->GetSpacing();
  ImageSourceType::SizeType    size    = m_HaeberleSource->GetSize();

  ImageSourceType::PointType origin;
  for (int i = 0; i < 3; i++) {
    origin[i] = -0.5 * static_cast<double>(size[i]-1) * spacing[i];
  }

  m_HaeberleSource->SetOrigin(origin);
}


void
HaeberleWidefieldPointSpreadFunction
::GetXMLConfiguration(xmlNodePtr node) {
  xmlNodePtr root = xmlNewChild(node, NULL, BAD_CAST PSF_ELEMENT.c_str(), NULL);

  char intFormat[] = "%d";
  char doubleFormat[] = "%f";
  char buf[128];

  xmlNewProp(root, BAD_CAST NAME_ATTRIBUTE.c_str(), BAD_CAST m_Name.c_str());
  sprintf(buf, "%f", GetSummedIntensity());
  xmlNewProp(root, BAD_CAST SUMMED_INTENSITY_ATTRIBUTE.c_str(), BAD_CAST buf);

  xmlNodePtr sizeNode = xmlNewChild(root, NULL, BAD_CAST SIZE_ELEMENT.c_str(), NULL);
  sprintf(buf, intFormat, m_HaeberleSource->GetSize()[0]);
  xmlNewProp(sizeNode, BAD_CAST X_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, intFormat, m_HaeberleSource->GetSize()[1]);
  xmlNewProp(sizeNode, BAD_CAST Y_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, intFormat, m_HaeberleSource->GetSize()[2]);
  xmlNewProp(sizeNode, BAD_CAST Z_ATTRIBUTE.c_str(), BAD_CAST buf);

  xmlNodePtr spacingNode = xmlNewChild(root, NULL, BAD_CAST SPACING_ELEMENT.c_str(), NULL);
  sprintf(buf, doubleFormat, m_HaeberleSource->GetSpacing()[0]);
  xmlNewProp(spacingNode, BAD_CAST X_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_HaeberleSource->GetSpacing()[1]);
  xmlNewProp(spacingNode, BAD_CAST Y_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_HaeberleSource->GetSpacing()[2]);
  xmlNewProp(spacingNode, BAD_CAST Z_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_HaeberleSource->GetEmissionWavelength());
  xmlNewProp(root, BAD_CAST EMISSION_WAVELENGTH_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_HaeberleSource->GetNumericalAperture());
  xmlNewProp(root, BAD_CAST NUMERICAL_APERTURE_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_HaeberleSource->GetMagnification());
  xmlNewProp(root, BAD_CAST MAGNIFICATION_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_HaeberleSource->GetDesignCoverSlipRefractiveIndex());
  xmlNewProp(root, BAD_CAST DESIGN_COVER_SLIP_REFRACTIVE_INDEX_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_HaeberleSource->GetActualCoverSlipRefractiveIndex());
  xmlNewProp(root, BAD_CAST ACTUAL_COVER_SLIP_REFRACTIVE_INDEX_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_HaeberleSource->GetDesignCoverSlipThickness());
  xmlNewProp(root, BAD_CAST DESIGN_COVER_SLIP_THICKNESS_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_HaeberleSource->GetActualCoverSlipThickness());
  xmlNewProp(root, BAD_CAST ACTUAL_COVER_SLIP_THICKNESS_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_HaeberleSource->GetDesignImmersionOilRefractiveIndex());
  xmlNewProp(root, BAD_CAST DESIGN_IMMERSION_OIL_REFRACTIVE_INDEX_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_HaeberleSource->GetActualImmersionOilRefractiveIndex());
  xmlNewProp(root, BAD_CAST ACTUAL_IMMERSION_OIL_REFRACTIVE_INDEX_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_HaeberleSource->GetDesignImmersionOilThickness());
  xmlNewProp(root, BAD_CAST DESIGN_IMMERSION_OIL_THICKNESS_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_HaeberleSource->GetDesignSpecimenLayerRefractiveIndex());
  xmlNewProp(root, BAD_CAST DESIGN_SPECIMEN_LAYER_REFRACTIVE_INDEX_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_HaeberleSource->GetActualSpecimenLayerRefractiveIndex());
  xmlNewProp(root, BAD_CAST ACTUAL_SPECIMEN_LAYER_REFRACTIVE_INDEX_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_HaeberleSource->GetActualPointSourceDepthInSpecimenLayer());
  xmlNewProp(root, BAD_CAST ACTUAL_POINT_SOURCE_DEPTH_IN_SPECIMEN_LAYER_ATTRIBUTE.c_str(), BAD_CAST buf);

}


void
HaeberleWidefieldPointSpreadFunction
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
  m_HaeberleSource->SetSize(size);

  ImageSourceType::SpacingType spacing;
  xmlNodePtr spacingNode = xmlGetFirstElementChildWithName(node, BAD_CAST SPACING_ELEMENT.c_str());
  spacing[0] = atof((const char*) xmlGetProp(spacingNode, BAD_CAST X_ATTRIBUTE.c_str()));
  spacing[1] = atof((const char*) xmlGetProp(spacingNode, BAD_CAST Y_ATTRIBUTE.c_str()));
  spacing[2] = atof((const char*) xmlGetProp(spacingNode, BAD_CAST Z_ATTRIBUTE.c_str()));
  m_HaeberleSource->SetSpacing(spacing);

  const char* attribute;

  attribute = (const char*) xmlGetProp(node, BAD_CAST EMISSION_WAVELENGTH_ATTRIBUTE.c_str());
  if (attribute) {
    m_HaeberleSource->SetEmissionWavelength(atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST NUMERICAL_APERTURE_ATTRIBUTE.c_str());
  if (attribute) {
    m_HaeberleSource->SetNumericalAperture(atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST MAGNIFICATION_ATTRIBUTE.c_str());
  if (attribute) {
    m_HaeberleSource->SetMagnification(atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST DESIGN_COVER_SLIP_REFRACTIVE_INDEX_ATTRIBUTE.c_str());
  if (attribute) {
    m_HaeberleSource->SetDesignCoverSlipRefractiveIndex(atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST ACTUAL_COVER_SLIP_REFRACTIVE_INDEX_ATTRIBUTE.c_str());
  if (attribute) {
    m_HaeberleSource->SetActualCoverSlipRefractiveIndex(atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST DESIGN_COVER_SLIP_THICKNESS_ATTRIBUTE.c_str());
  if (attribute) {
    m_HaeberleSource->SetDesignCoverSlipThickness(atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST ACTUAL_COVER_SLIP_THICKNESS_ATTRIBUTE.c_str());
  if (attribute) {
    m_HaeberleSource->SetActualCoverSlipThickness(atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST DESIGN_IMMERSION_OIL_REFRACTIVE_INDEX_ATTRIBUTE.c_str());
  if (attribute) {
    m_HaeberleSource->SetDesignImmersionOilRefractiveIndex(atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST ACTUAL_IMMERSION_OIL_REFRACTIVE_INDEX_ATTRIBUTE.c_str());
  if (attribute) {
    m_HaeberleSource->SetActualImmersionOilRefractiveIndex(atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST DESIGN_IMMERSION_OIL_THICKNESS_ATTRIBUTE.c_str());
  if (attribute) {
    m_HaeberleSource->SetDesignImmersionOilThickness(atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST DESIGN_SPECIMEN_LAYER_REFRACTIVE_INDEX_ATTRIBUTE.c_str());
  if (attribute) {
    m_HaeberleSource->SetDesignSpecimenLayerRefractiveIndex(atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST ACTUAL_SPECIMEN_LAYER_REFRACTIVE_INDEX_ATTRIBUTE.c_str());
  if (attribute) {
    m_HaeberleSource->SetActualSpecimenLayerRefractiveIndex(atof(attribute));
  }

  attribute = (const char*) xmlGetProp(node, BAD_CAST ACTUAL_POINT_SOURCE_DEPTH_IN_SPECIMEN_LAYER_ATTRIBUTE.c_str());
  if (attribute) {
    m_HaeberleSource->SetActualPointSourceDepthInSpecimenLayer(atof(attribute));
  }

  RecenterImage();

  m_HaeberleSource->Update();

  // It is critical to call this to ensure that the PSF is normalized after loading
  Update();
}
