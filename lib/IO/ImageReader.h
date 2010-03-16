#ifndef _IMAGE_READER_H_
#define _IMAGE_READER_H_

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <ITKImageToVTKImage.h>

class vtkAlgorithmOutput;
class vtkImageData;


class ImageReader {

 public:

  typedef float                           PixelType;
  typedef itk::Image<PixelType, 3>        ImageType;
  typedef itk::ImageFileReader<ImageType> ImageSourceType;
  typedef ImageSourceType::Pointer        ImageSourceTypePointer;

  ImageReader();
  virtual ~ImageReader();

  void SetFileName(const std::string& fileName);
  std::string GetFileName() const;

  vtkImageData* GetImageOutput(const std::string& fileName);
  vtkAlgorithmOutput* GetImageOutputPort(const std::string& fileName);

  ImageType* GetITKImage();

 protected:
  ImageSourceTypePointer m_ImageReader;

  ITKImageToVTKImage<ImageType>* m_ITKToVTKFilter;

};

#endif // _IMAGE_READER_H_
