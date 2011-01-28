#include <itkRecursiveGaussianImageFilter.txx>
#include <itkStatisticsImageFilter.h>
#include <itkShiftScaleImageFilter.h>

#include <vtkImageAppendComponents.h>

// WARNING: Always include the header file for this class AFTER
// including the ITK headers. Otherwise, the ITK headers will be included
// without including the implementation files, and you will have many linker
// errors.
#include <PointSpreadFunction.h>

const std::string PointSpreadFunction::NAME_ATTRIBUTE  = "Name";
const std::string PointSpreadFunction::SUMMED_INTENSITY_ATTRIBUTE = "SummedIntensity";
const std::string PointSpreadFunction::SIZE_ELEMENT    = "Size";
const std::string PointSpreadFunction::SPACING_ELEMENT = "Spacing";
const std::string PointSpreadFunction::X_ATTRIBUTE     = "X";
const std::string PointSpreadFunction::Y_ATTRIBUTE     = "Y";
const std::string PointSpreadFunction::Z_ATTRIBUTE     = "Z";


PointSpreadFunction
::PointSpreadFunction() {
  SetSummedIntensity(1.0);
  SetSigma(200.0);

  m_Statistics = StatisticsType::New();
  
  m_ScaleFilter = ScaleFilterType::New();

  m_DerivativeX = DerivativeFilterType::New();
  m_DerivativeX->SetDirection(0);
  m_DerivativeX->SetSigma(m_Sigma);
  m_DerivativeX->SetOrder(DerivativeFilterType::FirstOrder);
  m_VTKDerivativeX = new ITKImageToVTKImage<ImageType>();
  m_VTKDerivativeX->SetInput(m_DerivativeX->GetOutput());

  m_DerivativeY = DerivativeFilterType::New();
  m_DerivativeY->SetDirection(1);
  m_DerivativeY->SetSigma(m_Sigma);
  m_DerivativeY->SetOrder(DerivativeFilterType::FirstOrder);
  m_VTKDerivativeY = new ITKImageToVTKImage<ImageType>();
  m_VTKDerivativeY->SetInput(m_DerivativeY->GetOutput());

  m_DerivativeZ = DerivativeFilterType::New();
  m_DerivativeZ->SetDirection(2);
  m_DerivativeZ->SetSigma(m_Sigma);
  m_DerivativeZ->SetOrder(DerivativeFilterType::FirstOrder);
  m_VTKDerivativeZ = new ITKImageToVTKImage<ImageType>();
  m_VTKDerivativeZ->SetInput(m_DerivativeZ->GetOutput());

  m_VTKGradient = vtkSmartPointer<vtkImageAppendComponents>::New();
  m_VTKGradient->SetInputConnection(0, m_VTKDerivativeX->GetOutputPort());
  m_VTKGradient->AddInputConnection(0, m_VTKDerivativeY->GetOutputPort());
  m_VTKGradient->AddInputConnection(0, m_VTKDerivativeZ->GetOutputPort());
}


PointSpreadFunction
::~PointSpreadFunction() {
  delete m_VTKDerivativeX;
  delete m_VTKDerivativeY;
  delete m_VTKDerivativeZ;
}


void
PointSpreadFunction
::SetName(const std::string& name) {
  m_Name = name;
}


std::string&
PointSpreadFunction
::GetName() {
  return m_Name;
}


void
PointSpreadFunction
::Update() {
  m_Statistics->Update();
  m_ScaleFilter->SetScale(m_SummedIntensity / m_Statistics->GetSum());
  UpdateGradientImage();
}


vtkImageData*
PointSpreadFunction
::GetGradientOutput() {
  return m_VTKGradient->GetOutput();
}


vtkAlgorithmOutput*
PointSpreadFunction
::GetGradientOutputPort() {
  return m_VTKGradient->GetOutputPort();
}


void
PointSpreadFunction
::SetSummedIntensity(double intensity) {
  m_SummedIntensity = intensity;
}


double
PointSpreadFunction
::GetSummedIntensity() {
  return  m_SummedIntensity;
}


void
PointSpreadFunction
::SetSigma(double sigma) {
  m_Sigma = sigma;
}


double
PointSpreadFunction
::GetSigma() {
  return m_Sigma;
}


void
PointSpreadFunction
::UpdateGradientImage() {
  m_DerivativeX->Modified();
  m_DerivativeY->Modified();
  m_DerivativeZ->Modified();
  m_VTKDerivativeX->Modified();
  m_VTKDerivativeY->Modified();
  m_VTKDerivativeZ->Modified();
  m_VTKGradient->Modified();
  m_VTKGradient->Update();
}
