#ifndef _WIDEFIELD_POINT_SPREAD_FUNCTION_H_
#define _WIDEFIELD_POINT_SPREAD_FUNCTION_H_

#include <vector>

#include <itkGibsonLanniPSFImageSource.h>
#include <ITKImageToVTKImage.h>

#include <PointSpreadFunction.h>


class WidefieldPointSpreadFunction : public PointSpreadFunction {

 public:
  static const std::string SIZE_ELEMENT;
  static const std::string SPACING_ELEMENT;
  static const std::string ORIGIN_ELEMENT;
  static const std::string POINT_CENTER_ELEMENT;
  static const std::string CCD_BORDER_WIDTH_ELEMENT;
  static const std::string EMISSION_WAVELENGTH_ATTRIBUTE;
  static const std::string NUMERICAL_APERTURE_ATTRIBUTE;
  static const std::string MAGNIFICATION_ATTRIBUTE;
  static const std::string DESIGN_COVER_SLIP_REFRACTIVE_INDEX_ATTRIBUTE;
  static const std::string ACTUAL_COVER_SLIP_REFRACTIVE_INDEX_ATTRIBUTE;
  static const std::string DESIGN_COVER_SLIP_THICKNESS_ATTRIBUTE;
  static const std::string ACTUAL_COVER_SLIP_THICKNESS_ATTRIBUTE;
  static const std::string DESIGN_IMMERSION_OIL_REFRACTIVE_INDEX_ATTRIBUTE;
  static const std::string ACTUAL_IMMERSION_OIL_REFRACTIVE_INDEX_ATTRIBUTE;
  static const std::string DESIGN_IMMERSION_OIL_THICKNESS_ATTRIBUTE;
  static const std::string DESIGN_SPECIMEN_LAYER_REFRACTIVE_INDEX_ATTRIBUTE;
  static const std::string ACTUAL_SPECIMEN_LAYER_REFRACTIVE_INDEX_ATTRIBUTE;
  static const std::string ACTUAL_POINT_SOURCE_DEPTH_IN_SPECIMEN_LAYER_ATTRIBUTE;
  static const std::string DESIGN_DISTANCE_FROM_BACK_FOCAL_PLANE_TO_DETECTOR_ATTRIBUTE;
  static const std::string ACTUAL_DISTANCE_FROM_BACK_FOCAL_PLANE_TO_DETECTOR_ATTRIBUTE;
  static const std::string X_ATTRIBUTE;
  static const std::string Y_ATTRIBUTE;
  static const std::string Z_ATTRIBUTE;


  WidefieldPointSpreadFunction();
  virtual ~WidefieldPointSpreadFunction();

  virtual vtkImageData* GetOutput();
  virtual vtkAlgorithmOutput* GetOutputPort();

  virtual int GetNumberOfProperties();
  virtual std::string GetParameterName(int index);
  virtual double      GetParameterValue(int index);
  virtual void SetParameterValue(int index, double value);

  virtual void GetXMLConfiguration(xmlNodePtr node);
  virtual void RestoreFromXML(xmlNodePtr node);

  typedef float                                     PixelType;
  typedef itk::Image<PixelType, 3>                  ImageType;
  typedef itk::GibsonLanniPSFImageSource<ImageType> ImageSourceType;
  typedef ImageSourceType::Pointer                  ImageSourceTypePointer;

 protected:
  ImageSourceTypePointer    m_GibsonLanniSource;
  ITKImageToVTKImage<ImageType>* m_ITKToVTKFilter;

  std::vector<std::string> m_ParameterNames;

  void RecenterImage();
};

#endif // _WIDEFIELD_POINT_SPREAD_FUNCTION_H_
