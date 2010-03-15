#ifndef _IMAGE_IO_H_
#define _IMAGE_IO_H_

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <ITKImageToVTKImage.h>

class vtkAlgorithmOutput;
class vtkImageData;


class ImageIO {

 public:
  ImageIO();
  virtual ~ImageIO();

  vtkImageData* GetImageOutput(const std::string& fileName);
  vtkAlgorithmOutput* GetImageOutputPort(const std::string& fileName);

  typedef float                           PixelType;
  typedef itk::Image<PixelType, 3>        ImageType;
  typedef itk::ImageFileReader<ImageType> ImageSourceType;
  typedef ImageSourceType::Pointer        ImageSourceTypePointer;

 protected:
  ImageSourceTypePointer m_ImageReader;

  ITKImageToVTKImage<ImageType>* m_ITKToVTKFilter;

};

#endif // _IMAGE_IO_H_
