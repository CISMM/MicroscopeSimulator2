#include <itkFlipImageFilter.hxx>
#include <itkImage.hxx>
#include <itkImageFileWriter.hxx>
#include <itkShiftScaleImageFilter.hxx>

#include <VTKImageToITKImage.cxx>

#include <vtkAlgorithmOutput.h>

// WARNING: Always include the header file for this class AFTER
// including the ITK headers. Otherwise, the ITK headers will be included
// without including the implementation files, and you will have many linker
// errors.
#include <ImageWriter.h>


ImageWriter
::ImageWriter() {
  m_FloatVTKToITKFilter = new VTKImageToITKImage<FloatImageType>();

  m_FloatCaster = FloatCastFilterType::New();

  m_UShortWriter = UShortWriterType::New();
  m_UShortWriter->SetInput(m_FloatCaster->GetOutput());
}


ImageWriter
::~ImageWriter() {
  delete m_FloatVTKToITKFilter;
}


void
ImageWriter
::SetFileName(const std::string& fileName) {
  m_UShortWriter->SetFileName(fileName);
}


std::string
ImageWriter
::GetFileName() const {
  return m_UShortWriter->GetFileName();
}


void
ImageWriter
::SetImage(FloatImageType* image) {
  m_FloatCaster->SetInput(image);
}


void
ImageWriter
::SetImage(vtkImageData* image) {
  m_FloatVTKToITKFilter->SetInput(image);
  m_FloatCaster->SetInput(m_FloatVTKToITKFilter->GetOutput());
}


void
ImageWriter
::SetInputConnection(vtkAlgorithmOutput* input) {
  m_FloatVTKToITKFilter->SetInputConnection(input);
  m_FloatCaster->SetInput(m_FloatVTKToITKFilter->GetOutput());
}


void
ImageWriter
::WriteUShortImage() {
  m_UShortWriter->Write();
}



