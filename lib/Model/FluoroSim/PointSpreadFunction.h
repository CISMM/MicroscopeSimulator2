#ifndef _POINT_SPREAD_FUNCTION_H_
#define _POINT_SPREAD_FUNCTION_H_

#include <string>

#include <XMLStorable.h>

#define ITK_MANUAL_INSTANTIATION
#include <itkRecursiveGaussianImageFilter.h>
#include <ITKImageToVTKImage.h>
#undef ITK_MANUAL_INSTANTIATION

#include <vtkSmartPointer.h>

class vtkAlgorithmOutput;
class vtkImageData;
class vtkImageAppendComponents;


class PointSpreadFunction : public XMLStorable {
 public:
  static const std::string NAME_ATTRIBUTE;
  static const std::string SIZE_ELEMENT;
  static const std::string SPACING_ELEMENT;
  static const std::string X_ATTRIBUTE;
  static const std::string Y_ATTRIBUTE;
  static const std::string Z_ATTRIBUTE;

  typedef float                               PixelType;
  typedef itk::Image<PixelType, 3>            ImageType;
  typedef itk::RecursiveGaussianImageFilter<ImageType, ImageType>
    DerivativeFilterType;

  PointSpreadFunction();
  virtual ~PointSpreadFunction();

  void SetName(const std::string& name);
  std::string& GetName();

  virtual vtkImageData* GetOutput() = 0;
  virtual vtkAlgorithmOutput* GetOutputPort() = 0;

  virtual vtkImageData* GetGradientOutput();
  virtual vtkAlgorithmOutput* GetGradientOutputPort();
  void SetSigma(double sigma);
  double GetSigma();

  virtual int GetNumberOfProperties() = 0;
  virtual std::string GetParameterName(int index) = 0;
  virtual double      GetParameterValue(int index) = 0;
  virtual void SetParameterValue(int index, double value) = 0;

  virtual void RestoreFromXML(xmlNodePtr node) = 0;

 protected:
  std::string m_Name;

  double                        m_Sigma;
  DerivativeFilterType::Pointer m_DerivativeX;
  DerivativeFilterType::Pointer m_DerivativeY;
  DerivativeFilterType::Pointer m_DerivativeZ;

  ITKImageToVTKImage<ImageType>* m_VTKDerivativeX;
  ITKImageToVTKImage<ImageType>* m_VTKDerivativeY;
  ITKImageToVTKImage<ImageType>* m_VTKDerivativeZ;

  vtkSmartPointer<vtkImageAppendComponents> m_VTKGradient;

  void UpdateGradientImage();
};


#endif // _POINT_SPREAD_FUNCTION_H_
