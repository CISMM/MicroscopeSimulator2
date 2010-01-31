#ifndef _GAUSSIAN_POINT_SPREAD_FUNCTION_H_
#define _GAUSSIAN_POINT_SPREAD_FUNCTION_H_

#include <vector>

#include <itkGaussianImageSource.h>
#include <ITKImageToVTKImage.h>

#include <PointSpreadFunction.h>


class GaussianPointSpreadFunction : public PointSpreadFunction {
 public:
  static const std::string SIZE_ELEMENT;
  static const std::string SPACING_ELEMENT;
  static const std::string MEAN_ELEMENT;
  static const std::string SIGMA_ELEMENT;
  static const std::string X_ATTRIBUTE;
  static const std::string Y_ATTRIBUTE;
  static const std::string Z_ATTRIBUTE;

  GaussianPointSpreadFunction();
  virtual ~GaussianPointSpreadFunction();

  virtual vtkImageData* GetOutput();
  virtual vtkAlgorithmOutput* GetOutputPort();

  virtual int GetNumberOfProperties();
  virtual std::string GetParameterName(int index);
  virtual double      GetParameterValue(int index);
  virtual void SetParameterValue(int index, double value);

  virtual void GetXMLConfiguration(xmlNodePtr node);
  virtual void RestoreFromXML(xmlNodePtr node);

  typedef float                               PixelType;
  typedef itk::Image<PixelType, 3>            ImageType;
  typedef itk::GaussianImageSource<ImageType> ImageSourceType;
  typedef ImageSourceType::Pointer            ImageSourceTypePointer;

 protected:
  ImageSourceTypePointer         m_GaussianSource;
  ITKImageToVTKImage<ImageType>* m_ITKToVTKFilter;

  std::vector<std::string> m_ParameterNames;

};


#endif // _GAUSSIAN_POINT_SPREAD_FUNCTION_H_
