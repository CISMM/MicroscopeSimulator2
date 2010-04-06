#ifndef _FLUORESCENCE_OPTIMIZER_H_
#define _FLUORESCENCE_OPTIMIZER_H_


#define ITK_MANUAL_INSTANTIATION
#include <itkAmoebaOptimizer.h>
#include <itkFluorescenceImageSource.h>
#include <itkGradientDescentOptimizer.h>
#include <itkImage.h>
#include <itkImageToParameterizedImageSourceMetric.h>
#include <itkMeanSquaresImageToImageMetric.h>
#include <itkNormalizedCorrelationImageToImageMetric.h>
#include <itkPoissonNoiseImageToImageMetric.h>
#include <itkSingleValuedNonLinearOptimizer.h>
#undef ITK_MANUAL_INSTANTIATION


class FluorescenceSimulation;
class ImageModelObject;
class ModelObjectList;


class FluorescenceOptimizer {

 public:

  typedef enum {
    GAUSSIAN_NOISE_COST_FUNCTION,
    POISSON_NOISE_COST_FUNCTION,
    NORMALIZED_CORRELATION_COST_FUNCTION
  } CostFunction_t;

  typedef enum {
    NELDER_MEAD_OPTIMIZER,
    GRADIENT_DESCENT_OPTIMIZER
  } Optimizer_t;

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
    
  typedef itk::SingleValuedNonLinearOptimizer OptimizerType;
  typedef itk::AmoebaOptimizer               NelderMeadOptimizerType;
  typedef itk::GradientDescentOptimizer      GradientDescentOptimizerType;


  /** Constructor/destructor. */
  FluorescenceOptimizer();
  virtual ~FluorescenceOptimizer();

  void SetFluorescenceSimulation(FluorescenceSimulation* simulation);

  void SetModelObjectList(ModelObjectList* list);
  void SetComparisonImageModelObjectIndex(int index);
  ImageModelObject* GetComparisonImageModelObject();

  void SetCostFunctionToGaussianNoise() {
    m_ImageToImageCostFunctionType = GAUSSIAN_NOISE_COST_FUNCTION;
  }

  void SetCostFunctionToPoissonNoise() {
    m_ImageToImageCostFunctionType = POISSON_NOISE_COST_FUNCTION;
  }

  void SetCostFunctionToNormalizedCorrelation() {
    m_ImageToImageCostFunctionType = NORMALIZED_CORRELATION_COST_FUNCTION;
  }

  void SetOptimizerToNelderMead() {
    m_OptimizerType = NELDER_MEAD_OPTIMIZER;
  }

  void SetOptimizerToGradientDescent() {
    m_OptimizerType = GRADIENT_DESCENT_OPTIMIZER;
  }

  void Optimize();

  void SetParameters(double* params);

 protected:
  FluorescenceSimulation* m_FluoroSim;

  ModelObjectList* m_ModelObjectList;
  ImageModelObject* m_ComparisonImageModelObject;

  SyntheticImageSourceType::Pointer m_FluorescenceImageSource;

  // The cost function used by the optimizer
  ParameterizedCostFunctionType::Pointer m_CostFunction;

  // The delegate cost function used by m_CostFunction
  ImageToImageCostFunctionType::Pointer  m_ImageToImageCostFunction;

  CostFunction_t m_ImageToImageCostFunctionType;
  
  // The optimizer
  OptimizerType::Pointer m_Optimizer;

  Optimizer_t m_OptimizerType;

  void SetUpOptimizer();

};

#endif
