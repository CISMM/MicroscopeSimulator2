#include <ImageIO.h>


ImageIO
::ImageIO() {
  m_ImageReader = ImageSourceType::New();
  m_ITKToVTKFilter = new ITKImageToVTKImage<ImageType>();
}


ImageIO
::~ImageIO() {
  delete m_ITKToVTKFilter;
}


vtkImageData*
ImageIO
::GetImageOutput(const std::string& fileName) {
  // TODO - implement

  return NULL;
}


vtkAlgorithmOutput*
ImageIO
::GetImageOutputPort(const std::string& fileName) {
  m_ImageReader->SetFileName(fileName);
  m_ImageReader->UpdateLargestPossibleRegion();

  m_ITKToVTKFilter->SetInput(m_ImageReader->GetOutput());

  return m_ITKToVTKFilter->GetOutputPort();
}
