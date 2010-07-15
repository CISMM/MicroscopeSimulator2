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


const char* ITKFluorescenceOptimizer::GAUSSIAN_NOISE_OBJECTIVE_FUNCTION =
  "Gaussian Noise Maximum Likelihood";

const char* ITKFluorescenceOptimizer::POISSON_NOISE_OBJECTIVE_FUNCTION =
  "Poisson Noise Maximum Likelihood";

const char* ITKFluorescenceOptimizer::NORMALIZED_CORRELATION_OBJECTIVE_FUNCTION =
  "Normalized Cross Correlation";


ITKFluorescenceOptimizer
::ITKFluorescenceOptimizer(DirtyListener* listener) 
  : FluorescenceOptimizer(listener) {
  m_FluorescenceImageSource = SyntheticImageSourceType::New();

  m_CostFunction = ParameterizedCostFunctionType::New();

  m_ImageToImageCostFunction = GaussianNoiseCostFunctionType::New();

  AddObjectiveFunctionName(std::string(GAUSSIAN_NOISE_OBJECTIVE_FUNCTION));
  AddObjectiveFunctionName(std::string(POISSON_NOISE_OBJECTIVE_FUNCTION));
  AddObjectiveFunctionName(std::string(NORMALIZED_CORRELATION_OBJECTIVE_FUNCTION));

  m_ActiveObjectiveFunctionName = GAUSSIAN_NOISE_OBJECTIVE_FUNCTION;  
}


ITKFluorescenceOptimizer
::~ITKFluorescenceOptimizer() {

}


void
ITKFluorescenceOptimizer
::SetUpObjectiveFunction() {
  // Set up the cost function
  if (m_ActiveObjectiveFunctionName == GAUSSIAN_NOISE_OBJECTIVE_FUNCTION) {
    m_ImageToImageCostFunction = GaussianNoiseCostFunctionType::New();
    std::cout << "Using Gaussian noise cost function, ";
  } else if (m_ActiveObjectiveFunctionName == POISSON_NOISE_OBJECTIVE_FUNCTION) {
    m_ImageToImageCostFunction = PoissonNoiseCostFunctionType::New();
    std::cout << "Using Poisson noise cost function, ";
  } else if (m_ActiveObjectiveFunctionName == NORMALIZED_CORRELATION_OBJECTIVE_FUNCTION) {
    m_ImageToImageCostFunction = NormalizedCorrelationCostFunctionType::New();
    std::cout << "Using normalized correlation cost function, ";
  }
}
