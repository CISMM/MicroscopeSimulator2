#include <vtkImagePlaneWidgetRepresentation.h>

#include <vtkAbstractPicker.h>
#include <vtkAbstractPropPicker.h>
#include <vtkImageData.h>
#include <vtkImagePlaneWidget.h>
#include <vtkObjectFactory.h>
#include <vtkProperty.h>
#include <vtkRenderView.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

vtkStandardNewMacro(vtkImagePlaneWidgetRepresentation);

vtkImagePlaneWidgetRepresentation
::vtkImagePlaneWidgetRepresentation() {
  this->SetNumberOfInputPorts(1);

  this->XWidget = vtkSmartPointer<vtkImagePlaneWidget>::New();
  this->XWidget->SetKeyPressActivationValue('x');
  this->XWidget->RestrictPlaneToVolumeOn();
  this->XWidget->GetPlaneProperty()->SetColor(1, 0, 0);
  this->XWidget->TextureInterpolateOff();
  this->XWidget->SetResliceInterpolateToNearestNeighbour();
  this->XWidget->SetPlaneOrientationToXAxes();
  this->XWidget->SetMarginSizeX(0.0);
  this->XWidget->SetMarginSizeY(0.0);
  this->XWidget->DisplayTextOn();

  this->YWidget = vtkSmartPointer<vtkImagePlaneWidget>::New();
  this->YWidget->SetKeyPressActivationValue('y');
  this->YWidget->RestrictPlaneToVolumeOn();
  this->YWidget->GetPlaneProperty()->SetColor(1, 0, 0);
  this->YWidget->TextureInterpolateOff();
  this->YWidget->SetResliceInterpolateToNearestNeighbour();
  this->YWidget->SetPlaneOrientationToYAxes();
  this->YWidget->SetMarginSizeX(0.0);
  this->YWidget->SetMarginSizeY(0.0);
  this->YWidget->DisplayTextOn();

  this->ZWidget = vtkSmartPointer<vtkImagePlaneWidget>::New();
  this->ZWidget->SetKeyPressActivationValue('z');
  this->ZWidget->RestrictPlaneToVolumeOn();
  this->ZWidget->GetPlaneProperty()->SetColor(1, 0, 0);
  this->ZWidget->TextureInterpolateOff();
  this->ZWidget->SetResliceInterpolateToNearestNeighbour();
  this->ZWidget->SetPlaneOrientationToZAxes();
  this->ZWidget->SetMarginSizeX(0.0);
  this->ZWidget->SetMarginSizeY(0.0);
  this->ZWidget->DisplayTextOn();
}


vtkImagePlaneWidgetRepresentation
::~vtkImagePlaneWidgetRepresentation() {
  
}


void
vtkImagePlaneWidgetRepresentation
::PrepareForRendering(vtkRenderView* view) {
  vtkRenderWindowInteractor* interactor = view->GetRenderWindow()->GetInteractor();
  if (!this->XWidget->GetInteractor()) {
    this->XWidget->SetInteractor(interactor);
    this->YWidget->SetInteractor(interactor);
    this->ZWidget->SetInteractor(interactor);
  }

  if (this->GetInputDataObject(0,0)) {
    this->XWidget->SetInputData(dynamic_cast<vtkImageData*>(this->GetInputDataObject(0, 0)));
    this->YWidget->SetInputData(dynamic_cast<vtkImageData*>(this->GetInputDataObject(0, 0)));
    this->ZWidget->SetInputData(dynamic_cast<vtkImageData*>(this->GetInputDataObject(0, 0)));

    vtkAbstractPropPicker* picker = dynamic_cast<vtkAbstractPropPicker*>(interactor->GetPicker());
    this->XWidget->SetPicker(picker);
    this->YWidget->SetPicker(picker);
    this->ZWidget->SetPicker(picker);
  }
}


bool
vtkImagePlaneWidgetRepresentation
::AddToView(vtkView* view) {
  this->XWidget->On();
  this->YWidget->On();
  this->ZWidget->On();

  return true;
}


bool
vtkImagePlaneWidgetRepresentation
::RemoveFromView(vtkView* view) {
  this->XWidget->Off();
  this->YWidget->Off();
  this->ZWidget->Off();

  return true;
}


void
vtkImagePlaneWidgetRepresentation
::PrintSelf(ostream& os, vtkIndent indent) {

}
