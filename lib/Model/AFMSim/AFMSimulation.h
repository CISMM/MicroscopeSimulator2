#ifndef _AFM_SIMULATION_H_
#define _AFM_SIMULATION_H_

#include <string>

#include <DirtyListener.h>
#include <XMLStorable.h>

class AFMSimulation : public DirtyListener, public XMLStorable {

 public:
  static const char* PIXEL_SIZE_ATT;
  static const char* IMAGE_WIDTH_ATT;
  static const char* IMAGE_HEIGHT_ATT;
  static const char* CLIP_GROUND_PLANE_ATT;
  static const char* DISPLAY_AS_WIREFRAME_ATT;
  static const char* SURFACE_OPACITY_ATT;


  AFMSimulation(DirtyListener* dirtyListener);
  virtual ~AFMSimulation();

  void NewSimulation();

  virtual void GetXMLConfiguration(xmlNodePtr node);
  virtual void RestoreFromXML(xmlNodePtr node);

  virtual void Sully();
  virtual void SetStatusMessage(const std::string& status);

  SetGetMacro(PixelSize, double);
  SetGetMacro(ImageWidth, unsigned int);
  SetGetMacro(ImageHeight, unsigned int);
  SetGetMacro(ClipGroundPlane, bool);
  SetGetMacro(DisplayAsWireframe, bool);
  SetGetMacro(SurfaceOpacity, double);

 protected:
  DirtyListener* m_DirtyListener;

  double       m_PixelSize;
  unsigned int m_ImageWidth;
  unsigned int m_ImageHeight;
  bool         m_ClipGroundPlane;
  bool         m_DisplayAsWireframe;
  double       m_SurfaceOpacity;

 private:
  AFMSimulation() {};

};

#endif // _AFM_SIMULATION_H_
