#include <itkFlipImageFilter.hxx>
#include <itkImage.hxx>
#include <itkImageFileReader.hxx>
#include <ITKImageToVTKImage.cxx>

// WARNING: Always include the header file for this class AFTER
// including the ITK headers. Otherwise, the ITK headers will be included
// without including the implementation files, and you will have many linker
// errors.
#include <ImageReader.h>


ImageReader
::ImageReader() {
  m_ImageReader = ImageSourceType::New();

  m_ITKToVTKFilter = new ITKImageToVTKImage<ImageType>();
  m_ITKToVTKFilter->SetInput(m_ImageReader->GetOutput());
}


ImageReader
::~ImageReader() {
  delete m_ITKToVTKFilter;
}


void
ImageReader
::SetFileName(const std::string& fileName) {
  m_ImageReader->SetFileName(fileName);
}


std::string
ImageReader
::GetFileName() const {
  return m_ImageReader->GetFileName();
}


vtkImageData*
ImageReader
::GetImageOutput(const std::string& fileName) {
  m_ImageReader->SetFileName(fileName);
  m_ImageReader->UpdateLargestPossibleRegion();

  return m_ITKToVTKFilter->GetOutput();
}


vtkAlgorithmOutput*
ImageReader
::GetImageOutputPort(const std::string& fileName) {
  m_ImageReader->SetFileName(fileName);
  m_ImageReader->UpdateLargestPossibleRegion();

  return m_ITKToVTKFilter->GetOutputPort();
}


ImageReader::ImageType*
ImageReader
::GetITKImage() {
  return m_ImageReader->GetOutput();
}
