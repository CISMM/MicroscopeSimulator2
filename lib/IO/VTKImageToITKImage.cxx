#ifndef _VTK_IMAGE_TO_ITK_IMAGE_CXX_
#define _VTK_IMAGE_TO_ITK_IMAGE_CXX_

#include <VTKImageToITKImage.h>

#include <vtkImageData.h>
#include <vtkImageExport.h>
#include <vtkImageFlip.h>


template <class TImage>
VTKImageToITKImage<TImage>
::VTKImageToITKImage() {
  m_FlipFilter = vtkImageFlip::New();
  m_FlipFilter->SetFilteredAxis(1);

  m_VTKExporter = vtkImageExport::New();
  m_VTKExporter->SetInputConnection(m_FlipFilter->GetOutputPort());

  m_ITKImporter = ImageImportType::New();
  m_ITKImporter->
    SetCallbackUserData(m_VTKExporter);
  m_ITKImporter->
    SetBufferPointerCallback(m_VTKExporter->GetBufferPointerCallback());
  m_ITKImporter->
    SetDataExtentCallback(m_VTKExporter->GetDataExtentCallback());
  m_ITKImporter->
    SetOriginCallback(m_VTKExporter->GetOriginCallback());
  m_ITKImporter->
    SetSpacingCallback(m_VTKExporter->GetSpacingCallback());
  m_ITKImporter->
    SetNumberOfComponentsCallback(m_VTKExporter->GetNumberOfComponentsCallback());
  m_ITKImporter->
    SetPipelineModifiedCallback(m_VTKExporter->GetPipelineModifiedCallback());
  m_ITKImporter->
    SetPropagateUpdateExtentCallback(m_VTKExporter->GetPropagateUpdateExtentCallback());
  m_ITKImporter->
    SetScalarTypeCallback(m_VTKExporter->GetScalarTypeCallback());
  m_ITKImporter->
    SetUpdateDataCallback(m_VTKExporter->GetUpdateDataCallback());
  m_ITKImporter->
    SetUpdateInformationCallback(m_VTKExporter->GetUpdateInformationCallback());
  m_ITKImporter->
    SetWholeExtentCallback(m_VTKExporter->GetWholeExtentCallback());
}


template <class TImage>
VTKImageToITKImage<TImage>
::~VTKImageToITKImage() {
  m_FlipFilter->Delete();
  m_VTKExporter->Delete();
}


template <class TImage>
void
VTKImageToITKImage<TImage>
::SetInput(vtkImageData* image) {
  m_FlipFilter->SetInput(image);
}


template <class TImage>
void
VTKImageToITKImage<TImage>
::SetInputConnection(vtkAlgorithmOutput* input) {
  m_FlipFilter->SetInputConnection(input);
}


template <class TImage>
typename VTKImageToITKImage<TImage>::ImageTypePointer
VTKImageToITKImage<TImage>
::GetOutput() {
  return m_ITKImporter->GetOutput();
}


template <class TImage>
void
VTKImageToITKImage<TImage>
::GraftOutput(ImageTypePointer image) {
  m_ITKImporter->GraftOutput(image);
}


template <class TImage>
void
VTKImageToITKImage<TImage>
::Modified() {
  m_ITKImporter->Modified();
}


template <class TImage>
void
VTKImageToITKImage<TImage>
::Update() {
  m_ITKImporter->Update();
}

#endif //  _VTK_IMAGE_TO_ITK_IMAGE_CXX_
