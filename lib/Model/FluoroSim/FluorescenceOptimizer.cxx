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

  for (double x = 0; x < 3000.0; x += 100) {
    double params[1];
    params[0]= x;
    SetParameters(params);
  }
}


void
FluorescenceOptimizer
::SetParameters(double* params) {
  if (!m_ModelObjectList)
    return;

  m_FluoroSim->GetFluorescenceImageSource()->SetParameters(params);
}
