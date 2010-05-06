#include <vtkFluorescenceWidgetsRepresentation.h>

#include <vtkActor.h>
#include <vtkImageData.h>
#include <vtkImageShiftScale.h>
#include <vtkLookupTable.h>
#include <vtkObjectFactory.h>
#include <vtkOutlineSource.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkReferenceGridSource.h>
#include <vtkRenderer.h>
#include <vtkRenderView.h>
#include <vtkTexture.h>

#include <FluorescenceImageSource.h>


vtkCxxRevisionMacro(vtkFluorescenceWidgetsRepresentation, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkFluorescenceWidgetsRepresentation);


vtkFluorescenceWidgetsRepresentation
::vtkFluorescenceWidgetsRepresentation() {
  this->SetNumberOfInputPorts(0);

  this->Simulation = NULL;

  // Set up the textured focal plane
  this->FocalPlaneImageShiftScale = vtkSmartPointer<vtkImageShiftScale>::New();
  this->FocalPlaneImageShiftScale->SetOutputScalarTypeToUnsignedChar();
  this->FocalPlaneImageShiftScale->ClampOverflowOn();

  vtkSmartPointer<vtkLookupTable> table = vtkSmartPointer<vtkLookupTable>::New();
  table->SetRange(0, 65535);
  table->SetValueRange(0.0, 1.0);
  table->SetSaturationRange(0.0, 0.0);
  table->SetRampToLinear();
  table->Build();

  this->FocalPlaneTexture = vtkSmartPointer<vtkTexture>::New();
  this->FocalPlaneTexture->InterpolateOff();
  this->FocalPlaneTexture->RepeatOff();
  this->FocalPlaneTexture->SetLookupTable(table);
  this->FocalPlaneTexture->
    SetInputConnection(this->FocalPlaneImageShiftScale->GetOutputPort());

  this->FocalPlaneSource = vtkSmartPointer<vtkPlaneSource>::New();
  this->FocalPlaneSource->SetNormal(0.0, 0.0, 1.0);

  this->FocalPlaneMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->FocalPlaneMapper->SetInputConnection(this->FocalPlaneSource->GetOutputPort());
  //this->FocalPlaneMapper->ImmediateModeRenderingOn();

  this->FocalPlaneActor = vtkSmartPointer<vtkActor>::New();
  this->FocalPlaneActor->SetMapper(this->FocalPlaneMapper);
  this->FocalPlaneActor->SetTexture(this->FocalPlaneTexture);
  this->FocalPlaneActor->PickableOff();

  // Set up the reference grid
  vtkSmartPointer<vtkProperty> focalPlaneProperty = vtkSmartPointer<vtkProperty>::New();
  focalPlaneProperty->SetRepresentationToWireframe();
  focalPlaneProperty->SetColor(1.0, 1.0, 1.0);
  focalPlaneProperty->LightingOff();

  this->FocalPlaneGrid = vtkSmartPointer<vtkReferenceGridSource>::New();
  this->FocalPlaneGrid->SetOrigin(0.0, 0.0, 0.0);

  this->FocalPlaneGridMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->FocalPlaneGridMapper->SetInputConnection(this->FocalPlaneGrid->GetOutputPort());
  
  this->FocalPlaneGridActor = vtkSmartPointer<vtkActor>::New();
  this->FocalPlaneGridActor->SetMapper(this->FocalPlaneGridMapper);
  this->FocalPlaneGridActor->PickableOff();
  this->FocalPlaneGridActor->SetProperty(focalPlaneProperty);

  // Set up the volume outline source
  this->ImageVolumeOutlineSource = vtkSmartPointer<vtkOutlineSource>::New();

  this->ImageVolumeOutlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->ImageVolumeOutlineMapper->SetInputConnection(this->ImageVolumeOutlineSource->GetOutputPort());
  
  this->ImageVolumeOutlineActor = vtkSmartPointer<vtkActor>::New();
  this->ImageVolumeOutlineActor->SetMapper(this->ImageVolumeOutlineMapper);
}


vtkFluorescenceWidgetsRepresentation
::~vtkFluorescenceWidgetsRepresentation() {

}


void
vtkFluorescenceWidgetsRepresentation
::SetFluorescenceSimulation(FluorescenceSimulation* simulation) {
  this->Simulation = simulation;
}


int
vtkFluorescenceWidgetsRepresentation
::RequestData(vtkInformation* request, vtkInformationVector** inputVector,
              vtkInformationVector* outputVector) {
  return 1;
}


void
vtkFluorescenceWidgetsRepresentation
::PrepareForRendering(vtkRenderView* view) {
  if (this->Simulation) {
    double pixelSize = this->Simulation->GetPixelSize();
    double width  = static_cast<double>(this->Simulation->GetImageWidth()) * pixelSize;
    double height = static_cast<double>(this->Simulation->GetImageHeight()) * pixelSize;
    double depth  = this->Simulation->GetFocalPlaneDepth();

    bool focalPlaneVisible = this->Simulation->GetSuperimposeFluorescenceImage();

    if (focalPlaneVisible) {
      double mapsToZero = this->Simulation->GetMinimumIntensityLevel();
      double mapsToOne  = this->Simulation->GetMaximumIntensityLevel();
      this->FocalPlaneImageShiftScale->SetShift(-mapsToZero);
      this->FocalPlaneImageShiftScale->SetScale(255.0 / (mapsToOne - mapsToZero));

      FluorescenceImageSource* imageSource = 
        this->Simulation->GetFluorescenceImageSource();

      vtkDataObject* oldImage = this->FocalPlaneImageShiftScale->GetInput();
      vtkImageData* fluorescenceImage = imageSource->GenerateFluorescenceImage();
      this->FocalPlaneImageShiftScale->SetInput(fluorescenceImage);
      if (oldImage)
        oldImage->Delete();
      this->FocalPlaneTexture->Modified();
      this->FocalPlaneTexture->Update();
                                                
      this->FocalPlaneSource->SetPoint1(width, 0.0, 0.0);
      this->FocalPlaneSource->SetPoint2(0.0, height, 0.0);
      this->FocalPlaneActor->SetPosition(0.0, 0.0, depth);
    }

    this->FocalPlaneActor->SetVisibility(focalPlaneVisible ? 1 : 0);

    this->FocalPlaneGrid->SetPoint1(0.0, height, 0.0);
    this->FocalPlaneGrid->SetPoint2(width, 0.0, 0.0);
    this->FocalPlaneGridActor->SetPosition(0.0, 0.0, depth);
    this->FocalPlaneGridActor->
      SetVisibility(this->Simulation->GetShowReferencePlane() ? 1 : 0);
    
    if (this->Simulation->GetShowReferenceGrid()) {
      double spacing = this->Simulation->GetReferenceGridSpacing();
      this->FocalPlaneGrid->SetSpacing(spacing, spacing);
    } else {
      this->FocalPlaneGrid->SetSpacing(width, height);
    }

    this->ImageVolumeOutlineSource->
      SetBounds(0.0, width, 0.0, height, 
                this->Simulation->GetFocalPlaneDepthMinimum(),
                this->Simulation->GetFocalPlaneDepthMaximum());
    this->ImageVolumeOutlineActor->SetVisibility(this->Simulation->GetShowImageVolumeOutline() ? 1 : 0);
  }

}


bool
vtkFluorescenceWidgetsRepresentation
::AddToView(vtkView* view) {
  vtkRenderView* rv = vtkRenderView::SafeDownCast(view);
  if (!rv) {
    vtkErrorMacro("Can only add to a subclass of vtkRenderView.");
    return false;
  }
  rv->GetRenderer()->AddActor(this->FocalPlaneActor);
  rv->GetRenderer()->AddActor(this->FocalPlaneGridActor);
  rv->GetRenderer()->AddActor(this->ImageVolumeOutlineActor);

  return true;
}


bool
vtkFluorescenceWidgetsRepresentation
::RemoveFromView(vtkView* view) {
  vtkRenderView* rv = vtkRenderView::SafeDownCast(view);
  if (!rv) {
    return false;
  }
  rv->GetRenderer()->RemoveActor(this->FocalPlaneActor);
  rv->GetRenderer()->RemoveActor(this->FocalPlaneGridActor);
  rv->GetRenderer()->RemoveActor(this->ImageVolumeOutlineActor);

  return true;
}


void
vtkFluorescenceWidgetsRepresentation
::PrintSelf(ostream& os, vtkIndent indent) {

}
