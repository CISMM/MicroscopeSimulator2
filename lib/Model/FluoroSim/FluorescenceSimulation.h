#ifndef _FLUORESCENCE_SIMULATION_H_
#define _FLUORESCENCE_SIMULATION_H_

#include <string>
#include <vector>

#include <DirtyListener.h>
#include <XMLStorable.h>

class FluorescenceImageSource;
class ImageModelObject;
class PointSpreadFunction;
class PointSpreadFunctionList;

class FluorescenceSimulation : public DirtyListener, public XMLStorable {

 public:
  static const char* FOCAL_PLANE_INDEX_ATT;
  static const char* FOCAL_PLANE_SPACING_ATT;
  static const char* NUMBER_OF_FOCAL_PLANES_ATT;
  static const char* USE_CUSTOM_FOCAL_PLANE_POSITIONS_ATT;
  static const char* CUSTOM_FOCAL_PLANE_POSITIONS_ELEM;
  static const char* PLANE_ELEM;
  static const char* INDEX_ATT;
  static const char* POSITION_ATT;
  static const char* GAIN_ATT;
  static const char* OFFSET_ATT;
  static const char* MAXIMUM_VOXEL_INTENSITY_ATT;
  static const char* PIXEL_SIZE_ATT;
  static const char* PSF_NAME_ATT;
  static const char* IMAGE_WIDTH_ATT;
  static const char* IMAGE_HEIGHT_ATT;
  static const char* SHEAR_IN_X_ATT;
  static const char* SHEAR_IN_Y_ATT;
  static const char* ADD_GAUSSIAN_NOISE_ATT;
  static const char* NOISE_STD_DEV_ATT;
  static const char* SHOW_IMAGE_VOLUME_OUTLINE_ATT;
  static const char* SHOW_REFERENCE_GRID_ATT;
  static const char* REFERENCE_GRID_SPACING_ATT;
  static const char* SUPERIMPOSE_SIMULATED_IMAGE_ATT;
  static const char* SUPERIMPOSE_COMPARISON_IMAGE_ATT;
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

  SetGetMacro(FocalPlaneIndex, unsigned int);
  SetGetMacro(FocalPlaneSpacing, double);

  void SetNumberOfFocalPlanes(unsigned int n);
  unsigned int  GetNumberOfFocalPlanes();

  SetGetMacro(UseCustomFocalPlanePositions, bool);
  SetGetMacro(ActivePSFIndex, int);
  SetGetMacro(Gain, double);
  SetGetMacro(Offset, double);
  SetGetMacro(MaximumVoxelIntensity, double);
  SetGetMacro(PixelSize, double);
  SetGetMacro(ImageWidth, unsigned int);
  SetGetMacro(ImageHeight, unsigned int);
  SetGetMacro(ShearInX, double);
  SetGetMacro(ShearInY, double);
  SetGetMacro(AddGaussianNoise, bool);
  SetGetMacro(NoiseStdDev, double);
  SetGetMacro(ShowImageVolumeOutline, bool);
  SetGetMacro(ShowReferenceGrid, bool);
  SetGetMacro(ReferenceGridSpacing, double);
  SetGetMacro(SuperimposeSimulatedImage, bool);
  SetGetMacro(SuperimposeComparisonImage, bool);
  SetGetMacro(MinimumIntensityLevel, double);
  SetGetMacro(MaximumIntensityLevel, double);

  void   SetCustomFocalPlanePosition(unsigned int i, double position);
  double GetCustomFocalPlanePosition(unsigned int i);

  double GetFocalPlanePosition();
  double GetFocalPlanePosition(unsigned int i);
  double GetMinimumFocalPlanePosition();
  double GetMaximumFocalPlanePosition();

  void SetPSFList(PointSpreadFunctionList* psfList) {
    m_PSFList = psfList;
  }

  PointSpreadFunctionList* GetPSFList() {
    return m_PSFList;
  }

  SetGetMacro(PSFListSettingsFileName, std::string);

  void SetActivePSFByName(const std::string& psfName);

  void SetComparisonImageModelObject(ImageModelObject* object);
  ImageModelObject* GetComparisonImageModelObject();

 protected:
  DirtyListener* m_DirtyListener;

  unsigned int m_FocalPlaneIndex;
  double       m_FocalPlaneSpacing;
  unsigned int m_NumberOfFocalPlanes;
  bool         m_UseCustomFocalPlanePositions;
  int          m_ActivePSFIndex;
  double       m_Gain;
  double       m_Offset;
  double       m_MaximumVoxelIntensity;
  double       m_PixelSize;
  unsigned int m_ImageWidth;
  unsigned int m_ImageHeight;
  double       m_ShearInX;
  double       m_ShearInY;
  bool         m_AddGaussianNoise;
  double       m_NoiseStdDev;
  bool         m_ShowImageVolumeOutline;
  bool         m_ShowReferenceGrid;
  double       m_ReferenceGridSpacing;
  bool         m_SuperimposeSimulatedImage;
  bool         m_SuperimposeComparisonImage;
  double       m_MinimumIntensityLevel;
  double       m_MaximumIntensityLevel;

  FluorescenceImageSource* m_FluoroImageSource;

  PointSpreadFunctionList* m_PSFList;
  std::string m_PSFListSettingsFileName;

  std::vector<double> m_FocalPlanePositions;

  ImageModelObject* m_ComparisonImageModelObject;

 private:
  FluorescenceSimulation();

};

#endif //  _FLUORESCENCE_SIMULATION_H_
