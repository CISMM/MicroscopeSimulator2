#include <FluorescenceSimulation.h>


const char* FluorescenceSimulation::FOCAL_DEPTH_ATT = "focalDepth";
const char* FluorescenceSimulation::FOCAL_DEPTH_MIN_ATT = "focalDepthMin";
const char* FluorescenceSimulation::FOCAL_DEPTH_MAX_ATT = "focalDepthMax";
const char* FluorescenceSimulation::FOCAL_DEPTH_SPACING_ATT = "focalDepthSpacing";
const char* FluorescenceSimulation::EXPOSURE_TIME_ATT = "exposureTime";
const char* FluorescenceSimulation::PIXEL_SIZE_ATT = "pixelSize";
const char* FluorescenceSimulation::IMAGE_WIDTH_ATT = "imageWidth";
const char* FluorescenceSimulation::IMAGE_HEIGHT_ATT = "imageHeight";
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

  NewSimulation();
}


FluorescenceSimulation
::~FluorescenceSimulation() {
}


void
FluorescenceSimulation
::NewSimulation() {
  m_FocalPlaneDepth = 0.0;
  m_FocalPlaneDepthMinimum = -5000.0;
  m_FocalPlaneDepthMaximum =  5000.0;
  m_FocalPlaneDepthSpacing =   200.0;
  m_Exposure    = 1.0;
  m_PixelSize   = 65.0;
  m_ImageWidth  = 200;
  m_ImageHeight = 200;
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
  sprintf(buf, "%f", GetFocalPlaneDepth());
  xmlNewProp(node, BAD_CAST FOCAL_DEPTH_ATT, BAD_CAST buf);
  sprintf(buf, "%f", GetFocalPlaneDepthMinimum());
  xmlNewProp(node, BAD_CAST FOCAL_DEPTH_MIN_ATT, BAD_CAST buf);
  sprintf(buf, "%f", GetFocalPlaneDepthMaximum());
  xmlNewProp(node, BAD_CAST FOCAL_DEPTH_MAX_ATT, BAD_CAST buf);
  sprintf(buf, "%f", GetFocalPlaneDepthSpacing());
  xmlNewProp(node, BAD_CAST FOCAL_DEPTH_SPACING_ATT, BAD_CAST buf);
  sprintf(buf, "%f", GetExposure());
  xmlNewProp(node, BAD_CAST EXPOSURE_TIME_ATT, BAD_CAST buf);
  sprintf(buf, "%f", GetPixelSize());
  xmlNewProp(node, BAD_CAST PIXEL_SIZE_ATT, BAD_CAST buf);
  sprintf(buf, "%d", GetImageWidth());
  xmlNewProp(node, BAD_CAST IMAGE_WIDTH_ATT, BAD_CAST buf);
  sprintf(buf, "%d", GetImageHeight());
  xmlNewProp(node, BAD_CAST IMAGE_HEIGHT_ATT, BAD_CAST buf);
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
