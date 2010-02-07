#include <WidefieldPointSpreadFunction.h>

#include <XMLHelper.h>

const std::string WidefieldPointSpreadFunction::POINT_CENTER_ELEMENT = "PointCenter";
const std::string WidefieldPointSpreadFunction::CCD_BORDER_WIDTH_ELEMENT = "CCDBorderWidth";;
const std::string WidefieldPointSpreadFunction::EMISSION_WAVELENGTH_ATTRIBUTE = "EmissionWavelength";
const std::string WidefieldPointSpreadFunction::NUMERICAL_APERTURE_ATTRIBUTE = "NumericalAperture";
const std::string WidefieldPointSpreadFunction::MAGNIFICATION_ATTRIBUTE = "Magnification";
const std::string WidefieldPointSpreadFunction::DESIGN_COVER_SLIP_REFRACTIVE_INDEX_ATTRIBUTE = "DesignCoverSlipRefractiveIndex";
const std::string WidefieldPointSpreadFunction::ACTUAL_COVER_SLIP_REFRACTIVE_INDEX_ATTRIBUTE = "ActualCoverSlipRefractiveIndex";
const std::string WidefieldPointSpreadFunction::DESIGN_COVER_SLIP_THICKNESS_ATTRIBUTE = "DesignCoverSlipThickness";
const std::string WidefieldPointSpreadFunction::ACTUAL_COVER_SLIP_THICKNESS_ATTRIBUTE = "ActualCoverSlipThickness";
const std::string WidefieldPointSpreadFunction::DESIGN_IMMERSION_OIL_REFRACTIVE_INDEX_ATTRIBUTE = "DesignImmersionOilRefractiveIndex";
const std::string WidefieldPointSpreadFunction::ACTUAL_IMMERSION_OIL_REFRACTIVE_INDEX_ATTRIBUTE = "ActualImmersionOilRefractiveIndex";
const std::string WidefieldPointSpreadFunction::DESIGN_IMMERSION_OIL_THICKNESS_ATTRIBUTE = "DesignImmersionOilThickness";
const std::string WidefieldPointSpreadFunction::DESIGN_SPECIMEN_LAYER_REFRACTIVE_INDEX_ATTRIBUTE = "DesignSpecimenLayerRefractiveIndex";
const std::string WidefieldPointSpreadFunction::ACTUAL_SPECIMEN_LAYER_REFRACTIVE_INDEX_ATTRIBUTE = "ActualSpecimenLayerRefractiveIndex";
const std::string WidefieldPointSpreadFunction::ACTUAL_POINT_SOURCE_DEPTH_IN_SPECIMEN_LAYER_ATTRIBUTE = "ActualPointSourceDepthInSpecimenLayer";
const std::string WidefieldPointSpreadFunction::DESIGN_DISTANCE_FROM_BACK_FOCAL_PLANE_TO_DETECTOR_ATTRIBUTE = "DesignDistanceFromBackFocalPlaneToDetector";
const std::string WidefieldPointSpreadFunction::ACTUAL_DISTANCE_FROM_BACK_FOCAL_PLANE_TO_DETECTOR_ATTRIBUTE = "ActualDistanceFromBackFocalPlaneToDetector";


WidefieldPointSpreadFunction
::WidefieldPointSpreadFunction() {
  // Set up parameter names and default parameters
  m_ParameterNames.push_back("X Size (voxels)");
  m_ParameterNames.push_back("Y Size (voxels)");
  m_ParameterNames.push_back("Z Size (voxels)");
  m_ParameterNames.push_back("X Voxel Spacing (nm)");
  m_ParameterNames.push_back("Y Voxel Spacing (nm)");
  m_ParameterNames.push_back("Z Voxel Spacing (nm)");
  m_ParameterNames.push_back("X Point Center (nm)");
  m_ParameterNames.push_back("Y Point Center (nm)");
  m_ParameterNames.push_back("Z Point Center (nm)");
  m_ParameterNames.push_back("CCD Border Width X (microns)");
  m_ParameterNames.push_back("CCD Border Width Y (microns)");
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
  m_ParameterNames.push_back("Actual Point Source Depth in Specimen Layer (nm)");
  m_ParameterNames.push_back("Design Distance From Back Focal Plane to Detector (mm)");
  m_ParameterNames.push_back("Actual Distance From Back Focal Plane to Detector (mm)");

  m_GibsonLanniSource = ImageSourceType::New();

  m_ITKToVTKFilter = new ITKImageToVTKImage<ImageType>();
  m_ITKToVTKFilter->SetInput(m_GibsonLanniSource->GetOutput());

  RecenterImage();
}


WidefieldPointSpreadFunction
::~WidefieldPointSpreadFunction() {
  delete m_ITKToVTKFilter;
}


vtkImageData*
WidefieldPointSpreadFunction
::GetOutput() {
  return m_ITKToVTKFilter->GetOutput();
}


vtkAlgorithmOutput*
WidefieldPointSpreadFunction
::GetOutputPort() {
  return m_ITKToVTKFilter->GetOutputPort();
}


int
WidefieldPointSpreadFunction
::GetNumberOfProperties() {
  return static_cast<int>(m_ParameterNames.size());
}


std::string
WidefieldPointSpreadFunction
::GetParameterName(int index) {
  try {
    return m_ParameterNames.at(index);
  } catch (...) {
  }

  return std::string("Error");
}


double
WidefieldPointSpreadFunction
::GetParameterValue(int index) {
  switch (index) {
  case  0: return m_GibsonLanniSource->GetSize()[0]; break;
  case  1: return m_GibsonLanniSource->GetSize()[1]; break;
  case  2: return m_GibsonLanniSource->GetSize()[2]; break;
  case  3: return m_GibsonLanniSource->GetSpacing()[0]; break;
  case  4: return m_GibsonLanniSource->GetSpacing()[1]; break;
  case  5: return m_GibsonLanniSource->GetSpacing()[2]; break;
  case  6: return m_GibsonLanniSource->GetPointCenter()[0]; break;
  case  7: return m_GibsonLanniSource->GetPointCenter()[1]; break;
  case  8: return m_GibsonLanniSource->GetPointCenter()[2]; break;
  case  9: return m_GibsonLanniSource->GetCCDBorderWidth()[0]; break;
  case 10: return m_GibsonLanniSource->GetCCDBorderWidth()[1]; break;
  case 11: return m_GibsonLanniSource->GetEmissionWavelength(); break;
  case 12: return m_GibsonLanniSource->GetNumericalAperture(); break;
  case 13: return m_GibsonLanniSource->GetMagnification(); break;
  case 14: return m_GibsonLanniSource->GetDesignCoverSlipRefractiveIndex(); break;
  case 15: return m_GibsonLanniSource->GetActualCoverSlipRefractiveIndex(); break;
  case 16: return m_GibsonLanniSource->GetDesignCoverSlipThickness(); break;
  case 17: return m_GibsonLanniSource->GetActualCoverSlipThickness(); break;
  case 18: return m_GibsonLanniSource->GetDesignImmersionOilRefractiveIndex(); break;
  case 19: return m_GibsonLanniSource->GetActualImmersionOilRefractiveIndex(); break;
  case 20: return m_GibsonLanniSource->GetDesignImmersionOilThickness(); break;
  case 21: return m_GibsonLanniSource->GetDesignSpecimenLayerRefractiveIndex(); break;
  case 22: return m_GibsonLanniSource->GetActualSpecimenLayerRefractiveIndex(); break;
  case 23: return m_GibsonLanniSource->GetActualPointSourceDepthInSpecimenLayer(); break;
  case 24: return m_GibsonLanniSource->GetDesignDistanceFromBackFocalPlaneToDetector(); break;
  case 25: return m_GibsonLanniSource->GetActualDistanceFromBackFocalPlaneToDetector(); break;  

  default: return 0.0;
  }

  return 0.0;
}


void
WidefieldPointSpreadFunction
::SetParameterValue(int index, double value) {
  const unsigned long* constSize = m_GibsonLanniSource->GetSize();
  unsigned long size[3];
  const float* constSpacing = m_GibsonLanniSource->GetSpacing();
  float spacing[3];
  const float* constPointCenter = m_GibsonLanniSource->GetPointCenter();
  float pointCenter[3];  
  const float* constCCDBorderWidth = m_GibsonLanniSource->GetCCDBorderWidth();
  float ccdBorderWidth[2];

  for (int i = 0; i < 3; i++) {
    size[i]        = constSize[i];
    spacing[i]     = constSpacing[i];
    pointCenter[i] = constPointCenter[i];
  }
  ccdBorderWidth[0] = constCCDBorderWidth[0];
  ccdBorderWidth[1] = constCCDBorderWidth[1];

  switch (index) {
  case 0:
  case 1:
  case 2:
    constSize = m_GibsonLanniSource->GetSize();
    size[index] = static_cast<unsigned long>(value);
    m_GibsonLanniSource->SetSize(size);
    RecenterImage();
    break;

  case 3:
  case 4:
  case 5:
    spacing[index-3] = value;
    m_GibsonLanniSource->SetSpacing(spacing);
    RecenterImage();
    break;

  case 6:
  case 7:
  case 8:
    pointCenter[index-6] = value;
    m_GibsonLanniSource->SetPointCenter(pointCenter);
    break;

  case 9:
  case 10:
    ccdBorderWidth[index-9] = value;
    m_GibsonLanniSource->SetCCDBorderWidth(ccdBorderWidth);
    break;

  case 11:
    m_GibsonLanniSource->SetEmissionWavelength(value);
    break;

  case 12:
    m_GibsonLanniSource->SetNumericalAperture(value);
    break;

  case 13:
    m_GibsonLanniSource->SetMagnification(value);
    break;

  case 14:
    m_GibsonLanniSource->SetDesignCoverSlipRefractiveIndex(value);
    break;
    
  case 15:
    m_GibsonLanniSource->SetActualCoverSlipRefractiveIndex(value);
    break;
    
  case 16:
    m_GibsonLanniSource->SetDesignCoverSlipThickness(value);
    break;
    
  case 17:
    m_GibsonLanniSource->SetActualCoverSlipThickness(value);
    break;

  case 18:
    m_GibsonLanniSource->SetDesignImmersionOilRefractiveIndex(value);
    break;

  case 19:
    m_GibsonLanniSource->SetActualImmersionOilRefractiveIndex(value);
    break;

  case 20:
    m_GibsonLanniSource->SetDesignImmersionOilThickness(value);
    break;

  case 21:
    m_GibsonLanniSource->SetDesignSpecimenLayerRefractiveIndex(value);
    break;
    
  case 22:
    m_GibsonLanniSource->SetActualSpecimenLayerRefractiveIndex(value);
    break;

  case 23:
    m_GibsonLanniSource->SetActualPointSourceDepthInSpecimenLayer(value);
    break;

  case 24:
    m_GibsonLanniSource->SetDesignDistanceFromBackFocalPlaneToDetector(value);
    break;

  case 25:
    m_GibsonLanniSource->SetActualDistanceFromBackFocalPlaneToDetector(value);
    break;

  default:
    break;
  }

}


void
WidefieldPointSpreadFunction
::RecenterImage() {
  const float*         constSpacing = m_GibsonLanniSource->GetSpacing();
  const unsigned long* constSize    = m_GibsonLanniSource->GetSize();

  float origin[3];
  for (int i = 0; i < 3; i++) {
    origin[i] = -0.5 * static_cast<double>(constSize[i]-1) * constSpacing[i];
  }

  m_GibsonLanniSource->SetOrigin(origin);
}


void
WidefieldPointSpreadFunction
::GetXMLConfiguration(xmlNodePtr node) {
  xmlNodePtr root = xmlNewChild(node, NULL, BAD_CAST "WidefieldPointSpreadFunction", NULL);

  char intFormat[] = "%d";
  char doubleFormat[] = "%f";
  char buf[128];

  xmlNewProp(root, BAD_CAST NAME_ATTRIBUTE.c_str(), BAD_CAST m_Name.c_str());
  
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

  xmlNodePtr ccdBorderWidthNode = xmlNewChild(root, NULL, BAD_CAST CCD_BORDER_WIDTH_ELEMENT.c_str(), NULL);
  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetCCDBorderWidth()[0]);
  xmlNewProp(ccdBorderWidthNode, BAD_CAST X_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetCCDBorderWidth()[1]);
  xmlNewProp(ccdBorderWidthNode, BAD_CAST Y_ATTRIBUTE.c_str(), BAD_CAST buf);

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

  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetActualPointSourceDepthInSpecimenLayer());
  xmlNewProp(root, BAD_CAST ACTUAL_POINT_SOURCE_DEPTH_IN_SPECIMEN_LAYER_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetDesignDistanceFromBackFocalPlaneToDetector());
  xmlNewProp(root, BAD_CAST DESIGN_DISTANCE_FROM_BACK_FOCAL_PLANE_TO_DETECTOR_ATTRIBUTE.c_str(), BAD_CAST buf);

  sprintf(buf, doubleFormat, m_GibsonLanniSource->GetActualDistanceFromBackFocalPlaneToDetector());
  xmlNewProp(root, BAD_CAST ACTUAL_DISTANCE_FROM_BACK_FOCAL_PLANE_TO_DETECTOR_ATTRIBUTE.c_str(), BAD_CAST buf);
  
}


void
WidefieldPointSpreadFunction
::RestoreFromXML(xmlNodePtr node) {
  const char* name =
    (const char*) xmlGetProp(node, BAD_CAST NAME_ATTRIBUTE.c_str());
  SetName(name);

  unsigned long size[3];
  xmlNodePtr sizeNode = xmlGetFirstElementChildWithName(node, BAD_CAST SIZE_ELEMENT.c_str());
  size[0] = atof((const char*) xmlGetProp(sizeNode, BAD_CAST X_ATTRIBUTE.c_str()));
  size[1] = atof((const char*) xmlGetProp(sizeNode, BAD_CAST Y_ATTRIBUTE.c_str()));
  size[2] = atof((const char*) xmlGetProp(sizeNode, BAD_CAST Z_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetSize(size);

  float spacing[3];
  xmlNodePtr spacingNode = xmlGetFirstElementChildWithName(node, BAD_CAST SPACING_ELEMENT.c_str());
  spacing[0] = atof((const char*) xmlGetProp(spacingNode, BAD_CAST X_ATTRIBUTE.c_str()));
  spacing[1] = atof((const char*) xmlGetProp(spacingNode, BAD_CAST Y_ATTRIBUTE.c_str()));
  spacing[2] = atof((const char*) xmlGetProp(spacingNode, BAD_CAST Z_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetSpacing(spacing);

  float pointCenter[3];
  xmlNodePtr pointCenterNode = xmlGetFirstElementChildWithName(node, BAD_CAST POINT_CENTER_ELEMENT.c_str());
  pointCenter[0] = atof((const char*) xmlGetProp(pointCenterNode, BAD_CAST X_ATTRIBUTE.c_str()));
  pointCenter[1] = atof((const char*) xmlGetProp(pointCenterNode, BAD_CAST Y_ATTRIBUTE.c_str()));
  pointCenter[2] = atof((const char*) xmlGetProp(pointCenterNode, BAD_CAST Z_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetPointCenter(pointCenter);

  float ccdBorderWidth[2];
  xmlNodePtr ccdBorderWidthNode = xmlGetFirstElementChildWithName(node, BAD_CAST CCD_BORDER_WIDTH_ELEMENT.c_str());
  ccdBorderWidth[0] = atof((const char*) xmlGetProp(ccdBorderWidthNode, BAD_CAST X_ATTRIBUTE.c_str()));
  ccdBorderWidth[1] = atof((const char*) xmlGetProp(ccdBorderWidthNode, BAD_CAST Y_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetCCDBorderWidth(ccdBorderWidth);

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

  tmp = atof((const char*) xmlGetProp(node, BAD_CAST ACTUAL_POINT_SOURCE_DEPTH_IN_SPECIMEN_LAYER_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetActualPointSourceDepthInSpecimenLayer(tmp);

  tmp = atof((const char*) xmlGetProp(node, BAD_CAST DESIGN_DISTANCE_FROM_BACK_FOCAL_PLANE_TO_DETECTOR_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetDesignDistanceFromBackFocalPlaneToDetector(tmp);

  tmp = atof((const char*) xmlGetProp(node, BAD_CAST ACTUAL_DISTANCE_FROM_BACK_FOCAL_PLANE_TO_DETECTOR_ATTRIBUTE.c_str()));
  m_GibsonLanniSource->SetActualDistanceFromBackFocalPlaneToDetector(tmp);

  RecenterImage();
  
  m_GibsonLanniSource->Update();
}
