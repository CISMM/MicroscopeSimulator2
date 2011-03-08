#ifndef _ITK_IMAGE_TO_VTK_IMAGE_CXX_
#define _ITK_IMAGE_TO_VTK_IMAGE_CXX_

#include <ITKImageToVTKImage.h>

#include <vtkImageData.h>
#include <vtkImageImport.h>
#include <vtkImageFlip.h>


template <class TImage>
ITKImageToVTKImage<TImage>
::ITKImageToVTKImage() {
  m_FlipFilter = vtkImageFlip::New();
  m_FlipFilter->SetFilteredAxis(1);

  m_Exporter = itk::VTKImageExport<TImage>::New();

  m_Importer = vtkImageImport::New();
  m_Importer->SetUpdateInformationCallback(m_Exporter->GetUpdateInformationCallback());
  m_Importer->SetPipelineModifiedCallback(m_Exporter->GetPipelineModifiedCallback());
  m_Importer->SetWholeExtentCallback(m_Exporter->GetWholeExtentCallback());
  m_Importer->SetSpacingCallback(m_Exporter->GetSpacingCallback());
  m_Importer->SetOriginCallback(m_Exporter->GetOriginCallback());
  m_Importer->SetScalarTypeCallback(m_Exporter->GetScalarTypeCallback());
  m_Importer->SetNumberOfComponentsCallback(m_Exporter->GetNumberOfComponentsCallback());
  m_Importer->SetPropagateUpdateExtentCallback(m_Exporter->GetPropagateUpdateExtentCallback());
  m_Importer->SetUpdateDataCallback(m_Exporter->GetUpdateDataCallback());
  m_Importer->SetDataExtentCallback(m_Exporter->GetDataExtentCallback());
  m_Importer->SetBufferPointerCallback(m_Exporter->GetBufferPointerCallback());
  m_Importer->SetCallbackUserData(m_Exporter->GetCallbackUserData());

  m_FlipFilter->SetInputConnection(m_Importer->GetOutputPort());
}


template <class TImage>
ITKImageToVTKImage<TImage>
::~ITKImageToVTKImage() {
  m_Importer->Delete();
  m_FlipFilter->Delete();
}


template <class TImage>
void
ITKImageToVTKImage<TImage>
::SetInput(typename TImage::Pointer input) {
  m_Exporter->SetInput(input);
}


template <class TImage>
vtkAlgorithmOutput*
ITKImageToVTKImage<TImage>
::GetOutputPort() {
  return m_FlipFilter->GetOutputPort();
}


template <class TImage>
vtkImageData*
ITKImageToVTKImage<TImage>
::GetOutput() {
  return m_FlipFilter->GetOutput();
}


template <class TImage>
void
ITKImageToVTKImage<TImage>
::Modified() {
  m_Importer->Modified();
  m_FlipFilter->Modified();
}


template <class TImage>
void
ITKImageToVTKImage<TImage>
::Update() {
  m_Importer->Update();
  m_FlipFilter->Update();
}

#endif // _ITK_IMAGE_TO_VTK_IMAGE_CXX_ 
