#include <itkFlipImageFilter.txx>
#include <itkImage.txx>
#include <itkImageFileReader.txx>
#include <ITKImageToVTKImage.cxx>

// WARNING: Always include the header file for this class AFTER
// including the ITK headers. Otherwise, the ITK headers will be included
// without including the implementation files, and you will have many linker
// errors.
#include <ImageReader.h>


ImageReader
::ImageReader() {
  m_ImageReader = ImageSourceType::New();

  // ITK apparently flips the y axis when reading TIFs.
  m_Flipper = FloatFlipType::New();
  itk::FixedArray<bool,3> flipArray;
  flipArray[0] = false;
  flipArray[1] = true; // Flip about the y-axis
  flipArray[2] = false;
  m_Flipper->SetFlipAxes(flipArray);
  m_Flipper->SetInput(m_ImageReader->GetOutput());

  m_ITKToVTKFilter = new ITKImageToVTKImage<ImageType>();
  m_ITKToVTKFilter->SetInput(m_Flipper->GetOutput());
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
