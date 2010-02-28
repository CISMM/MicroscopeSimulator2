#include <vtkModelObjectFluorescenceRepresentation.h>

#include <vtkActor.h>
#include <vtkBlendingFluorescencePolyDataMapper.h>
#include <vtkGatherFluorescencePolyDataMapper.h>
#include <vtkFluorescenceRenderView.h>
#include <vtkFluorescenceRenderer.h>
#include <vtkOpenGL3DTexture.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkPolyDataToTetrahedralGrid.h>
#include <vtkProperty.h>
#include <vtkRenderView.h>
#include <vtkSurfaceUniformPointSampler.h>
#include <vtkTriangleFilter.h>
#include <vtkUniformPointSampler.h>
#include <vtkVolumeUniformPointSampler.h>

#include <ModelObject.h>

vtkCxxRevisionMacro(vtkModelObjectFluorescenceRepresentation, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkModelObjectFluorescenceRepresentation);


vtkModelObjectFluorescenceRepresentation::vtkModelObjectFluorescenceRepresentation() {
  this->ModelObject = NULL;
  this->FluorophoreProperty = NULL;
  this->MapperType = GATHER_MAPPER;

  this->Sampler = NULL;

  this->GatherMapper = vtkSmartPointer<vtkGatherFluorescencePolyDataMapper>::New();
  this->GatherMapper->ImmediateModeRenderingOn();
  this->GatherMapper->SetPointsPerPass(3000);
  this->GatherMapper->SetPixelSize(65.0, 65.0);

  this->GatherProperty = vtkSmartPointer<vtkProperty>::New();
  this->GatherProperty->LoadMaterial("lib/shaders/gather.xml");
  this->GatherProperty->ShadingOn();
  this->GatherProperty->SetColor(0.0, 1.0, 0.0);
  this->GatherProperty->AddShaderVariable("startIndex", 0);
  this->GatherProperty->AddShaderVariable("endIndex", 0);
  this->GatherProperty->AddShaderVariable("pointTexDim", 0);
  this->GatherProperty->AddShaderVariable("focalDepth", 0.0);
  this->GatherProperty->AddShaderVariable("psfSampler", 0);
  this->GatherProperty->AddShaderVariable("ptsSampler", 1);
  this->GatherProperty->AddShaderVariable("exposure", 1.0);

  this->BlendingMapper = vtkSmartPointer<vtkBlendingFluorescencePolyDataMapper>::New();
  // TODO - finish setting up this property

  this->Actor = vtkSmartPointer<vtkActor>::New();

  // Use gather mapper by default.
  this->Actor->SetMapper(this->GatherMapper);
  this->Actor->SetProperty(this->GatherProperty);
}


vtkModelObjectFluorescenceRepresentation::~vtkModelObjectFluorescenceRepresentation() {
  
}


void vtkModelObjectFluorescenceRepresentation::SetModelObject(ModelObjectPtr mo) {
  this->ModelObject = mo;
  this->UpdateRepresentation();
}


ModelObjectPtr vtkModelObjectFluorescenceRepresentation::GetModelObject() {
  return this->ModelObject;
}


void vtkModelObjectFluorescenceRepresentation
::SetFluorophoreModelObjectProperty(FluorophoreModelObjectProperty* property) {
  this->FluorophoreProperty = property;
  vtkPolyDataAlgorithm* geometry = property->GetGeometry();

  FluorophoreModelType fluorType = property->GetFluorophoreModelType();
  if (fluorType == GEOMETRY_VERTICES) {

    geometry->GetOutput()->Update();
    std::cout << "Geometry points: " << geometry->GetOutput()->GetNumberOfPoints() << std::endl;
    this->SetInputConnection(geometry->GetOutputPort());
 
  } else if (fluorType == UNIFORM_RANDOM_SURFACE_SAMPLE) {

    vtkSmartPointer<vtkTriangleFilter> triangulizer = vtkSmartPointer<vtkTriangleFilter>::New();
    triangulizer->PassLinesOff();
    triangulizer->PassVertsOff();
    triangulizer->SetInputConnection(geometry->GetOutputPort());
    
    vtkSurfaceUniformPointSampler* surfaceSampler = vtkSurfaceUniformPointSampler::New();
    surfaceSampler->SetInputConnection(triangulizer->GetOutputPort());
    surfaceSampler->Update();
    surfaceSampler->GetOutput()->Update();
    std::cout << "Surface point samples: " << surfaceSampler->GetOutput()->GetNumberOfPoints()
              << std::endl;
    this->Sampler = surfaceSampler;

    this->SetInputConnection(this->Sampler->GetOutputPort());

  } else if (fluorType == UNIFORM_RANDOM_VOLUME_SAMPLE) {

    vtkSmartPointer<vtkPolyDataToTetrahedralGrid> tetrahedralizer =
      vtkSmartPointer<vtkPolyDataToTetrahedralGrid>::New();
    tetrahedralizer->SetInputConnection(geometry->GetOutputPort());

    vtkVolumeUniformPointSampler* volumeSampler = vtkVolumeUniformPointSampler::New();
    volumeSampler->SetInputConnection(tetrahedralizer->GetOutputPort());
    std::cout << "Volume point samples: " << volumeSampler->GetOutput()->GetNumberOfPoints()
              << std::endl;
    this->Sampler = volumeSampler;

    this->SetInputConnection(this->Sampler->GetOutputPort());
  }
}


FluorophoreModelObjectProperty*
vtkModelObjectFluorescenceRepresentation
::GetFluorophoreModelObjectProperty() {
  return this->FluorophoreProperty;
}


void vtkModelObjectFluorescenceRepresentation::UseGatherMapper() {
  this->MapperType = GATHER_MAPPER;
  this->Actor->SetMapper(this->GatherMapper);
  this->Actor->SetProperty(this->GatherProperty);
}


void vtkModelObjectFluorescenceRepresentation::UseBlendingMapper() {
  this->MapperType = BLENDING_MAPPER;
  this->Actor->SetMapper(this->BlendingMapper);
  this->Actor->SetProperty(this->BlendingProperty);
}


vtkModelObjectFluorescenceRepresentation::Mapper_t vtkModelObjectFluorescenceRepresentation::GetMapperType() {
  return this->MapperType;
}


vtkActor* vtkModelObjectFluorescenceRepresentation::GetActor() {
  return this->Actor;
}


int vtkModelObjectFluorescenceRepresentation
::RequestData(vtkInformation* request,
              vtkInformationVector** inputVector,
              vtkInformationVector* outputVector) {
  this->GatherMapper->SetInputConnection(0, this->GetInternalOutputPort());
  this->BlendingMapper->SetInputConnection(0, this->GetInternalOutputPort());
  
  return 1;
}


void vtkModelObjectFluorescenceRepresentation::PrepareForRendering(vtkView* view) {
  this->UpdateRepresentation();
}


void vtkModelObjectFluorescenceRepresentation::UpdateRepresentation() {
  if (!this->ModelObject || !this->FluorophoreProperty)
    return;

  FluorophoreChannelType channel = 
    this->FluorophoreProperty->GetFluorophoreChannel();
  if (channel == RED_CHANNEL) {
    this->Actor->GetProperty()->SetColor(1.0, 0.0, 0.0);
  } else if (channel == GREEN_CHANNEL) {
    this->Actor->GetProperty()->SetColor(0.0, 1.0, 0.0);
  } else if (channel == BLUE_CHANNEL) {
    this->Actor->GetProperty()->SetColor(0.0, 0.0, 1.0);
  } else if (channel == ALL_CHANNELS) {
    this->Actor->GetProperty()->SetColor(1.0, 1.0, 1.0);
  }

  double divisor = 1.0;
  if (this->FluorophoreProperty->GetFluorophoreModelType() == UNIFORM_RANDOM_SURFACE_SAMPLE) {
    divisor = 1.0e6;
  } else if (this->FluorophoreProperty->GetFluorophoreModelType() == UNIFORM_RANDOM_VOLUME_SAMPLE) {
    divisor = 1.0e9;
  }
  if (this->Sampler) {
    this->Sampler->SetDensity(this->FluorophoreProperty->GetDensity() / divisor);
    this->Sampler->GetOutput()->Update();
    std::cout << "Density: " << this->FluorophoreProperty->GetDensity() / divisor << std::endl;
  }
    
  bool visible = this->ModelObject->GetVisible() && 
    this->FluorophoreProperty->GetEnabled();
  this->Actor->SetVisibility(visible ? 1 : 0);

  double position[3];
  this->ModelObject->GetPosition(position);
  this->SetPosition(position);

  double rotation[4];
  this->ModelObject->GetRotation(rotation);
  this->SetRotationWXYZ(rotation);
}


void vtkModelObjectFluorescenceRepresentation::SetPosition(double position[3]) {
  this->Actor->SetPosition(position);
}


void vtkModelObjectFluorescenceRepresentation::SetRotationWXYZ(double rotation[4]) {
  this->Actor->SetOrientation(0.0, 0.0, 0.0);
  this->Actor->RotateWXYZ(rotation[0], rotation[1], rotation[2], rotation[3]);
}


bool vtkModelObjectFluorescenceRepresentation::AddToView(vtkView* view) {
  vtkFluorescenceRenderView* rv = vtkFluorescenceRenderView::SafeDownCast(view);
  if (!rv) {
    vtkErrorMacro("Can only add to a subclass of vtkFluorescenceRenderView.");
    return false;
  }
  rv->GetRenderer()->AddActor(this->Actor);
  return true;
}


bool vtkModelObjectFluorescenceRepresentation::RemoveFromView(vtkView* view) {
  vtkFluorescenceRenderView* rv = vtkFluorescenceRenderView::SafeDownCast(view);
  if (!rv) {
    return false;
  }
  rv->GetRenderer()->RemoveActor(this->Actor);
  return true;
}


void vtkModelObjectFluorescenceRepresentation::PrintSelf(ostream& os, vtkIndent indent) {
  
}

