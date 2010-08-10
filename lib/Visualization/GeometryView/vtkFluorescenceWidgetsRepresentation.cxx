#include <vtkFluorescenceWidgetsRepresentation.h>

#include <vtkActor.h>
#include <vtkImageClip.h>
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
#include <ImageModelObject.h>


vtkCxxRevisionMacro(vtkFluorescenceWidgetsRepresentation, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkFluorescenceWidgetsRepresentation);


vtkFluorescenceWidgetsRepresentation
::vtkFluorescenceWidgetsRepresentation() {
  this->SetNumberOfInputPorts(0);

  this->Simulation = NULL;

  vtkSmartPointer<vtkLookupTable> table = vtkSmartPointer<vtkLookupTable>::New();
  table->SetRange(0, 65535);
  table->SetValueRange(0.0, 1.0);
  table->SetSaturationRange(0.0, 0.0);
  table->SetRampToLinear();
  table->Build();

  this->ShearTransformMatrix = vtkSmartPointer<vtkMatrix4x4>::New();

  // Set up the textured focal plane for the synthetic image
  this->SimulatedFocalPlaneImageShiftScale = vtkSmartPointer<vtkImageShiftScale>::New();
  this->SimulatedFocalPlaneImageShiftScale->SetOutputScalarTypeToUnsignedChar();
  this->SimulatedFocalPlaneImageShiftScale->ClampOverflowOn();
  
  this->SimulatedFocalPlaneTexture = vtkSmartPointer<vtkTexture>::New();
  this->SimulatedFocalPlaneTexture->InterpolateOff();
  this->SimulatedFocalPlaneTexture->RepeatOff();
  this->SimulatedFocalPlaneTexture->SetLookupTable(table);
  this->SimulatedFocalPlaneTexture->
    SetInputConnection(this->SimulatedFocalPlaneImageShiftScale->GetOutputPort());

  this->SimulatedFocalPlaneSource = vtkSmartPointer<vtkPlaneSource>::New();
  this->SimulatedFocalPlaneSource->SetNormal(0.0, 0.0, 1.0);

  this->SimulatedFocalPlaneMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->SimulatedFocalPlaneMapper->SetInputConnection(this->SimulatedFocalPlaneSource->GetOutputPort());

  this->SimulatedFocalPlaneActor = vtkSmartPointer<vtkActor>::New();
  this->SimulatedFocalPlaneActor->SetMapper(this->SimulatedFocalPlaneMapper);
  this->SimulatedFocalPlaneActor->SetTexture(this->SimulatedFocalPlaneTexture);
  this->SimulatedFocalPlaneActor->SetUserMatrix(this->ShearTransformMatrix);
  this->SimulatedFocalPlaneActor->PickableOff();

  // Set up the textured focal plane for the experimental image
  this->ComparisonFocalPlaneImageShiftScale = vtkSmartPointer<vtkImageShiftScale>::New();
  this->ComparisonFocalPlaneImageShiftScale->SetOutputScalarTypeToUnsignedChar();
  this->ComparisonFocalPlaneImageShiftScale->ClampOverflowOn();
  
  this->ComparisonFocalPlaneTexture = vtkSmartPointer<vtkTexture>::New();
  this->ComparisonFocalPlaneTexture->InterpolateOff();
  this->ComparisonFocalPlaneTexture->RepeatOff();
  this->ComparisonFocalPlaneTexture->SetLookupTable(table);
  this->ComparisonFocalPlaneTexture->
    SetInputConnection(this->ComparisonFocalPlaneImageShiftScale->GetOutputPort());

  this->ComparisonFocalPlaneSource = vtkSmartPointer<vtkPlaneSource>::New();
  this->ComparisonFocalPlaneSource->SetNormal(0.0, 0.0, 1.0);

  this->ComparisonFocalPlaneMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->ComparisonFocalPlaneMapper->SetInputConnection(this->ComparisonFocalPlaneSource->GetOutputPort());

  this->ComparisonFocalPlaneActor = vtkSmartPointer<vtkActor>::New();
  this->ComparisonFocalPlaneActor->SetMapper(this->ComparisonFocalPlaneMapper);
  this->ComparisonFocalPlaneActor->SetTexture(this->ComparisonFocalPlaneTexture);
  this->ComparisonFocalPlaneActor->SetUserMatrix(this->ShearTransformMatrix);
  this->ComparisonFocalPlaneActor->PickableOff();

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
    double mapsToZero = this->Simulation->GetMinimumIntensityLevel();
    double mapsToOne  = this->Simulation->GetMaximumIntensityLevel();

    bool simulatedImageVisible = this->Simulation->GetSuperimposeSimulatedImage();
    if (simulatedImageVisible) {
      this->SimulatedFocalPlaneImageShiftScale->SetShift(-mapsToZero);
      this->SimulatedFocalPlaneImageShiftScale->SetScale(255.0 / (mapsToOne - mapsToZero));

      FluorescenceImageSource* imageSource = 
        this->Simulation->GetFluorescenceImageSource();

      vtkDataObject* oldImage = this->SimulatedFocalPlaneImageShiftScale->GetInput();
      vtkImageData* fluorescenceImage = imageSource->GenerateFluorescenceImage();
      this->SimulatedFocalPlaneImageShiftScale->SetInput(fluorescenceImage);
      if (oldImage)
        oldImage->Delete();

      this->SimulatedFocalPlaneTexture->Modified();
                                                
      this->SimulatedFocalPlaneSource->SetPoint1(width, 0.0, 0.0);
      this->SimulatedFocalPlaneSource->SetPoint2(0.0, height, 0.0);
      this->SimulatedFocalPlaneActor->SetPosition(0.0, 0.0, depth);
    }
    this->SimulatedFocalPlaneActor->SetVisibility(simulatedImageVisible ? 1 : 0);


    bool comparisonImageVisible = this->Simulation->GetSuperimposeComparisonImage();
    if (comparisonImageVisible) {
      this->ComparisonFocalPlaneImageShiftScale->SetShift(-mapsToZero);
      this->ComparisonFocalPlaneImageShiftScale->SetScale(255.0 / (mapsToOne - mapsToZero));

      ImageModelObject* comparisonImageObject = this->Simulation->GetComparisonImageModelObject();
      if (comparisonImageObject) {
        vtkImageData* comparisonImage = comparisonImageObject->GetImageData();
        int extent[6];
        comparisonImage->GetExtent(extent);
        extent[4] = extent[5] = this->Simulation->GetFocalPlaneIndex();

        double comparisonImageWidth  = (extent[1]+1) * comparisonImageObject->
          GetProperty(ImageModelObject::X_SPACING_PROP)->GetDoubleValue();
        double comparisonImageHeight = (extent[3]+1) * comparisonImageObject->
          GetProperty(ImageModelObject::Y_SPACING_PROP)->GetDoubleValue();

        vtkImageClip* clipper = vtkImageClip::New();
        clipper->SetInput(comparisonImage);
        clipper->SetOutputWholeExtent(extent);

        this->ComparisonFocalPlaneImageShiftScale->SetInputConnection
          (clipper->GetOutputPort());
        this->ComparisonFocalPlaneSource->SetPoint1(comparisonImageWidth, 0.0, 0.0);
        this->ComparisonFocalPlaneSource->SetPoint2(0.0, comparisonImageHeight, 0.0);
        this->ComparisonFocalPlaneActor->SetPosition(0.0, 0.0, depth);
        this->ComparisonFocalPlaneActor->VisibilityOn();

      } else {
        this->ComparisonFocalPlaneImageShiftScale->SetInput(NULL);
        this->ComparisonFocalPlaneActor->VisibilityOff();
      }

    } else {
      this->ComparisonFocalPlaneActor->VisibilityOff();
    }

    this->FocalPlaneGrid->SetPoint1(0.0, height, 0.0);
    this->FocalPlaneGrid->SetPoint2(width, 0.0, 0.0);
    this->FocalPlaneGridActor->SetPosition(0.0, 0.0, depth);
    this->FocalPlaneGridActor->
      SetVisibility(this->Simulation->GetShowReferenceGrid() ? 1 : 0);
    
    double spacing = this->Simulation->GetReferenceGridSpacing();
    this->FocalPlaneGrid->SetSpacing(spacing, spacing);

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
  rv->GetRenderer()->AddActor(this->SimulatedFocalPlaneActor);
  rv->GetRenderer()->AddActor(this->ComparisonFocalPlaneActor);
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
  rv->GetRenderer()->RemoveActor(this->SimulatedFocalPlaneActor);
  rv->GetRenderer()->RemoveActor(this->ComparisonFocalPlaneActor);
  rv->GetRenderer()->RemoveActor(this->FocalPlaneGridActor);
  rv->GetRenderer()->RemoveActor(this->ImageVolumeOutlineActor);

  return true;
}


void
vtkFluorescenceWidgetsRepresentation
::PrintSelf(ostream& os, vtkIndent indent) {

}
