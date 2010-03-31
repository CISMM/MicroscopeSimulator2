#include <FluorescenceOptimizer.h>

#include <vtkImageData.h>

#include <FluorescenceImageSource.h>
#include <FluorescenceSimulation.h>
#include <ImageModelObject.h>
#include <ModelObjectList.h>

#include <itkExceptionObject.h>


FluorescenceOptimizer
::FluorescenceOptimizer() {
  m_FluoroSim = NULL;
  m_ModelObjectList = NULL;
  m_ComparisonImageModelObject = NULL;
  
  m_FluorescenceImageSource = SyntheticImageSourceType::New();

  m_ImageToImageCostFunctionType = GAUSSIAN_NOISE_COST_FUNCTION;
  m_OptimizerType = NELDER_MEAD_OPTIMIZER;

  m_CostFunction = ParameterizedCostFunctionType::New();
  
  m_ImageToImageCostFunction = GaussianNoiseCostFunctionType::New();

  SetUpOptimizer();
}


FluorescenceOptimizer
::~FluorescenceOptimizer() {
}


void
FluorescenceOptimizer
::SetFluorescenceSimulation(FluorescenceSimulation* simulation) {
  m_FluoroSim = simulation;
}


void
FluorescenceOptimizer
::SetModelObjectList(ModelObjectList* list) {
  m_ModelObjectList = list;
}


void
FluorescenceOptimizer
::SetComparisonImageModelObjectIndex(int index) {
  if (index < 0) {
    m_ComparisonImageModelObject = NULL;
    return;
  }

  ModelObjectPtr mo = m_ModelObjectList->GetModelObjectAtIndex(index, ImageModelObject::OBJECT_TYPE_NAME);
  m_ComparisonImageModelObject = static_cast<ImageModelObject*>(mo);

  // Connect comparison image to optimization machinery
  m_CostFunction->SetFixedImage(m_ComparisonImageModelObject->GetITKImage());
}


void
FluorescenceOptimizer
::Optimize() {

  try {

    SetUpOptimizer();

    // Make sure to set the fluorescence image source and moving image.
    m_FluorescenceImageSource->
      SetFluorescenceImageSource(m_FluoroSim->GetFluorescenceImageSource());

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
    m_ImageToImageCostFunction
      ->SetFixedImageRegion(m_FluorescenceImageSource->GetOutput()->GetLargestPossibleRegion());

    m_CostFunction->SetImageToImageMetric(m_ImageToImageCostFunction);

    m_Optimizer->SetCostFunction(m_CostFunction);
    m_Optimizer->SetInitialPosition(activeParameters);
    m_Optimizer->StartOptimization();
    
  } catch (itk::ExceptionObject exception) {
    std::cout << "Optimizer exception: " << exception.GetDescription() << std::endl;
  }

}


void
FluorescenceOptimizer
::SetParameters(double* params) {
  if (!m_ModelObjectList)
    return;

  m_FluoroSim->GetFluorescenceImageSource()->SetParameters(params);
}


void
FluorescenceOptimizer
::SetUpOptimizer() {

  // Set up the cost function
  if (m_ImageToImageCostFunctionType == GAUSSIAN_NOISE_COST_FUNCTION) {
    m_ImageToImageCostFunction = GaussianNoiseCostFunctionType::New();
    std::cout << "Gaussian noise cost function, ";
  } else if (m_ImageToImageCostFunctionType == POISSON_NOISE_COST_FUNCTION) {
    m_ImageToImageCostFunction = PoissonNoiseCostFunctionType::New();
    std::cout << "Poisson noise cost function, ";
  } else if (m_ImageToImageCostFunctionType == NORMALIZED_CORRELATION_COST_FUNCTION) {
    m_ImageToImageCostFunction = NormalizedCorrelationCostFunctionType::New();
    std::cout << "Normalized correlation cost function, ";
  }

  // Set up the optimizer
  if (m_OptimizerType == NELDER_MEAD_OPTIMIZER) {
    NelderMeadOptimizerType::Pointer optimizer = NelderMeadOptimizerType::New();
    optimizer->SetFunctionConvergenceTolerance(1e-3);
    m_Optimizer = optimizer;
    std::cout << "Nelder-Mead optimizer";
  } else if (m_OptimizerType == GRADIENT_DESCENT_OPTIMIZER) {
    GradientDescentOptimizerType::Pointer optimizer = GradientDescentOptimizerType::New();
    m_Optimizer = optimizer;
    std::cout << "Gradient descent optimizer";
  }

  std::cout << std::endl;
}
