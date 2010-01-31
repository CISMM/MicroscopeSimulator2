#include "ImagePlaneVisualizationPipeline.h"

#include <vtkAlgorithmOutput.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageShiftScale.h>
#include <vtkRenderer.h>


const int ImagePlaneVisualizationPipeline::X_PLANE = 1;
const int ImagePlaneVisualizationPipeline::Y_PLANE = 2;
const int ImagePlaneVisualizationPipeline::Z_PLANE = 3;


ImagePlaneVisualizationPipeline
::ImagePlaneVisualizationPipeline() {

  // Set to X dimension by default.
  this->planeDimension = X_PLANE;

  // Set up pipeline.
  this->shiftScaler = vtkSmartPointer<vtkImageShiftScale>::New();
  this->shiftScaler->SetOutputScalarTypeToUnsignedChar();
  this->shiftScaler->ClampOverflowOn();

  // It is essential to set the input algorithm.
  this->SetInputAlgorithm(this->shiftScaler);

  this->imageActor = vtkSmartPointer<vtkImageActor>::New();
  this->imageActor->InterpolateOn();
  this->imageActor->SetInput(this->shiftScaler->GetOutput());

  this->autoRescalingOn = true;
}


ImagePlaneVisualizationPipeline
::~ImagePlaneVisualizationPipeline() {
}


void
ImagePlaneVisualizationPipeline
::SetInputConnection(vtkAlgorithmOutput* input) {
  this->input = input;
  this->inputAlgorithm->SetInputConnection(input);

    // Update shift/scale filter automatically if auto-rescaling is on.
  if (input && this->autoRescalingOn) {
    // Gotta be a better way to do this.
    input->GetProducer()->Update();
    vtkImageData* originalImage 
      = vtkImageData::SafeDownCast(input->GetProducer()->GetOutputDataObject(0));
    double* scalarRange = originalImage->GetScalarRange();
    this->shiftScaler->SetShift(-scalarRange[0]);
    this->shiftScaler->SetScale(255.0 / (scalarRange[1] - scalarRange[0]));
  } else {
    this->shiftScaler->SetShift(0.0);
    this->shiftScaler->SetScale(1.0);
  }
  this->shiftScaler->Update();
}


void
ImagePlaneVisualizationPipeline
::AddToRenderer(vtkRenderer* renderer) {
  renderer->AddActor(this->imageActor);
}


void
ImagePlaneVisualizationPipeline
::SetToXPlane() {
  this->planeDimension = X_PLANE;
  this->SetSliceNumber(0);
}


void
ImagePlaneVisualizationPipeline
::SetToYPlane() {
  this->planeDimension = Y_PLANE;
  this->SetSliceNumber(0);
}


void
ImagePlaneVisualizationPipeline
::SetToZPlane() {
  this->planeDimension = Z_PLANE;
  this->SetSliceNumber(0);
}


void
ImagePlaneVisualizationPipeline
::SetVisible(bool visible) {
  this->imageActor->SetVisibility(visible ? 1 : 0);
}


bool
ImagePlaneVisualizationPipeline
::GetVisible() {
  return this->imageActor->GetVisibility() == 1;
}


void
ImagePlaneVisualizationPipeline
::SetSliceNumber(int sliceNumber) {
  int* extent = this->imageActor->GetInput()->GetWholeExtent();
  
  if (this->planeDimension == X_PLANE) {
    this->imageActor->SetDisplayExtent(sliceNumber, sliceNumber,
                 extent[2], extent[3],
                 extent[4], extent[5]);
  
  } else if (this->planeDimension == Y_PLANE) {
    this->imageActor->SetDisplayExtent(extent[0], extent[1],
                 sliceNumber, sliceNumber,
                 extent[4], extent[5]);
  } else {
    this->imageActor->SetDisplayExtent(extent[0], extent[1],
                 extent[2], extent[3],
                 sliceNumber, sliceNumber);
  }
  this->imageActor->Modified();
}


int
ImagePlaneVisualizationPipeline
::GetSliceNumber() {
  return this->imageActor->GetZSlice();
}
