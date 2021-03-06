#ifndef _VTK_IMAGE_TO_ITK_IMAGE_H_
#define _VTK_IMAGE_TO_ITK_IMAGE_H_

#include <itkImage.h>
#include <itkVTKImageImport.h>

class vtkImageData;
class vtkImageExport;
class vtkImageFlip;
class vtkAlgorithmOutput;


template <class TImage>
class VTKImageToITKImage {
	
 public:
  typedef typename TImage::Pointer          ImageTypePointer;
  typedef itk::VTKImageImport<TImage>       ImageImportType;
  typedef typename ImageImportType::Pointer ImageImportTypePointer;

  VTKImageToITKImage();
  virtual ~VTKImageToITKImage();

  void SetInput(vtkImageData* image);
  void SetInputConnection(vtkAlgorithmOutput* input);

  ImageTypePointer GetOutput();

  void GraftOutput(ImageTypePointer image);

  void Modified();

  void Update();
	
 protected:
  ImageImportTypePointer m_ITKImporter;

  vtkImageFlip*   m_FlipFilter;
  vtkImageExport* m_VTKExporter;
	
};

#ifndef ITK_MANUAL_INSTANTIATION
#include "VTKImageToITKImage.cxx"
#endif

#endif // _VTK_IMAGE_TO_ITK_IMAGE_H_
