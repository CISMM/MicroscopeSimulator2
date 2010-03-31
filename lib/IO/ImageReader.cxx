#include <itkImage.txx>
#include <itkImageFileReader.txx>
#include <ITKImageToVTKImage.cxx>

#include <ImageReader.h>


ImageReader
::ImageReader() {
  m_ImageReader = ImageSourceType::New();
  m_ITKToVTKFilter = new ITKImageToVTKImage<ImageType>();
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
  // TODO - implement

  return NULL;
}


vtkAlgorithmOutput*
ImageReader
::GetImageOutputPort(const std::string& fileName) {
  m_ImageReader->SetFileName(fileName);
  m_ImageReader->UpdateLargestPossibleRegion();

  m_ITKToVTKFilter->SetInput(m_ImageReader->GetOutput());

  return m_ITKToVTKFilter->GetOutputPort();
}


ImageReader::ImageType*
ImageReader
::GetITKImage() {
  return m_ImageReader->GetOutput();
}
