#ifndef _GIBSON_LANNI_WIDEFIELD_POINT_SPREAD_FUNCTION_H_
#define _GIBSON_LANNI_WIDEFIELD_POINT_SPREAD_FUNCTION_H_

#include <vector>

#define ITK_MANUAL_INSTANTIATION
#include <itkGibsonLanniPointSpreadFunctionImageSource.h>
#include <ITKImageToVTKImage.h>
#undef ITK_MANUAL_INSTANTIATION


#include <PointSpreadFunction.h>


class GibsonLanniWidefieldPointSpreadFunction : public PointSpreadFunction {

 public:
  static const std::string PSF_ELEMENT;
  static const std::string POINT_CENTER_ELEMENT;
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
  static const std::string POINT_SOURCE_DEPTH_IN_SPECIMEN_LAYER_ATTRIBUTE;


  GibsonLanniWidefieldPointSpreadFunction();
  virtual ~GibsonLanniWidefieldPointSpreadFunction();

  virtual vtkImageData*       GetOutput();
  virtual vtkAlgorithmOutput* GetOutputPort();

  virtual int         GetNumberOfProperties();
  virtual std::string GetParameterName(int index);
  virtual double      GetParameterValue(int index);
  virtual void        SetParameterValue(int index, double value);

  virtual void GetXMLConfiguration(xmlNodePtr node);
  virtual void RestoreFromXML(xmlNodePtr node);

  typedef float                                     PixelType;
  typedef itk::Image<PixelType, 3>                  ImageType;
  typedef itk::GibsonLanniPointSpreadFunctionImageSource<ImageType> ImageSourceType;
  typedef ImageSourceType::Pointer                  ImageSourceTypePointer;

 protected:
  ImageSourceTypePointer    m_GibsonLanniSource;
  ITKImageToVTKImage<ImageType>* m_ITKToVTKFilter;

  std::vector<std::string> m_ParameterNames;

  void RecenterImage();
};

#endif // _GIBSON_LANNI_WIDEFIELD_POINT_SPREAD_FUNCTION_H_
