#ifndef _IMAGE_WRITER_H_
#define _IMAGE_WRITER_H_

#define ITK_MANUAL_INSTANTIATION
#include <itkImage.h>
#include <itkImageFileWriter.h>
#include <itkShiftScaleImageFilter.h>

#include <VTKImageToITKImage.h>
#undef ITK_MANUAL_INSTANTIATION


class ImageWriter {

 public:

  // unsigned short image type
  typedef unsigned short                  UShortPixelType;
  typedef itk::Image<UShortPixelType, 3>  UShortImageType;
  
  typedef float                           FloatPixelType;
  typedef itk::Image<FloatPixelType, 3>   FloatImageType;

  typedef itk::ShiftScaleImageFilter<FloatImageType, UShortImageType> FloatCastFilterType;
  typedef FloatCastFilterType::Pointer                                FloatCastFilterTypePointer;

  typedef itk::ImageFileWriter<UShortImageType> UShortWriterType;
  typedef UShortWriterType::Pointer             UShortWriterTypePointer;


  ImageWriter();
  virtual ~ImageWriter();

  void SetFileName(const std::string& fileName);
  std::string GetFileName() const;

  void SetImage(FloatImageType* image);

  void SetImage(vtkImageData* image);
  void SetInputConnection(vtkAlgorithmOutput* input);

  void WriteUShortImage();

 protected:
  VTKImageToITKImage<FloatImageType>* m_FloatVTKToITKFilter;

  FloatCastFilterTypePointer m_FloatCaster;

  UShortWriterTypePointer    m_UShortWriter;
};


#endif // _IMAGE_WRITER_H_
