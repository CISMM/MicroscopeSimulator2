#ifndef _MODIFIED_GIBSON_LANNI_WIDEFIELD_POINT_SPREAD_FUNCTION_H_
#define _MODIFIED_GIBSON_LANNI_WIDEFIELD_POINT_SPREAD_FUNCTION_H_

#include <vector>

#define ITK_MANUAL_INSTANTIATION
#include <itkModifiedGibsonLanniPointSpreadFunctionImageSource.h>
#include <ITKImageToVTKImage.h>
#undef ITK_MANUAL_INSTANTIATION


#include <PointSpreadFunction.h>


class ModifiedGibsonLanniWidefieldPointSpreadFunction : public PointSpreadFunction {

 public:
  static const std::string PSF_ELEMENT;
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
  static const std::string GAUSSIAN_CENTER_X_ATTRIBUTE;
  static const std::string GAUSSIAN_CENTER_Y_ATTRIBUTE;
  static const std::string GAUSSIAN_CENTER_Z_ATTRIBUTE;
  static const std::string GAUSSIAN_SIGMA_X_ATTRIBUTE;
  static const std::string GAUSSIAN_SIGMA_Y_ATTRIBUTE;
  static const std::string GAUSSIAN_SIGMA_Z_ATTRIBUTE;
  static const std::string GAUSSIAN_INTENSITY_SCALE_ATTRIBUTE;


  ModifiedGibsonLanniWidefieldPointSpreadFunction();
  virtual ~ModifiedGibsonLanniWidefieldPointSpreadFunction();

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
  typedef itk::ModifiedGibsonLanniPointSpreadFunctionImageSource<ImageType>
    ImageSourceType;
  typedef ImageSourceType::Pointer
    ImageSourceTypePointer;

 protected:
  ImageSourceTypePointer    m_ModifiedGibsonLanniSource;
  ITKImageToVTKImage<ImageType>* m_ITKToVTKFilter;

  std::vector<std::string> m_ParameterNames;

  void RecenterImage();
};

#endif // _MODIFIED_GIBSON_LANNI_WIDEFIELD_POINT_SPREAD_FUNCTION_H_
