#include <FluorescenceOptimizer.h>

#include <vtkImageData.h>

#include <FluorescenceImageSource.h>
#include <FluorescenceSimulation.h>
#include <ModelObjectList.h>

// TEMP
#include <itkImageFileWriter.h>
#include <itkCastImageFilter.h>
// END TEMP


FluorescenceOptimizer
::FluorescenceOptimizer() {
  m_FluoroSim = NULL;
  m_ModelObjectList = NULL;
  
  m_FluorescenceImageSource = SyntheticImageSourceType::New();

  m_ImageToImageCostFunction = ImageToImageCostFunctionType::New();
  m_CostFunction = ParameterizedCostFunctionType::New();
  m_CostFunction->SetImageToImageMetric(m_ImageToImageCostFunction);
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
::Optimize() {
  // Make sure to set the fluorescence image source.
  m_FluorescenceImageSource->
    SetFluorescenceImageSource(m_FluoroSim->GetFluorescenceImageSource());

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

  // Connect to the cost function, set the initial parameters, and optimize.
  m_ImageToImageCostFunction
    ->SetFixedImageRegion(m_FluorescenceImageSource->GetOutput()->GetLargestPossibleRegion());
  m_Optimizer->SetCostFunction(m_CostFunction);
  m_Optimizer->SetFunctionConvergenceTolerance(1e-3);
  m_Optimizer->SetInitialPosition(activeParameters);
  m_Optimizer->StartOptimization();





  /* Test writing out a TIF file. */
  typedef SyntheticImageSourceType::OutputImageType ImageType;
  typedef unsigned short UnsignedShortPixelType;
  typedef itk::Image<UnsignedShortPixelType, ImageType::ImageDimension> OutputImageType;
  typedef itk::CastImageFilter<ImageType, OutputImageType> CastFilterType;

  CastFilterType::Pointer caster = CastFilterType::New();
  caster->SetInput(m_FluorescenceImageSource->GetOutput());

  itk::ImageFileWriter<OutputImageType>::Pointer writer =
    itk::ImageFileWriter<OutputImageType>::New();
  writer->SetInput(caster->GetOutput());
  writer->SetFileName("/Users/cquammen/Desktop/ITK.tif");
  try {
    writer->Write();
  } catch (itk::ExceptionObject e) {
    std::cout << "Error on write:" << std::endl;
    std::cout << e.GetDescription() << std::endl;
  }
  /* End test writing out a TIF file. */

#if 0
  for (double x = 0; x < 3000.0; x += 100) {
    double params[1];
    params[0]= x;
    SetParameters(params);
  }
#endif
}


void
FluorescenceOptimizer
::SetParameters(double* params) {
  if (!m_ModelObjectList)
    return;

  m_FluoroSim->GetFluorescenceImageSource()->SetParameters(params);
}
