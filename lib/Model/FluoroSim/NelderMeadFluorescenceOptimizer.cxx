#include <itkAmoebaOptimizer.h>
#include <itkFluorescenceImageSource.h>

#include <NelderMeadFluorescenceOptimizer.h>

#include <FluorescenceSimulation.h>
#include <ImageModelObject.h>


const char* NelderMeadFluorescenceOptimizer::OPTIMIZER_ELEM = 
  "NelderMeadFluorescenceOptimizer";

const char* NelderMeadFluorescenceOptimizer::MAXIMUM_ITERATIONS_PARAM = 
  "Maximum Iterations";

const char* NelderMeadFluorescenceOptimizer::PARAMETERS_CONVERGENCE_TOLERANCE_PARAM =
  "Parameters Convergence Tolerance";


NelderMeadFluorescenceOptimizer
::NelderMeadFluorescenceOptimizer(DirtyListener* listener) 
  : ITKFluorescenceOptimizer(listener) {
  Variant maxIterations;
  maxIterations.iValue = 100;
  AddOptimizerParameter(std::string(MAXIMUM_ITERATIONS_PARAM), INT_TYPE, maxIterations);

  Variant  parametersConvergenceTolerance;
  parametersConvergenceTolerance.dValue = 1e-8;
  AddOptimizerParameter(std::string(PARAMETERS_CONVERGENCE_TOLERANCE_PARAM), DOUBLE_TYPE, parametersConvergenceTolerance);
}


NelderMeadFluorescenceOptimizer
::~NelderMeadFluorescenceOptimizer() {
}


void
NelderMeadFluorescenceOptimizer
::Optimize() {
  SetUpObjectiveFunction();

  if (!m_ImageToImageCostFunction) {
    std::cout << "ERROR: No image to image cost function set." << std::endl;
    return;
  }
  
  NelderMeadOptimizerType::Pointer optimizer = NelderMeadOptimizerType::New();

  // Set the optimizer parameters
  int maxIterations = GetOptimizerParameterValue(MAXIMUM_ITERATIONS_PARAM).iValue;
  optimizer->SetMaximumNumberOfIterations(maxIterations);

  double parametersConvergenceTolerance = GetOptimizerParameterValue(PARAMETERS_CONVERGENCE_TOLERANCE_PARAM).dValue;
  optimizer->SetParametersConvergenceTolerance(parametersConvergenceTolerance);

  // Make sure to set the fluorescence image source and moving image.
  m_FluorescenceImageSource->
    SetFluorescenceImageSource(m_FluoroSim->GetFluorescenceImageSource());

  if (m_ComparisonImageModelObject)
    m_CostFunction->SetFixedImage(m_ComparisonImageModelObject->GetITKImage());
  m_CostFunction->SetMovingImageSource(m_FluorescenceImageSource);
  
  typedef ParameterizedCostFunctionType::ParametersMaskType
    ParametersMaskType;
  ParametersMaskType* mask = m_CostFunction->GetParametersMask();
  
  // Pluck out the active parameters
  typedef ParameterizedCostFunctionType::ParametersType ParametersType;
  ParametersType activeParameters
    = ParametersType(m_CostFunction->GetNumberOfParameters());
  int activeIndex = 0;
  for (unsigned int i = 0; i < mask->Size(); i++) {
    if (mask->GetElement(i)) {
      // TODO - The right hand side is slower than it has to be. Make it faster
      activeParameters[activeIndex++] = m_FluorescenceImageSource->GetParameters()[i];
    }
  }
  
  std::cout << "Starting parameters: " << activeParameters << std::endl;
  
  // Connect to the cost function, set the initial parameters, and optimize.
  m_ImageToImageCostFunction->SetFixedImageRegion
    (m_FluorescenceImageSource->GetOutput()->GetLargestPossibleRegion());
  
  m_CostFunction->SetImageToImageMetric(m_ImageToImageCostFunction);
  optimizer->SetCostFunction(m_CostFunction);
  optimizer->SetInitialPosition(activeParameters);
  
  try {
    optimizer->StartOptimization();
  } catch (itk::ExceptionObject exception) {
    std::cout << "Optimizer exception: " << exception.GetDescription() << std::endl;
  }
}
