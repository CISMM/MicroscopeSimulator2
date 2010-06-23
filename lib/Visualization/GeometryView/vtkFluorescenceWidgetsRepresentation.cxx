#include <vtkFluorescenceWidgetsRepresentation.h>

#include <vtkActor.h>
#include <vtkImageData.h>
#include <vtkImageShiftScale.h>
#include <vtkLookupTable.h>
#include <vtkMatrix4x4.h>
#include <vtkMatrixToLinearTransform.h>
#include <vtkObjectFactory.h>
#include <vtkOutlineSource.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkReferenceGridSource.h>
#include <vtkRenderer.h>
#include <vtkRenderView.h>
#include <vtkTexture.h>
#include <vtkTransformPolyDataFilter.h>

#include <FluorescenceImageSource.h>


vtkCxxRevisionMacro(vtkFluorescenceWidgetsRepresentation, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkFluorescenceWidgetsRepresentation);


vtkFluorescenceWidgetsRepresentation
::vtkFluorescenceWidgetsRepresentation() {
  this->SetNumberOfInputPorts(0);

  this->Simulation = NULL;

  // Set up the textured focal plane
  this->SyntheticFocalPlaneImageShiftScale = vtkSmartPointer<vtkImageShiftScale>::New();
  this->SyntheticFocalPlaneImageShiftScale->SetOutputScalarTypeToUnsignedChar();
  this->SyntheticFocalPlaneImageShiftScale->ClampOverflowOn();

  vtkSmartPointer<vtkLookupTable> table = vtkSmartPointer<vtkLookupTable>::New();
  table->SetRange(0, 65535);
  table->SetValueRange(0.0, 1.0);
  table->SetSaturationRange(0.0, 0.0);
  table->SetRampToLinear();
  table->Build();

  this->ShearTransformMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  
  this->SyntheticFocalPlaneTexture = vtkSmartPointer<vtkTexture>::New();
  this->SyntheticFocalPlaneTexture->InterpolateOff();
  this->SyntheticFocalPlaneTexture->RepeatOff();
  this->SyntheticFocalPlaneTexture->SetLookupTable(table);
  this->SyntheticFocalPlaneTexture->
    SetInputConnection(this->SyntheticFocalPlaneImageShiftScale->GetOutputPort());

  this->SyntheticFocalPlaneSource = vtkSmartPointer<vtkPlaneSource>::New();
  this->SyntheticFocalPlaneSource->SetNormal(0.0, 0.0, 1.0);

  this->SyntheticFocalPlaneMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->SyntheticFocalPlaneMapper->SetInputConnection(this->SyntheticFocalPlaneSource->GetOutputPort());

  this->SyntheticFocalPlaneActor = vtkSmartPointer<vtkActor>::New();
  this->SyntheticFocalPlaneActor->SetMapper(this->SyntheticFocalPlaneMapper);
  this->SyntheticFocalPlaneActor->SetTexture(this->SyntheticFocalPlaneTexture);
  this->SyntheticFocalPlaneActor->SetUserMatrix(this->ShearTransformMatrix);
  this->SyntheticFocalPlaneActor->PickableOff();

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
  this->FocalPlaneGridActor->SetUserMatrix(this->ShearTransformMatrix);

  // Set up the volume outline source
  this->ImageVolumeOutlineSource = vtkSmartPointer<vtkOutlineSource>::New();

  this->ImageVolumeOutlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->ImageVolumeOutlineMapper->SetInputConnection(this->ImageVolumeOutlineSource->GetOutputPort());
  
  this->ImageVolumeOutlineActor = vtkSmartPointer<vtkActor>::New();
  this->ImageVolumeOutlineActor->SetMapper(this->ImageVolumeOutlineMapper);
  this->ImageVolumeOutlineActor->SetUserMatrix(this->ShearTransformMatrix);
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
    double depth  = this->Simulation->GetFocalPlanePosition();

    bool focalPlaneVisible = this->Simulation->GetSuperimposeFluorescenceImage();

    if (focalPlaneVisible) {
      double mapsToZero = this->Simulation->GetMinimumIntensityLevel();
      double mapsToOne  = this->Simulation->GetMaximumIntensityLevel();
      this->SyntheticFocalPlaneImageShiftScale->SetShift(-mapsToZero);
      this->SyntheticFocalPlaneImageShiftScale->SetScale(255.0 / (mapsToOne - mapsToZero));

      FluorescenceImageSource* imageSource = 
        this->Simulation->GetFluorescenceImageSource();

      vtkDataObject* oldImage = this->SyntheticFocalPlaneImageShiftScale->GetInput();
      vtkImageData* fluorescenceImage = imageSource->GenerateFluorescenceImage();
      this->SyntheticFocalPlaneImageShiftScale->SetInput(fluorescenceImage);
      if (oldImage)
        oldImage->Delete();
      this->SyntheticFocalPlaneTexture->Modified();
      this->SyntheticFocalPlaneTexture->Update();
                                                
      this->SyntheticFocalPlaneSource->SetPoint1(width, 0.0, 0.0);
      this->SyntheticFocalPlaneSource->SetPoint2(0.0, height, 0.0);
      this->SyntheticFocalPlaneActor->SetPosition(0.0, 0.0, depth);
    }

    this->SyntheticFocalPlaneActor->SetVisibility(focalPlaneVisible ? 1 : 0);

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

    double minDepth = this->Simulation->GetMinimumFocalPlanePosition();
    double maxDepth = this->Simulation->GetMaximumFocalPlanePosition();
    this->ImageVolumeOutlineSource->
      SetBounds(0.0, width, 0.0, height, minDepth, maxDepth);
    this->ImageVolumeOutlineActor->SetVisibility(this->Simulation->GetShowImageVolumeOutline() ? 1 : 0);

    this->ShearTransformMatrix->Identity();
    this->ShearTransformMatrix->SetElement(0, 2, -(this->Simulation->GetShearInX()));
    this->ShearTransformMatrix->SetElement(1, 2, -(this->Simulation->GetShearInY()));
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
  rv->GetRenderer()->AddActor(this->SyntheticFocalPlaneActor);
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
  rv->GetRenderer()->RemoveActor(this->SyntheticFocalPlaneActor);
  rv->GetRenderer()->RemoveActor(this->FocalPlaneGridActor);
  rv->GetRenderer()->RemoveActor(this->ImageVolumeOutlineActor);

  return true;
}


void
vtkFluorescenceWidgetsRepresentation
::PrintSelf(ostream& os, vtkIndent indent) {

}
