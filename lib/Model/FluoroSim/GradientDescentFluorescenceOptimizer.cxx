#include <itkGradientDescentOptimizer.h>
#include <itkFluorescenceImageSource.h>

#include <GradientDescentFluorescenceOptimizer.h>

#include <FluorescenceSimulation.h>
#include <ImageModelObject.h>


GradientDescentFluorescenceOptimizer
::GradientDescentFluorescenceOptimizer() {

}


GradientDescentFluorescenceOptimizer
::~GradientDescentFluorescenceOptimizer() {

}


void
GradientDescentFluorescenceOptimizer
::Optimize() {
  GradientDescentOptimizerType::Pointer optimizer = GradientDescentOptimizerType::New();
  
  // TODO - set optimizer parameters here

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
  m_ImageToImageCostFunction
    ->SetFixedImageRegion(m_FluorescenceImageSource->GetOutput()->GetLargestPossibleRegion());
  
  m_CostFunction->SetImageToImageMetric(m_ImageToImageCostFunction);
  optimizer->SetCostFunction(m_CostFunction);
  optimizer->SetInitialPosition(activeParameters);
  
  try {
    optimizer->StartOptimization();
  } catch (itk::ExceptionObject exception) {
    std::cout << "Optimizer exception: " << exception.GetDescription() << std::endl;
  }
}

