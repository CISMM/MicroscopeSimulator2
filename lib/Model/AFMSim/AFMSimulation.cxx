#include <AFMSimulation.h>


const char* AFMSimulation::PIXEL_SIZE_ATT           = "pixelSize";
const char* AFMSimulation::IMAGE_WIDTH_ATT          = "imageWidth";
const char* AFMSimulation::IMAGE_HEIGHT_ATT         = "imageHeight";
const char* AFMSimulation::CLIP_GROUND_PLANE_ATT    = "clipGroundPlane";
const char* AFMSimulation::DISPLAY_AS_WIREFRAME_ATT = "displayAsWireframe";
const char* AFMSimulation::SURFACE_OPACITY_ATT      = "surfaceOpacity";


AFMSimulation
::AFMSimulation(DirtyListener* dirtyListener) {
  m_DirtyListener = dirtyListener;

  NewSimulation();
}


AFMSimulation
::~AFMSimulation() {

}


void
AFMSimulation
::NewSimulation() {
  m_PixelSize   = 10.0;
  m_ImageWidth  = 300;
  m_ImageHeight = 300;
  m_ClipGroundPlane    = false;
  m_DisplayAsWireframe = false;
  m_SurfaceOpacity     = 1.0;
}


void
AFMSimulation
::GetXMLConfiguration(xmlNodePtr node) {
  char trueStr[] = "true";
  char falseStr[] = "false";
  char buf[128];

  sprintf(buf, "%f", GetPixelSize());
  xmlNewProp(node, BAD_CAST PIXEL_SIZE_ATT, BAD_CAST buf);
  sprintf(buf, "%d", GetImageWidth());
  xmlNewProp(node, BAD_CAST IMAGE_WIDTH_ATT, BAD_CAST buf);
  sprintf(buf, "%d", GetImageHeight());
  xmlNewProp(node, BAD_CAST IMAGE_HEIGHT_ATT, BAD_CAST buf);
  xmlNewProp(node, BAD_CAST CLIP_GROUND_PLANE_ATT, 
             BAD_CAST (GetClipGroundPlane() ? trueStr : falseStr));
  xmlNewProp(node, BAD_CAST DISPLAY_AS_WIREFRAME_ATT,
             BAD_CAST (GetDisplayAsWireframe() ? trueStr : falseStr));
  sprintf(buf, "%f", GetSurfaceOpacity());
  xmlNewProp(node, BAD_CAST SURFACE_OPACITY_ATT, BAD_CAST buf);  
}


void
AFMSimulation
::RestoreFromXML(xmlNodePtr node) {
  std::string trueValue("true");

  char* pixelSizeStr = (char *) xmlGetProp(node, BAD_CAST PIXEL_SIZE_ATT);
  if (pixelSizeStr) {
    double pixelSize = atof(pixelSizeStr);
    SetPixelSize(pixelSize);
  }

  char* imageWidthStr = (char *) xmlGetProp(node, BAD_CAST IMAGE_WIDTH_ATT);
  if (imageWidthStr) {
    unsigned int imageWidth = (unsigned int) atoi(imageWidthStr);
    SetImageWidth(imageWidth);
  }

  char* imageHeightStr = (char *) xmlGetProp(node, BAD_CAST IMAGE_HEIGHT_ATT);
  if (imageHeightStr) {
    unsigned int imageHeight = (unsigned int) atoi(imageHeightStr);
    SetImageHeight(imageHeight);
  }

  char* clipGroundPlaneStr = (char *) xmlGetProp(node, BAD_CAST CLIP_GROUND_PLANE_ATT);
  if (clipGroundPlaneStr) {
    bool clipGroundPlane = (std::string(clipGroundPlaneStr) == trueValue);
    SetClipGroundPlane(clipGroundPlane);
  }

  char* displayAsWireframeStr = (char *) xmlGetProp(node, BAD_CAST DISPLAY_AS_WIREFRAME_ATT);
  if (displayAsWireframeStr) {
    bool displayAsWireframe = (std::string(displayAsWireframeStr) == trueValue);
    SetDisplayAsWireframe(displayAsWireframe);
  }

  char* surfaceOpacityStr = (char *) xmlGetProp(node, BAD_CAST SURFACE_OPACITY_ATT);
  if (surfaceOpacityStr) {
    double surfaceOpacity = atof(surfaceOpacityStr);
    SetSurfaceOpacity(surfaceOpacity);
  }
}


void
AFMSimulation
::Sully() {
  if (m_DirtyListener)
    m_DirtyListener->Sully();
}


void
AFMSimulation
::SetStatusMessage(const std::string& status) {
  if (m_DirtyListener)
    m_DirtyListener->SetStatusMessage(status);
}



