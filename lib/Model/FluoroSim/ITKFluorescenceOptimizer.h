#ifndef _ITK_FLUORESCENCE_OPTIMIZER_H_
#define _ITK_FLUORESCENCE_OPTIMIZER_H_

#define ITK_MANUAL_INSTANTIATION
#include <itkFluorescenceImageSource.h>
#include <itkImage.h>
#include <itkImageToParameterizedImageSourceMetric.h>
#include <itkMeanSquaresImageToImageMetric.h>
#include <itkNormalizedCorrelationImageToImageMetric.h>
#include <itkPoissonNoiseImageToImageMetric.h>
#undef ITK_MANUAL_INSTANTIATION

#include <FluorescenceOptimizer.h>

class ITKFluorescenceOptimizer : public FluorescenceOptimizer {

 public:

  typedef enum {
    GAUSSIAN_NOISE_COST_FUNCTION,
    POISSON_NOISE_COST_FUNCTION,
    NORMALIZED_CORRELATION_COST_FUNCTION
  } CostFunction_t;


  typedef float PixelType;
  typedef itk::Image<PixelType, 3> FluorescenceImageType;
  typedef itk::FluorescenceImageSource<FluorescenceImageType> SyntheticImageSourceType;

  // Types for optimization.
  typedef itk::ImageToParameterizedImageSourceMetric<FluorescenceImageType, SyntheticImageSourceType>
    ParameterizedCostFunctionType;

  typedef itk::ImageToImageMetric<FluorescenceImageType, FluorescenceImageType>
    ImageToImageCostFunctionType;
  typedef itk::MeanSquaresImageToImageMetric<FluorescenceImageType, FluorescenceImageType>
    GaussianNoiseCostFunctionType;
  typedef itk::PoissonNoiseImageToImageMetric<FluorescenceImageType, FluorescenceImageType>
    PoissonNoiseCostFunctionType;
  typedef itk::NormalizedCorrelationImageToImageMetric<FluorescenceImageType, FluorescenceImageType>
    NormalizedCorrelationCostFunctionType;

  /** Constructor/destructor. */
  ITKFluorescenceOptimizer(DirtyListener* listener);
  virtual ~ITKFluorescenceOptimizer();

  void SetCostFunctionToGaussianNoise() {
    m_ImageToImageCostFunctionType = GAUSSIAN_NOISE_COST_FUNCTION;
  }

  void SetCostFunctionToPoissonNoise() {
    m_ImageToImageCostFunctionType = POISSON_NOISE_COST_FUNCTION;
  }

  void SetCostFunctionToNormalizedCorrelation() {
    m_ImageToImageCostFunctionType = NORMALIZED_CORRELATION_COST_FUNCTION;
  }

 protected:
  
  SyntheticImageSourceType::Pointer m_FluorescenceImageSource;

  // The cost function used by the optimizer
  ParameterizedCostFunctionType::Pointer m_CostFunction;

  // The delegate cost function used by m_CostFunction
  ImageToImageCostFunctionType::Pointer  m_ImageToImageCostFunction;

  CostFunction_t m_ImageToImageCostFunctionType;

  void SetUpCostFunction();

 protected:
  ITKFluorescenceOptimizer();

};


#endif // _ITK_FLUORESCENCE_OPTIMIZER_H_
