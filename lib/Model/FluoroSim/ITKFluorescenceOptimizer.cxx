#include <itkFluorescenceImageSource.cxx>
#include <itkImage.txx>
#include <itkImageToParameterizedImageSourceMetric.cxx>
#include <itkMeanSquaresImageToImageMetric.txx>
#include <itkNormalizedCorrelationImageToImageMetric.txx>
#include <itkPoissonNoiseImageToImageMetric.cxx>

// WARNING: Always include the header file for this class AFTER
// including the ITK headers. Otherwise, the ITK headers will be included
// without including the implementation files, and you will have many linker
// errors.
#include <ITKFluorescenceOptimizer.h>


ITKFluorescenceOptimizer
::ITKFluorescenceOptimizer(DirtyListener* listener) 
  : FluorescenceOptimizer(listener) {
  m_FluorescenceImageSource = SyntheticImageSourceType::New();

  m_CostFunction = ParameterizedCostFunctionType::New();

  m_ImageToImageCostFunction = GaussianNoiseCostFunctionType::New();

  AddObjectiveFunctionName(std::string("Gaussian Noise Maximum Likelihood"));
  AddObjectiveFunctionName(std::string("Poisson Noise Maximum Likelihood"));
  AddObjectiveFunctionName(std::string("Normalized Cross Correlation"));
}


ITKFluorescenceOptimizer
::~ITKFluorescenceOptimizer() {

}


void
ITKFluorescenceOptimizer
::SetUpCostFunction() {
  // Set up the cost function
  if (m_ImageToImageCostFunctionType == GAUSSIAN_NOISE_COST_FUNCTION) {
    m_ImageToImageCostFunction = GaussianNoiseCostFunctionType::New();
    std::cout << "Using Gaussian noise cost function, ";
  } else if (m_ImageToImageCostFunctionType == POISSON_NOISE_COST_FUNCTION) {
    m_ImageToImageCostFunction = PoissonNoiseCostFunctionType::New();
    std::cout << "Using Poisson noise cost function, ";
  } else if (m_ImageToImageCostFunctionType == NORMALIZED_CORRELATION_COST_FUNCTION) {
    m_ImageToImageCostFunction = NormalizedCorrelationCostFunctionType::New();
    std::cout << "Using normalized correlation cost function, ";
  }
}
