#ifndef _ITK_FLUORESCENCE_OPTIMIZER_H_
#define _ITK_FLUORESCENCE_OPTIMIZER_H_

#define ITK_MANUAL_INSTANTIATION
#include <itkFluorescenceImageSource.h>
#include <itkImage.h>
#include <itkImageToParametricImageSourceMetric.h>
#include <itkMeanSquaresImageToImageMetric.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkNormalizedCorrelationImageToImageMetric.h>
#include <itkPoissonNoiseImageToImageMetric.h>
#undef ITK_MANUAL_INSTANTIATION

#include <FluorescenceOptimizer.h>

class ITKFluorescenceOptimizer : public FluorescenceOptimizer {

 public:

  static const char* GAUSSIAN_NOISE_OBJECTIVE_FUNCTION;

  static const char* POISSON_NOISE_OBJECTIVE_FUNCTION;

  static const char* NORMALIZED_CORRELATION_OBJECTIVE_FUNCTION;


  typedef float PixelType;
  typedef itk::Image<PixelType, 3> FluorescenceImageType;
  typedef itk::FluorescenceImageSource<FluorescenceImageType> SyntheticImageSourceType;

  // Types for optimization.
  typedef itk::ImageToParametricImageSourceMetric<FluorescenceImageType, SyntheticImageSourceType>
    ParametricCostFunctionType;
  typedef itk::NearestNeighborInterpolateImageFunction<FluorescenceImageType, double>
    InterpolatorType;

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

  virtual double GetObjectiveFunctionValue();

 protected:

  SyntheticImageSourceType::Pointer m_FluorescenceImageSource;

  // The cost function used by the optimizer
  ParametricCostFunctionType::Pointer m_CostFunction;

  // The delegate cost function used by m_CostFunction
  ImageToImageCostFunctionType::Pointer  m_ImageToImageCostFunction;

  void SetUpObjectiveFunction();

 protected:
  ITKFluorescenceOptimizer();

};


#endif // _ITK_FLUORESCENCE_OPTIMIZER_H_
