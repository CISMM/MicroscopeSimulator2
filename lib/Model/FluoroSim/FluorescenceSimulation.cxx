#include <PointSpreadFunctionList.h>
#include <PointSpreadFunction.h>

#include <FluorescenceSimulation.h>
#include <FluorescenceImageSource.h>

const char* FluorescenceSimulation::FOCAL_PLANE_INDEX_ATT = "focalPlaneIndex";
const char* FluorescenceSimulation::FOCAL_PLANE_SPACING_ATT = "focalPlaneSpacing";
const char* FluorescenceSimulation::NUMBER_OF_FOCAL_PLANES_ATT = "numberOfFocalPlanes";
const char* FluorescenceSimulation::USE_CUSTOM_FOCAL_PLANE_POSITIONS_ATT = "useCustomFocalPlanePositions";
const char* FluorescenceSimulation::EXPOSURE_TIME_ATT = "exposureTime";
const char* FluorescenceSimulation::PIXEL_SIZE_ATT = "pixelSize";
const char* FluorescenceSimulation::PSF_NAME_ATT = "psfName";
const char* FluorescenceSimulation::IMAGE_WIDTH_ATT = "imageWidth";
const char* FluorescenceSimulation::IMAGE_HEIGHT_ATT = "imageHeight";
const char* FluorescenceSimulation::SHEAR_IN_X_ATT = "shearInX";
const char* FluorescenceSimulation::SHEAR_IN_Y_ATT = "shearInY";
const char* FluorescenceSimulation::ADD_GAUSSIAN_NOISE_ATT = "addGaussianNoise";
const char* FluorescenceSimulation::NOISE_STD_DEV_ATT = "noiseStdDev";
const char* FluorescenceSimulation::NOISE_MEAN_ATT = "noiseMean";
const char* FluorescenceSimulation::SHOW_IMAGE_VOLUME_OUTLINE_ATT = "showImageVolumeOutline";
const char* FluorescenceSimulation::SHOW_REFERENCE_PLANE_ATT = "showRefPlane";
const char* FluorescenceSimulation::SHOW_REFERENCE_GRID_ATT = "showRefGrid";
const char* FluorescenceSimulation::REFERENCE_GRID_SPACING_ATT = "refGridSpacing";
const char* FluorescenceSimulation::SUPERIMPOSE_FLUORESCENCE_IMAGE_ATT = "superimposeImage";
const char* FluorescenceSimulation::MIN_INTENSITY_LEVEL_ATT = "minimumIntensityLevel";
const char* FluorescenceSimulation::MAX_INTENSITY_LEVEL_ATT = "maximumIntensityLevel";


FluorescenceSimulation
::FluorescenceSimulation(DirtyListener* dirtyListener) {
  m_DirtyListener = dirtyListener;
  m_PSFList = new PointSpreadFunctionList();

  m_FluoroImageSource = NULL;

  NewSimulation();
}


FluorescenceSimulation
::~FluorescenceSimulation() {
  delete m_PSFList;
}


void
FluorescenceSimulation
::NewSimulation() {
  m_FocalPlaneIndex = 0.0;
  m_FocalPlaneSpacing =  200.0;
  m_NumberOfFocalPlanes = 30;
  m_UseCustomFocalPlanePositions = false;
  m_ActivePSFIndex = -1;
  m_Exposure    = 1.0;
  m_PixelSize   = 65.0;
  m_ImageWidth  = 200;
  m_ImageHeight = 200;
  m_ShearInX    = 0.0;
  m_ShearInY    = 0.0;
  m_AddGaussianNoise = false;
  m_NoiseStdDev = 0.0;
  m_NoiseMean   = 0.0;
  m_ShowImageVolumeOutline = false;
  m_ShowReferencePlane = true;
  m_ShowReferenceGrid = true;
  m_ReferenceGridSpacing = 1000.0;
  m_SuperimposeFluorescenceImage = false;
  m_MinimumIntensityLevel = 0.0;
  m_MaximumIntensityLevel = 0.0;
}


void
FluorescenceSimulation
::GetXMLConfiguration(xmlNodePtr node) {
  char trueStr[] = "true";
  char falseStr[] = "false";
  char buf[128];
  sprintf(buf, "%d", GetFocalPlaneIndex());
  xmlNewProp(node, BAD_CAST FOCAL_PLANE_INDEX_ATT, BAD_CAST buf);
  sprintf(buf, "%f", GetFocalPlaneSpacing());
  xmlNewProp(node, BAD_CAST FOCAL_PLANE_SPACING_ATT, BAD_CAST buf);
  sprintf(buf, "%d", GetNumberOfFocalPlanes());
  xmlNewProp(node, BAD_CAST NUMBER_OF_FOCAL_PLANES_ATT, BAD_CAST buf);
  sprintf(buf, "%s", GetUseCustomFocalPlanePositions() ? "true" : "false");
  xmlNewProp(node, BAD_CAST USE_CUSTOM_FOCAL_PLANE_POSITIONS_ATT, BAD_CAST buf);
  sprintf(buf, "%f", GetExposure());
  xmlNewProp(node, BAD_CAST EXPOSURE_TIME_ATT, BAD_CAST buf);
  sprintf(buf, "%f", GetPixelSize());
  xmlNewProp(node, BAD_CAST PIXEL_SIZE_ATT, BAD_CAST buf);

  PointSpreadFunction* activePSF = this->GetActivePointSpreadFunction();
  if (activePSF == NULL) {
    sprintf(buf, "None");
  } else {
    sprintf(buf, "%s", activePSF->GetName().c_str());
  }
  xmlNewProp(node, BAD_CAST PSF_NAME_ATT, BAD_CAST buf);

  sprintf(buf, "%d", GetImageWidth());
  xmlNewProp(node, BAD_CAST IMAGE_WIDTH_ATT, BAD_CAST buf);
  sprintf(buf, "%d", GetImageHeight());
  xmlNewProp(node, BAD_CAST IMAGE_HEIGHT_ATT, BAD_CAST buf);
  sprintf(buf, "%f", GetShearInX());
  xmlNewProp(node, BAD_CAST SHEAR_IN_X_ATT, BAD_CAST buf);
  sprintf(buf, "%f", GetShearInY());
  xmlNewProp(node, BAD_CAST SHEAR_IN_Y_ATT, BAD_CAST buf);
  sprintf(buf, "%s", GetAddGaussianNoise() ? trueStr : falseStr);
  xmlNewProp(node, BAD_CAST ADD_GAUSSIAN_NOISE_ATT, BAD_CAST buf);
  sprintf(buf, "%f", GetNoiseStdDev());
  xmlNewProp(node, BAD_CAST NOISE_STD_DEV_ATT, BAD_CAST buf);
  sprintf(buf, "%f", GetNoiseMean());
  xmlNewProp(node, BAD_CAST NOISE_MEAN_ATT, BAD_CAST buf);
  sprintf(buf, "%s", GetShowImageVolumeOutline() ? trueStr : falseStr);
  xmlNewProp(node, BAD_CAST SHOW_IMAGE_VOLUME_OUTLINE_ATT, BAD_CAST buf);
  sprintf(buf, "%s", GetShowReferencePlane() ? trueStr : falseStr);
  xmlNewProp(node, BAD_CAST SHOW_REFERENCE_PLANE_ATT, BAD_CAST buf);
  sprintf(buf, "%s", GetShowReferenceGrid() ? trueStr : falseStr);
  xmlNewProp(node, BAD_CAST SHOW_REFERENCE_GRID_ATT, BAD_CAST buf);
  sprintf(buf, "%f", GetReferenceGridSpacing());
  xmlNewProp(node, BAD_CAST REFERENCE_GRID_SPACING_ATT, BAD_CAST buf);
  sprintf(buf, "%s", GetSuperimposeFluorescenceImage() ? trueStr : falseStr);
  xmlNewProp(node, BAD_CAST SUPERIMPOSE_FLUORESCENCE_IMAGE_ATT, BAD_CAST buf);
  sprintf(buf, "%f", GetMinimumIntensityLevel());
  xmlNewProp(node, BAD_CAST MIN_INTENSITY_LEVEL_ATT, BAD_CAST buf);
  sprintf(buf, "%f", GetMaximumIntensityLevel());
  xmlNewProp(node, BAD_CAST MAX_INTENSITY_LEVEL_ATT, BAD_CAST buf);
}


void
FluorescenceSimulation
::RestoreFromXML(xmlNodePtr node) {
  std::string trueValue("true");

  char* focalPlaneIndexStr = (char*) xmlGetProp(node, BAD_CAST FOCAL_PLANE_INDEX_ATT);
  if (focalPlaneIndexStr) {
    SetFocalPlaneIndex(atof(focalPlaneIndexStr));
  }

  char* focalPlaneSpacingStr = (char*) xmlGetProp(node, BAD_CAST FOCAL_PLANE_SPACING_ATT);
  if (focalPlaneSpacingStr) {
    SetFocalPlaneSpacing(atof(focalPlaneSpacingStr));
  }

  char* numberOfFocalPlanesStr = (char *) xmlGetProp(node, BAD_CAST NUMBER_OF_FOCAL_PLANES_ATT);
  if (numberOfFocalPlanesStr) {
    SetNumberOfFocalPlanes((unsigned int) atoi(numberOfFocalPlanesStr));
  }

  char* useCustomFocalPlanePositionsStr = (char *) xmlGetProp(node, BAD_CAST USE_CUSTOM_FOCAL_PLANE_POSITIONS_ATT);
  if (useCustomFocalPlanePositionsStr) {
    SetUseCustomFocalPlanePositions(std::string(useCustomFocalPlanePositionsStr) == "true");
  }

  char* exposureTimeStr = (char*) xmlGetProp(node, BAD_CAST EXPOSURE_TIME_ATT);
  if (exposureTimeStr) {
    SetExposure(atof(exposureTimeStr));
  }

  char* pixelSizeStr = (char*) xmlGetProp(node, BAD_CAST PIXEL_SIZE_ATT);
  if (pixelSizeStr) {
    SetPixelSize(atof(pixelSizeStr));
  }

  char* psfNameStr = (char*) xmlGetProp(node, BAD_CAST PSF_NAME_ATT);
  if (psfNameStr) {
    SetActivePSFByName(std::string(psfNameStr));
  }

  char* imageWidthStr = (char*) xmlGetProp(node, BAD_CAST IMAGE_WIDTH_ATT);
  if (imageWidthStr) {
    SetImageWidth((unsigned int) atoi(imageWidthStr));
  }

  char* imageHeightStr = (char*) xmlGetProp(node, BAD_CAST IMAGE_HEIGHT_ATT);
  if (imageHeightStr) {
    SetImageHeight((unsigned int) atoi(imageHeightStr));
  }

  char* shearInXStr = (char*) xmlGetProp(node, BAD_CAST SHEAR_IN_X_ATT);
  if (shearInXStr) {
    SetShearInX(atof(shearInXStr));
  }

  char* shearInYStr = (char*) xmlGetProp(node, BAD_CAST SHEAR_IN_Y_ATT);
  if (shearInYStr) {
    SetShearInY(atof(shearInYStr));
  }

  char* addGaussianNoiseStr = (char*) xmlGetProp(node, BAD_CAST ADD_GAUSSIAN_NOISE_ATT);
  if (addGaussianNoiseStr) {
    SetAddGaussianNoise((std::string(addGaussianNoiseStr) == trueValue));
  }

  char* noiseStdDevStr = (char*) xmlGetProp(node, BAD_CAST NOISE_STD_DEV_ATT);
  if (noiseStdDevStr) {
    SetNoiseStdDev(atof(noiseStdDevStr));
  }

  char* noiseMeanStr = (char*) xmlGetProp(node, BAD_CAST NOISE_MEAN_ATT);
  if (noiseMeanStr) {
    SetNoiseMean(atof(noiseMeanStr));
  }

  char* showImageVolumeOutlineStr = (char*) xmlGetProp(node, BAD_CAST SHOW_IMAGE_VOLUME_OUTLINE_ATT);
  if (showImageVolumeOutlineStr) {
    SetShowImageVolumeOutline((std::string(showImageVolumeOutlineStr) == trueValue));
  }

  char* showReferencePlaneStr = (char*) xmlGetProp(node, BAD_CAST SHOW_REFERENCE_PLANE_ATT);
  if (showReferencePlaneStr) {
    SetShowReferencePlane((std::string(showReferencePlaneStr) == trueValue));
  }

  char* showReferenceGridStr = (char*) xmlGetProp(node, BAD_CAST SHOW_REFERENCE_GRID_ATT);
  if (showReferenceGridStr) {
    SetShowReferenceGrid((std::string(showReferenceGridStr) == trueValue));
  }

  char* referenceGridSpacingStr = (char*) xmlGetProp(node, BAD_CAST REFERENCE_GRID_SPACING_ATT);
  if (referenceGridSpacingStr) {
    SetReferenceGridSpacing(atof(referenceGridSpacingStr));
  }

  char* superimposeFluorescenceImageStr = (char*) xmlGetProp(node, BAD_CAST SUPERIMPOSE_FLUORESCENCE_IMAGE_ATT);
  if (superimposeFluorescenceImageStr) {
    SetSuperimposeFluorescenceImage((std::string(superimposeFluorescenceImageStr) == trueValue));
  }

  char* minIntensityLevelStr = (char*) xmlGetProp(node, BAD_CAST MIN_INTENSITY_LEVEL_ATT);
  if (minIntensityLevelStr) {
    SetMinimumIntensityLevel(atof(minIntensityLevelStr));
  }

  char* maxIntensityLevelStr = (char*) xmlGetProp(node, BAD_CAST MAX_INTENSITY_LEVEL_ATT);
  if (maxIntensityLevelStr) {
    SetMaximumIntensityLevel(atof(maxIntensityLevelStr));
  }
}


void
FluorescenceSimulation
::Sully() {
  if (m_DirtyListener) {
    m_DirtyListener->Sully();
  }
}


void
FluorescenceSimulation
::SetStatusMessage(const std::string& status) {
  if (m_DirtyListener) {
    m_DirtyListener->SetStatusMessage(status);
  }
}


void
FluorescenceSimulation
::SetFluorescenceImageSource(FluorescenceImageSource* source) {
  m_FluoroImageSource = source;
}


FluorescenceImageSource*
FluorescenceSimulation
::GetFluorescenceImageSource() {
  return m_FluoroImageSource;
}


PointSpreadFunction*
FluorescenceSimulation
::GetActivePointSpreadFunction() {
  if (m_ActivePSFIndex >= 0) {
    return m_PSFList->GetPointSpreadFunctionAt(m_ActivePSFIndex);
  }
  return NULL;
}


double
FluorescenceSimulation
::GetFocalPlanePosition() {
  double position = 0.0;
  if (m_UseCustomFocalPlanePositions) {
    position = m_FocalPlanePositions[m_FocalPlaneIndex];
  } else {
    position = static_cast<double>(m_FocalPlaneIndex) * m_FocalPlaneSpacing;
  }

  return position;
}


double
FluorescenceSimulation
::GetMinimumFocalPlanePosition() {
  return 0.0;
}


double
FluorescenceSimulation
::GetMaximumFocalPlanePosition() {
  double position = 0.0;
  if (m_UseCustomFocalPlanePositions) {
    position = m_FocalPlanePositions.back();
  } else {
    position = static_cast<double>(m_NumberOfFocalPlanes-1) * m_FocalPlaneSpacing;
  }

  return position;
}


void
FluorescenceSimulation
::SetActivePSFByName(const std::string& psfName) {
  for (int i = 0; i < m_PSFList->GetSize(); i++) {
    PointSpreadFunction* psf = m_PSFList->GetPointSpreadFunctionAt(i);
    if (psf && psfName == psf->GetName()) {
      SetActivePSFIndex(i);
      return;
    }
  }

  SetActivePSFIndex(-1);
}
