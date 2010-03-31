#ifndef _GAUSSIAN_POINT_SPREAD_FUNCTION_H_
#define _GAUSSIAN_POINT_SPREAD_FUNCTION_H_

#include <vector>

#define ITK_MANUAL_INSTANTIATION
#include <itkGaussianImageSource.h>
#include <ITKImageToVTKImage.h>
#undef ITK_MANUAL_INSTANTIATION

#include <PointSpreadFunction.h>


class GaussianPointSpreadFunction : public PointSpreadFunction {
 public:
  static const std::string MEAN_ELEMENT;
  static const std::string SIGMA_ELEMENT;


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

  void RecenterImage();

};


#endif // _GAUSSIAN_POINT_SPREAD_FUNCTION_H_
