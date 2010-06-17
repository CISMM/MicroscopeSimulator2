#ifndef _FLUORESCENCE_SIMULATION_H_
#define _FLUORESCENCE_SIMULATION_H_

#include <string>

#include <DirtyListener.h>
#include <XMLStorable.h>

class FluorescenceImageSource;
class PointSpreadFunction;
class PointSpreadFunctionList;

class FluorescenceSimulation : public DirtyListener, public XMLStorable {

 public:
  static const char* FOCAL_DEPTH_ATT;
  static const char* FOCAL_DEPTH_MIN_ATT;
  static const char* FOCAL_DEPTH_MAX_ATT;
  static const char* FOCAL_DEPTH_SPACING_ATT;
  static const char* EXPOSURE_TIME_ATT;
  static const char* PIXEL_SIZE_ATT;
  static const char* PSF_NAME_ATT;
  static const char* IMAGE_WIDTH_ATT;
  static const char* IMAGE_HEIGHT_ATT;
  static const char* SHEAR_IN_X_ATT;
  static const char* SHEAR_IN_Y_ATT;
  static const char* ADD_GAUSSIAN_NOISE_ATT;
  static const char* NOISE_STD_DEV_ATT;
  static const char* NOISE_MEAN_ATT;
  static const char* SHOW_IMAGE_VOLUME_OUTLINE_ATT;
  static const char* SHOW_REFERENCE_PLANE_ATT;
  static const char* SHOW_REFERENCE_GRID_ATT;
  static const char* REFERENCE_GRID_SPACING_ATT;
  static const char* SUPERIMPOSE_FLUORESCENCE_IMAGE_ATT;
  static const char* MIN_INTENSITY_LEVEL_ATT;
  static const char* MAX_INTENSITY_LEVEL_ATT;


  FluorescenceSimulation(DirtyListener* dirtyListener);
  virtual ~FluorescenceSimulation();

  void NewSimulation();

  virtual void GetXMLConfiguration(xmlNodePtr node);
  virtual void RestoreFromXML(xmlNodePtr node);

  virtual void Sully();
  virtual void SetStatusMessage(const std::string& status);

  void SetFluorescenceImageSource(FluorescenceImageSource* source);
  FluorescenceImageSource* GetFluorescenceImageSource();

  PointSpreadFunction* GetActivePointSpreadFunction();

  SetGetMacro(FocalPlaneDepth, double);
  SetGetMacro(FocalPlaneDepthMinimum, double);
  SetGetMacro(FocalPlaneDepthMaximum, double);
  SetGetMacro(FocalPlaneDepthSpacing, double);
  SetGetMacro(ActivePSFIndex, int);
  SetGetMacro(Exposure, double);
  SetGetMacro(PixelSize, double);
  SetGetMacro(ImageWidth, unsigned int);
  SetGetMacro(ImageHeight, unsigned int);
  SetGetMacro(ShearInX, double);
  SetGetMacro(ShearInY, double);
  SetGetMacro(AddGaussianNoise, bool);
  SetGetMacro(NoiseStdDev, double);
  SetGetMacro(NoiseMean, double);
  SetGetMacro(ShowImageVolumeOutline, bool);
  SetGetMacro(ShowReferencePlane, bool);
  SetGetMacro(ShowReferenceGrid, bool);
  SetGetMacro(ReferenceGridSpacing, double);
  SetGetMacro(SuperimposeFluorescenceImage, bool);
  SetGetMacro(MinimumIntensityLevel, double);
  SetGetMacro(MaximumIntensityLevel, double);

  void SetPSFList(PointSpreadFunctionList* psfList) {
    m_PSFList = psfList;
  }

  PointSpreadFunctionList* GetPSFList() {
    return m_PSFList;
  }

  SetGetMacro(PSFListSettingsFileName, std::string);

  void SetActivePSFByName(const std::string& psfName);

 protected:
  DirtyListener* m_DirtyListener;

  double m_FocalPlaneDepth;
  double m_FocalPlaneDepthMinimum;
  double m_FocalPlaneDepthMaximum;
  double m_FocalPlaneDepthSpacing;
  int    m_ActivePSFIndex;
  double m_Exposure;
  double m_PixelSize;
  unsigned int m_ImageWidth;
  unsigned int m_ImageHeight;
  double m_ShearInX;
  double m_ShearInY;
  bool   m_AddGaussianNoise;
  double m_NoiseStdDev;
  double m_NoiseMean;
  bool   m_ShowImageVolumeOutline;
  bool   m_ShowReferencePlane;
  bool   m_ShowReferenceGrid;
  double m_ReferenceGridSpacing;
  bool   m_SuperimposeFluorescenceImage;
  double m_MinimumIntensityLevel;
  double m_MaximumIntensityLevel;

  FluorescenceImageSource* m_FluoroImageSource;

  PointSpreadFunctionList* m_PSFList;
  std::string m_PSFListSettingsFileName;

 private:
  FluorescenceSimulation();

};

#endif //  _FLUORESCENCE_SIMULATION_H_
