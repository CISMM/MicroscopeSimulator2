#include <PointSpreadFunctionList.h>
#include <PointSpreadFunction.h>

#include <FluorescenceSimulation.h>
#include <FluorescenceImageSource.h>
#include <ImageModelObject.h>
#include <XMLHelper.h>

const char* FluorescenceSimulation::FOCAL_PLANE_INDEX_ATT = "focalPlaneIndex";
const char* FluorescenceSimulation::FOCAL_PLANE_SPACING_ATT = "focalPlaneSpacing";
const char* FluorescenceSimulation::NUMBER_OF_FOCAL_PLANES_ATT = "numberOfFocalPlanes";
const char* FluorescenceSimulation::USE_CUSTOM_FOCAL_PLANE_POSITIONS_ATT = "useCustomFocalPlanePositions";
const char* FluorescenceSimulation::CUSTOM_FOCAL_PLANE_POSITIONS_ELEM = "FocalPlanes";
const char* FluorescenceSimulation::PLANE_ELEM = "Plane";
const char* FluorescenceSimulation::INDEX_ATT = "index";
const char* FluorescenceSimulation::POSITION_ATT = "position";
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
const char* FluorescenceSimulation::SHOW_REFERENCE_GRID_ATT = "showRefGrid";
const char* FluorescenceSimulation::REFERENCE_GRID_SPACING_ATT = "refGridSpacing";
const char* FluorescenceSimulation::SUPERIMPOSE_SIMULATED_IMAGE_ATT = "superimposeSimulatedImage";
const char* FluorescenceSimulation::SUPERIMPOSE_COMPARISON_IMAGE_ATT = "superimposeComparisonImage";
const char* FluorescenceSimulation::MIN_INTENSITY_LEVEL_ATT = "minimumIntensityLevel";
const char* FluorescenceSimulation::MAX_INTENSITY_LEVEL_ATT = "maximumIntensityLevel";


FluorescenceSimulation
::FluorescenceSimulation(DirtyListener* dirtyListener) {
  m_DirtyListener = dirtyListener;
  m_PSFList = new PointSpreadFunctionList();

  m_FluoroImageSource = NULL;
  m_ComparisonImageModelObject = NULL;

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
  m_FocalPlanePositions.resize(0, 0.0);
  m_FocalPlanePositions.resize(m_NumberOfFocalPlanes, 0.0);
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
  m_ShowReferenceGrid = true;
  m_ReferenceGridSpacing = 1000.0;
  m_SuperimposeSimulatedImage = false;
  m_SuperimposeComparisonImage = false;
  m_MinimumIntensityLevel = 0.0;
  m_MaximumIntensityLevel = 0.0;
  m_ComparisonImageModelObject = NULL;
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

  xmlNodePtr focalPlanePositionsNode = xmlNewChild(node, NULL, BAD_CAST CUSTOM_FOCAL_PLANE_POSITIONS_ELEM, NULL);
  for (unsigned int i = 0; i < GetNumberOfFocalPlanes(); i++) {
    xmlNodePtr positionNode = xmlNewChild(focalPlanePositionsNode, NULL, BAD_CAST PLANE_ELEM, NULL);
    sprintf(buf, "%d", i);
    xmlNewProp(positionNode, BAD_CAST INDEX_ATT, BAD_CAST buf);
    sprintf(buf, "%f", GetCustomFocalPlanePosition(i));
    xmlNewProp(positionNode, BAD_CAST POSITION_ATT, BAD_CAST buf);
  }

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
  sprintf(buf, "%s", GetShowReferenceGrid() ? trueStr : falseStr);
  xmlNewProp(node, BAD_CAST SHOW_REFERENCE_GRID_ATT, BAD_CAST buf);
  sprintf(buf, "%f", GetReferenceGridSpacing());
  xmlNewProp(node, BAD_CAST REFERENCE_GRID_SPACING_ATT, BAD_CAST buf);
  sprintf(buf, "%s", GetSuperimposeSimulatedImage() ? trueStr : falseStr);
  xmlNewProp(node, BAD_CAST SUPERIMPOSE_SIMULATED_IMAGE_ATT, BAD_CAST buf);
  sprintf(buf, "%s", GetSuperimposeComparisonImage() ? trueStr : falseStr);
  xmlNewProp(node, BAD_CAST SUPERIMPOSE_COMPARISON_IMAGE_ATT, BAD_CAST buf);
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

  xmlNodePtr focalPlanePositionsNode = 
    xmlGetFirstElementChildWithName(node, BAD_CAST CUSTOM_FOCAL_PLANE_POSITIONS_ELEM);
  if (focalPlanePositionsNode) {
    unsigned int defaultPlaneIndex = 0;
    xmlNodePtr planeNode = focalPlanePositionsNode->children;
    while (planeNode != NULL) {
      if (planeNode->type == XML_ELEMENT_NODE) {
        std::string nodeName((const char*) planeNode->name);
        if (nodeName != std::string(PLANE_ELEM)) {
          planeNode = planeNode->next;
          continue;
        }

        char* positionStr = (char*) xmlGetProp(planeNode, BAD_CAST POSITION_ATT);
        double position = 0.0;
        if (positionStr)
          position = atof(positionStr);

        unsigned int planeIndex = 0;
        char* indexStr = (char*) xmlGetProp(planeNode, BAD_CAST INDEX_ATT);
        if (indexStr)
          planeIndex = (unsigned int) atoi(indexStr);
        else
          planeIndex = defaultPlaneIndex;

        SetCustomFocalPlanePosition(planeIndex, position);
        defaultPlaneIndex++;
      }

      planeNode = planeNode->next;
    }
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

  char* showReferenceGridStr = (char*) xmlGetProp(node, BAD_CAST SHOW_REFERENCE_GRID_ATT);
  if (showReferenceGridStr) {
    SetShowReferenceGrid((std::string(showReferenceGridStr) == trueValue));
  }

  char* referenceGridSpacingStr = (char*) xmlGetProp(node, BAD_CAST REFERENCE_GRID_SPACING_ATT);
  if (referenceGridSpacingStr) {
    SetReferenceGridSpacing(atof(referenceGridSpacingStr));
  }

  char* superimposeSimulatedImageStr = (char*) xmlGetProp(node, BAD_CAST SUPERIMPOSE_SIMULATED_IMAGE_ATT);
  if (superimposeSimulatedImageStr) {
    SetSuperimposeSimulatedImage((std::string(superimposeSimulatedImageStr) == trueValue));
  }

  char* superimposeComparisonImageStr = (char*) xmlGetProp(node, BAD_CAST SUPERIMPOSE_COMPARISON_IMAGE_ATT);
  if (superimposeComparisonImageStr) {
    SetSuperimposeComparisonImage((std::string(superimposeComparisonImageStr) == trueValue));
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


void
FluorescenceSimulation
::SetNumberOfFocalPlanes(unsigned int n) {
  m_NumberOfFocalPlanes = n;

  m_FocalPlanePositions.resize(n, 0.0);

  Sully();
}


unsigned int
FluorescenceSimulation
::GetNumberOfFocalPlanes() {
  return m_NumberOfFocalPlanes;
}


void
FluorescenceSimulation
::SetCustomFocalPlanePosition(unsigned int i, double position) {
  if (i < m_FocalPlanePositions.size()) {
    m_FocalPlanePositions[i] = position;
    Sully();
  }
}


double
FluorescenceSimulation
::GetCustomFocalPlanePosition(unsigned int i) {
  if (i < m_FocalPlanePositions.size()) {
    return m_FocalPlanePositions[i];
  }

  return 0.0;
}


double
FluorescenceSimulation
::GetFocalPlanePosition() {
  return GetFocalPlanePosition(m_FocalPlaneIndex);
}


double
FluorescenceSimulation
::GetFocalPlanePosition(unsigned int i) {
  double position = 0.0;
  if (m_UseCustomFocalPlanePositions) {
    position = m_FocalPlanePositions[i];
  } else {
    position = static_cast<double>(i) * m_FocalPlaneSpacing;
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


void
FluorescenceSimulation
::SetComparisonImageModelObject(ImageModelObject* object) {
  m_ComparisonImageModelObject = dynamic_cast<ImageModelObject*>(object);

  Sully();
}


ImageModelObject*
FluorescenceSimulation
::GetComparisonImageModelObject() {
  return m_ComparisonImageModelObject;
}
