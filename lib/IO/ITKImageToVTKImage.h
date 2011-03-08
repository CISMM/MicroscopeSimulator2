#ifndef _ITK_IMAGE_TO_VTK_IMAGE_H_
#define _ITK_IMAGE_TO_VTK_IMAGE_H_

#include <itkImage.h>
#include <itkVTKImageExport.h>


class vtkImageData;
class vtkImageImport;
class vtkImageFlip;
class vtkAlgorithmOutput;


template <class TImage>
class ITKImageToVTKImage {
	
public:
  ITKImageToVTKImage();
  virtual ~ITKImageToVTKImage();

  void SetInput(typename TImage::Pointer input);

  vtkAlgorithmOutput* GetOutputPort();
  vtkImageData* GetOutput();

  void Modified();

  void Update();
	
protected:
  typename itk::VTKImageExport<TImage>::Pointer m_Exporter;

  vtkImageFlip*   m_FlipFilter;
  vtkImageImport* m_Importer;
	
};

#ifndef ITK_MANUAL_INSTANTIATION
#include "ITKImageToVTKImage.cxx"
#endif

#endif // _ITK_IMAGE_TO_VTK_IMAGE_H_
