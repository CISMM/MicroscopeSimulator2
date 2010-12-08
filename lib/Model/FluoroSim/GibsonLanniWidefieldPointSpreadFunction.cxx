#include <itkGibsonLanniPointSpreadFunctionImageSource.txx>
#include <ITKImageToVTKImage.cxx>

// WARNING: Always include the header file for this class AFTER
// including the ITK headers. Otherwise, the ITK headers will be included
// without including the implementation files, and you will have many linker
// errors.
#include <GibsonLanniWidefieldPointSpreadFunction.h>

#include <XMLHelper.h>

const std::string GibsonLanniWidefieldPointSpreadFunction::PSF_ELEMENT = "GibsonLanniWidefieldPointSpreadFunction";
const std::string GibsonLanniWidefieldPointSpreadFunction::POINT_CENTER_ELEMENT = "PointCenter";
const std::string GibsonLanniWidefieldPointSpreadFunction::EMISSION_WAVELENGTH_ATTRIBUTE = "EmissionWavelength";
const std::string GibsonLanniWidefieldPointSpreadFunction::NUMERICAL_APERTURE_ATTRIBUTE = "NumericalAperture";
const std::string GibsonLanniWidefieldPointSpreadFunction::MAGNIFICATION_ATTRIBUTE = "Magnification";
const std::string GibsonLanniWidefieldPointSpreadFunction::DESIGN_COVER_SLIP_REFRACTIVE_INDEX_ATTRIBUTE = "DesignCoverSlipRefractiveIndex";
const std::string GibsonLanniWidefieldPointSpreadFunction::ACTUAL_COVER_SLIP_REFRACTIVE_INDEX_ATTRIBUTE = "ActualCoverSlipRefractiveIndex";
const std::string GibsonLanniWidefieldPointSpreadFunction::DESIGN_COVER_SLIP_THICKNESS_ATTRIBUTE = "DesignCoverSlipThickness";
const std::string GibsonLanniWidefieldPointSpreadFunction::ACTUAL_COVER_SLIP_THICKNESS_ATTRIBUTE = "ActualCoverSlipThickness";
const std::string GibsonLanniWidefieldPointSpreadFunction::DESIGN_IMMERSION_OIL_REFRACTIVE_INDEX_ATTRIBUTE = "DesignImmersionOilRefractiveIndex";
const std::string GibsonLanniWidefieldPointSpreadFunction::ACTUAL_IMMERSION_OIL_REFRACTIVE_INDEX_ATTRIBUTE = "ActualImmersionOilRefractiveIndex";
const std::string GibsonLanniWidefieldPointSpreadFunction::DESIGN_IMMERSION_OIL_THICKNESS_ATTRIBUTE = "DesignImmersionOilThickness";
const std::string GibsonLanniWidefieldPointSpreadFunction::DESIGN_SPECIMEN_LAYER_REFRACTIVE_INDEX_ATTRIBUTE = "DesignSpecimenLayerRefractiveIndex";
const std::string GibsonLanniWidefieldPointSpreadFunction::ACTUAL_SPECIMEN_LAYER_REFRACTIVE_INDEX_ATTRIBUTE = "ActualSpecimenLayerRefractiveIndex";
const std::string GibsonLanniWidefieldPointSpreadFunction::POINT_SOURCE_DEPTH_IN_SPECIMEN_LAYER_ATTRIBUTE = "ActualPointSourceDepthInSpecimenLayer";


GibsonLanniWidefieldPointSpreadFunction
::GibsonLanniWidefieldPointSpreadFunction() {
  // Set up parameter names and default parameters
  m_ParameterNames.push_back("Summed Intensity");
  m_ParameterNames.push_back("X Size (voxels)");
  m_ParameterNames.push_back("Y Size (voxels)");
  m_ParameterNames.push_back("Z Size (voxels)");
  m_ParameterNames.push_back("X Voxel Spacing (nm)");
  m_ParameterNames.push_back("Y Voxel Spacing (nm)");
  m_ParameterNames.push_back("Z Voxel Spacing (nm)");
  m_ParameterNames.push_back("X Point Center (nm)");
  m_ParameterNames.push_back("Y Point Center (nm)");
  m_ParameterNames.push_back("Z Point Center (nm)");
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
  m_ParameterNames.push_back("Point Source Depth in Specimen Layer (microns)");

  m_GibsonLanniSource = ImageSourceType::New();

  m_Statistics->SetInput(m_GibsonLanniSource->GetOutput());

  m_ScaleFilter->SetInput(m_GibsonLanniSource->GetOutput());

  m_ITKToVTKFilter = new ITKImageToVTKImage<ImageType>();
  m_ITKToVTKFilter->SetInput(m_ScaleFilter->GetOutput());

  m_DerivativeX->SetInput(m_ScaleFilter->GetOutput());
  m_DerivativeY->SetInput(m_ScaleFilter->GetOutput());
  m_DerivativeZ->SetInput(m_ScaleFilter->GetOutput());

  RecenterImage();
}


GibsonLanniWidefieldPointSpreadFunction
::~GibsonLanniWidefieldPointSpreadFunction() {
  delete m_ITKToVTKFilter;
}


vtkImageData*
GibsonLanniWidefieldPointSpreadFunction
::GetOutput() {
  return m_ITKToVTKFilter->GetOutput();
}


vtkAlgorithmOutput*
GibsonLanniWidefieldPointSpreadFunction
::GetOutputPort() {
  return m_ITKToVTKFilter->GetOutputPort();
}


int
GibsonLanniWidefieldPointSpreadFunction
::GetNumberOfProperties() {
  return static_cast<int>(m_ParameterNames.size());
}


std::string
GibsonLanniWidefieldPointSpreadFunction
::GetParameterName(int index) {
  try {
    return m_ParameterNames.at(index);
  } catch (...) {
  }

  return std::string("Error");
}


double
GibsonLanniWidefieldPointSpreadFunction
::GetParameterValue(int index) {
  switch (index) {
  case  0: return m_SummedIntensity;
  case  1: return m_GibsonLanniSource->GetSize()[0]; break;
  case  2: return m_GibsonLanniSource->GetSize()[1]; break;
  case  3: return m_GibsonLanniSource->GetSize()[2]; break;
  case  4: return m_GibsonLanniSource->GetSpacing()[0]; break;
  case  5: return m_GibsonLanniSource->GetSpacing()[1]; break;
  case  6: return m_GibsonLanniSource->GetSpacing()[2]; break;
  case  7: return m_GibsonLanniSource->GetPointCenter()[0]; break;
  case  8: return m_GibsonLanniSource->GetPointCenter()[1]; break;
  case  9: return m_GibsonLanniSource->GetPointCenter()[2]; break;
  case 10: return m_GibsonLanniSource->GetEmissionWavelength(); break;
  case 11: return m_GibsonLanniSource->GetNumericalAperture(); break;
  case 12: return m_GibsonLanniSource->GetMagnification(); break;
  case 13: return m_GibsonLanniSource->GetDesignCoverSlipRefractiveIndex(); break;
  case 14: return m_GibsonLanniSource->GetActualCoverSlipRefractiveIndex(); break;
  case 15: return m_GibsonLanniSource->GetDesignCoverSlipThickness(); break;
  case 16: return m_GibsonLanniSource->GetActualCoverSlipThickness(); break;
  case 17: return m_GibsonLanniSource->GetDesignImmersionOilRefractiveIndex(); break;
  case 18: return m_GibsonLanniSource->GetActualImmersionOilRefractiveIndex(); break;
  case 19: return m_GibsonLanniSource->GetDesignImmersionOilThickness(); break;
  case 20: return m_GibsonLanniSource->GetDesignSpecimenLayerRefractiveIndex(); break;
  case 21: return m_GibsonLanniSource->GetActualSpecimenLayerRefractiveIndex(); break;
  case 22: return m_GibsonLanniSource->GetPointSourceDepthInSpecimenLayer(); break;

  default: return 0.0;
  }

  return 0.0;
}


void
GibsonLanniWidefieldPointSpreadFunction
::SetParameterValue(int index, double value) {
  ImageSourceType::SizeType size = m_GibsonLanniSource->GetSize();
  ImageSourceType::SpacingType spacing = m_GibsonLanniSource->GetSpacing();
  ImageSourceType::PointType pointCenter = m_GibsonLanniSource->GetPointCenter();

  switch (index) {
  case 0:
    m_SummedIntensity = value;
    break;

  case 1:
  case 2:
  case 3:
    size[index-1] = static_cast<ImageSourceType::SizeValueType>(value);
    m_GibsonLanniSource->SetSize(size);
    RecenterImage();
    break;

  case 4:
  case 5:
  case 6:
    spacing[index-4] = static_cast<ImageSourceType::SpacingType::ValueType>(value);
    m_GibsonLanniSource->SetSpacing(spacing);
    RecenterImage();
    break;

  case 7:
  case 8:
  case 9:
    pointCenter[index-7] = value;
    m_GibsonLanniSource->SetPointCenter(pointCenter);
    break;

  case 10:
    m_GibsonLanniSource->SetEmissionWavelength(value);
    break;

  case 11:
    m_GibsonLanniSource->SetNumericalAperture(value);
    break;

  case 12:
    m_GibsonLanniSource->SetMagnification(value);
    break;

  case 13:
    m_GibsonLanniSource->SetDesignCoverSlipRefractiveIndex(value);
    break;

  case 14:
    m_GibsonLanniSource->SetActualCoverSlipRefractiveIndex(value);
    break;

  case 15:
    m_GibsonLanniSource->SetDesignCoverSlipThickness(value);
    break;

  case 16:
    m_GibsonLanniSource->SetActualCoverSlipThickness(value);
    break;

  case 17:
    m_GibsonLanniSource->SetDesignImmersionOilRefractiveIndex(value);
    break;

  case 18:
    m_GibsonLanniSource->SetActualImmersionOilRefractiveIndex(value);
    break;

  case 19:
    m_GibsonLanniSource->SetDesignImmersionOilThickness(value);
    break;

  case 20:
    m_GibsonLanniSource->SetDesignSpecimenLayerRefractiveIndex(value);
    break;

  case 21:
    m_GibsonLanniSource->SetActualSpecimenLayerRefractiveIndex(value);
    break;

  case 22:
    m_GibsonLanniSource->SetPointSourceDepthInSpecimenLayer(value);
    break;

  default:
    break;
  }

  UpdateGradientImage();

}


void
GibsonLanniWidefieldPointSpreadFunction
::RecenterImage() {
  ImageSourceType::SpacingType spacing = m_GibsonLanniSource->GetSpacing();
  ImageSourceType::SizeType size = m_GibsonLanniSource->GetSize();

  ImageSourceType::PointType origin;
  for (int i = 0; i < 3; i++) {
  origin[i] = -0.5 * static_cast<ImageSourceType::SpacingType::ValueType>
    (size[i]-1) * spacing[i];
  }

  m_GibsonLanniSource->SetOrigin(origin);
}


void
GibsonLanniWidefieldPointSpreadFunction
::GetXMLConfiguration(xmlNodePtr node) {
  xmlNodePtr root = xmlNewChild(node, NULL, BAD_CAST PSF_ELEMENT.c_str(), NULL);

  char intFormat[] = "%d";
  char doubleFormat[] = "%f";
  char buf[128];

  xmlNewProp(root, BAD_CAST NAME_ATTRIBUTE.c_str(), BAD_CAST m_Name.c_str());
  sprintf(buf, "%f", GetSummedIntensity());
  xmlNewProp(root, BAD_CAST SUMMED_INTENSITY_ATTRIBUTE.c_str(), BAD_CAST buf);

  xmlNodePtr sizeNode = xmlNewChild(root, NULL, BAD_CAST SIZE_ELEMENT.c_str(), NULL);
  sprintf(buf, intFormat, m_GibsonLanniSource->GetSize()[0]);
  xmlNewProp(sizeNode, BAD_CAST X_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, intFormat, m_GibsonLanniSource->GetSize()[1]);
  xmlNewProp(sizeNode, BAD_CAST Y_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, intFormat, m_GibsonLanniSource->GetSize()[2]);
  xmlNewProp(sizeNode, BAD_CAST Z_ATTRIBUTE.c_str(), BAD_CAST buf);

  xmlNodePtr spacingNode = xmlNewChild(root, NULL, BAD_CAST SPACING_ELEMENT.c_str(), NULL);
  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetSpacing()[0]);
  xmlNewProp(spacingNode, BAD_CAST X_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetSpacing()[1]);
  xmlNewProp(spacingNode, BAD_CAST Y_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetSpacing()[2]);
  xmlNewProp(spacingNode, BAD_CAST Z_ATTRIBUTE.c_str(), BAD_CAST buf);

  xmlNodePtr pointCenterNode = xmlNewChild(root, NULL, BAD_CAST POINT_CENTER_ELEMENT.c_str(), NULL);
  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetPointCenter()[0]);
  xmlNewProp(pointCenterNode, BAD_CAST X_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetPointCenter()[1]);
  xmlNewProp(pointCenterNode, BAD_CAST Y_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetPointCenter()[2]);
  xmlNewProp(pointCenterNode, BAD_CAST Z_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetEmissionWavelength());
  xmlNewProp(root, BAD_CAST EMISSION_WAVELENGTH_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetNumericalAperture());
  xmlNewProp(root, BAD_CAST NUMERICAL_APERTURE_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetMagnification());
  xmlNewProp(root, BAD_CAST MAGNIFICATION_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetDesignCoverSlipRefractiveIndex());
  xmlNewProp(root, BAD_CAST DESIGN_COVER_SLIP_REFRACTIVE_INDEX_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetActualCoverSlipRefractiveIndex());
  xmlNewProp(root, BAD_CAST ACTUAL_COVER_SLIP_REFRACTIVE_INDEX_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetDesignCoverSlipThickness());
  xmlNewProp(root, BAD_CAST DESIGN_COVER_SLIP_THICKNESS_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetActualCoverSlipThickness());
  xmlNewProp(root, BAD_CAST ACTUAL_COVER_SLIP_THICKNESS_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetDesignImmersionOilRefractiveIndex());
  xmlNewProp(root, BAD_CAST DESIGN_IMMERSION_OIL_REFRACTIVE_INDEX_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetActualImmersionOilRefractiveIndex());
  xmlNewProp(root, BAD_CAST ACTUAL_IMMERSION_OIL_REFRACTIVE_INDEX_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetDesignImmersionOilThickness());
  xmlNewProp(root, BAD_CAST DESIGN_IMMERSION_OIL_THICKNESS_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetDesignSpecimenLayerRefractiveIndex());
  xmlNewProp(root, BAD_CAST DESIGN_SPECIMEN_LAYER_REFRACTIVE_INDEX_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetActualSpecimenLayerRefractiveIndex());
  xmlNewProp(root, BAD_CAST ACTUAL_SPECIMEN_LAYER_REFRACTIVE_INDEX_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetPointSourceDepthInSpecimenLayer());
  xmlNewProp(root, BAD_CAST POINT_SOURCE_DEPTH_IN_SPECIMEN_LAYER_ATTRIBUTE.c_str(), BAD_CAST buf);

}


void
GibsonLanniWidefieldPointSpreadFunction
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
  size[0] = atof((const char*) xmlGetProp(sizeNode, BAD_CAST X_ATTRIBUTE.c_str()));
  size[1] = atof((const char*) xmlGetProp(sizeNode, BAD_CAST Y_ATTRIBUTE.c_str()));
  size[2] = atof((const char*) xmlGetProp(sizeNode, BAD_CAST Z_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetSize(size);

  ImageSourceType::SpacingType spacing;
  xmlNodePtr spacingNode = xmlGetFirstElementChildWithName(node, BAD_CAST SPACING_ELEMENT.c_str());
  spacing[0] = atof((const char*) xmlGetProp(spacingNode, BAD_CAST X_ATTRIBUTE.c_str()));
  spacing[1] = atof((const char*) xmlGetProp(spacingNode, BAD_CAST Y_ATTRIBUTE.c_str()));
  spacing[2] = atof((const char*) xmlGetProp(spacingNode, BAD_CAST Z_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetSpacing(spacing);

  ImageSourceType::PointType pointCenter;
  xmlNodePtr pointCenterNode = xmlGetFirstElementChildWithName(node, BAD_CAST POINT_CENTER_ELEMENT.c_str());
  pointCenter[0] = atof((const char*) xmlGetProp(pointCenterNode, BAD_CAST X_ATTRIBUTE.c_str()));
  pointCenter[1] = atof((const char*) xmlGetProp(pointCenterNode, BAD_CAST Y_ATTRIBUTE.c_str()));
  pointCenter[2] = atof((const char*) xmlGetProp(pointCenterNode, BAD_CAST Z_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetPointCenter(pointCenter);

  double emissionWavelength = atof((const char*) xmlGetProp(node, BAD_CAST EMISSION_WAVELENGTH_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetEmissionWavelength(emissionWavelength);

  double numericalAperture = atof((const char*) xmlGetProp(node, BAD_CAST NUMERICAL_APERTURE_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetNumericalAperture(numericalAperture);

  double magnification = atof((const char*) xmlGetProp(node, BAD_CAST MAGNIFICATION_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetMagnification(magnification);

  double tmp = atof((const char*) xmlGetProp(node, BAD_CAST DESIGN_COVER_SLIP_REFRACTIVE_INDEX_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetDesignCoverSlipRefractiveIndex(tmp);

  tmp = atof((const char*) xmlGetProp(node, BAD_CAST ACTUAL_COVER_SLIP_REFRACTIVE_INDEX_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetActualCoverSlipRefractiveIndex(tmp);

  tmp = atof((const char*) xmlGetProp(node, BAD_CAST DESIGN_COVER_SLIP_THICKNESS_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetDesignCoverSlipThickness(tmp);

  tmp = atof((const char*) xmlGetProp(node, BAD_CAST ACTUAL_COVER_SLIP_THICKNESS_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetActualCoverSlipThickness(tmp);

  tmp = atof((const char*) xmlGetProp(node, BAD_CAST DESIGN_IMMERSION_OIL_REFRACTIVE_INDEX_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetDesignImmersionOilRefractiveIndex(tmp);

  tmp = atof((const char*) xmlGetProp(node, BAD_CAST ACTUAL_IMMERSION_OIL_REFRACTIVE_INDEX_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetActualImmersionOilRefractiveIndex(tmp);

  tmp = atof((const char*) xmlGetProp(node, BAD_CAST DESIGN_IMMERSION_OIL_THICKNESS_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetDesignImmersionOilThickness(tmp);

  tmp = atof((const char*) xmlGetProp(node, BAD_CAST DESIGN_SPECIMEN_LAYER_REFRACTIVE_INDEX_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetDesignSpecimenLayerRefractiveIndex(tmp);

  tmp = atof((const char*) xmlGetProp(node, BAD_CAST ACTUAL_SPECIMEN_LAYER_REFRACTIVE_INDEX_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetActualSpecimenLayerRefractiveIndex(tmp);

  tmp = atof((const char*) xmlGetProp(node, BAD_CAST POINT_SOURCE_DEPTH_IN_SPECIMEN_LAYER_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetPointSourceDepthInSpecimenLayer(tmp);

  RecenterImage();

  m_GibsonLanniSource->Update();

  // It is critical to call this to ensure that the PSF is normalized after loading
  Update();
}
