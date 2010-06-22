#include "Visualization.h"

#include <ModelObjectList.h>
#include <CylinderModelObject.h>
#include <DiskModelObject.h>
#include <FlexibleTubeModelObject.h>
#include <PlaneModelObject.h>
#include <PointSetModelObject.h>
#include <SphereModelObject.h>
#include <TorusModelObject.h>

#include <FluorescenceRepresentation.h>
#include <GeometryRepresentation.h>
#include <PointSpreadFunction.h>
#include <Simulation.h>
#include <VisualizationFluorescenceImageSource.h>

#include <vtkActor.h>
#include <vtkAbstractPicker.h>
#include <vtkAlgorithmOutput.h>
#include <vtkCamera.h>
#include <vtkFluorescenceRenderView.h>
#include <vtkImageAppend.h>
#include <vtkImageData.h>
#include <vtkImagePlaneWidgetRepresentation.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkInteractorStyleTrackballActor.h>
#include <vtkFluorescenceWidgetsRepresentation.h>
#include <vtkModelObjectGeometryRepresentation.h>
#include <vtkModelObjectFluorescenceRepresentation.h>
#include <vtkRenderer.h>
//#include <vtkRenderView.h>
#include <vtkGeometryRenderView.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderedSurfaceRepresentation.h>
#include <vtkSurfaceUniformPointSampler.h>
#include <vtkTriangleFilter.h>
#include <vtkVisualizationInteractionObserver.h>

//#define IMAGE_PLANE_WIDGETS


Visualization
::Visualization() {
  m_Simulation = NULL;
  m_ImageSource = new VisualizationFluorescenceImageSource();
  m_ImageSource->SetVisualization(this);

  m_GeometryRepresentation = new GeometryRepresentation();
  m_FluorescenceRepresentation = new FluorescenceRepresentation();
  m_FluorescenceWidgetsRepresentation = vtkSmartPointer<vtkFluorescenceWidgetsRepresentation>::New();
#ifdef IMAGE_PLANE_WIDGETS
  m_ImagePlaneWidgetRepresentation = vtkSmartPointer<vtkImagePlaneWidgetRepresentation>::New();
#endif

  // Set up the model object view.
  //m_ModelObjectRenderView = vtkSmartPointer<vtkRenderView>::New();
  m_ModelObjectRenderView = vtkSmartPointer<vtkGeometryRenderView>::New();
  m_ModelObjectRenderView->GetRenderer()->SetBackground(0.5, 0.5, 1.0);

  m_InteractionObserver = vtkSmartPointer<vtkVisualizationInteractionObserver>::New();

  m_CameraInteractor = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
  m_ActorInteractor = vtkSmartPointer<vtkInteractorStyleTrackballActor>::New();
  m_ActorInteractor->AddObserver(vtkCommand::InteractionEvent, m_InteractionObserver);
  m_GeometryRepresentation->SetInteractionObserver(m_InteractionObserver);

  // Use the camera interactor initially.
  m_ModelObjectRenderView->SetInteractorStyle(m_CameraInteractor);

  // Set up the fluorescence render view.
  m_FluorescenceRenderView = vtkSmartPointer<vtkFluorescenceRenderView>::New();
}


Visualization
::~Visualization() {
  delete m_ImageSource;
  delete m_GeometryRepresentation;
}


void
Visualization
::SetSimulation(Simulation* simulation) {
  m_Simulation = simulation;

  m_ModelObjectRenderView->SetSimulation(simulation);

  m_Simulation->GetFluorescenceSimulation()->
    SetFluorescenceImageSource(m_ImageSource);

  m_GeometryRepresentation->SetModelObjectList(simulation->GetModelObjectList());
  m_FluorescenceRepresentation->SetModelObjectList(simulation->GetModelObjectList());
  m_FluorescenceWidgetsRepresentation->SetFluorescenceSimulation(simulation->GetFluorescenceSimulation());
  m_FluorescenceRenderView->SetSimulation(simulation);
}


Simulation*
Visualization
::GetSimulation() {
  return m_Simulation;
}


vtkRenderWindow*
Visualization
::GetModelObjectRenderWindow() {
  return m_ModelObjectRenderView->GetRenderWindow();
}


vtkRenderWindow*
Visualization
::GetFluorescenceRenderWindow() {
  return m_FluorescenceRenderView->GetRenderWindow();
}


void
Visualization
::ModelObjectViewRender() {
  m_ModelObjectRenderView->Render();
}


void
Visualization
::FluorescenceViewRender() {
  m_FluorescenceRenderView->Render();
}


void
Visualization
::SetShowFluorophoresInModelObjectRenderer(bool flag) {
  m_GeometryRepresentation->SetShowFluorophores(flag);
}


vtkImageData*
Visualization
::GenerateFluorescenceImage() {
  FluorescenceViewRender();

  vtkImageData* imageData = vtkImageData::New();
  imageData->DeepCopy(m_FluorescenceRenderView->GetImage());

  return imageData;
}


vtkImageData*
Visualization
::GenerateFluorescenceStackImage() {
  FluorescenceSimulation* fluoroSim = m_Simulation->GetFluorescenceSimulation();
  if (!fluoroSim)
    return NULL;

  vtkSmartPointer<vtkImageAppend> appender = vtkSmartPointer<vtkImageAppend>::New();
  appender->SetAppendAxis(2);

  for (unsigned int i = 0; i < fluoroSim->GetNumberOfFocalPlanes(); i++) {
    fluoroSim->SetFocalPlaneIndex(i);

    vtkImageData* image = GenerateFluorescenceImage();
    appender->AddInput(image);
    image->Delete();
  }

  appender->Update();

  vtkImageData* stackImage = vtkImageData::New();
  stackImage->ShallowCopy(appender->GetOutput());

  return stackImage;
}


void
Visualization
::ComputeFluorescencePointsGradient() {
  FluorescenceSimulation* fluoroSim = m_Simulation->GetFluorescenceSimulation();
  if (!fluoroSim)
    return;

  m_FluorescenceRenderView->ComputePointGradientsOn();

  // We just have to zip through the stack. The 
  // vtkFluorescencePointsGradientPolyDataMapper will take care of the gradient
  // computation.
  bool firstRender = true;
  for (unsigned int i = 0; i < fluoroSim->GetNumberOfFocalPlanes(); i++) {
    if (firstRender) {
      // Make sure we clear the point gradient mapper prior to the first render
      m_FluorescenceRenderView->ClearPointsGradientBuffersOn();
      firstRender = false;
    } else {
      m_FluorescenceRenderView->ClearPointsGradientBuffersOff();
    }

    fluoroSim->SetFocalPlaneIndex(i);
    FluorescenceViewRender();
  }

  m_FluorescenceRenderView->ComputePointGradientsOff();
}


void
Visualization
::ResetModelObjectCamera() {
  vtkCamera* camera = m_ModelObjectRenderView->GetRenderer()->GetActiveCamera();
  camera->SetPosition(0.0, 0.0, 1.0);
  camera->SetFocalPoint(0.0, 0.0, 0.0);
  camera->SetViewUp(0.0, 1.0, 0.0);
  camera->ComputeViewPlaneNormal();
  m_ModelObjectRenderView->ResetCamera();
}


void
Visualization
::RefreshModelObjectView() {
  m_ModelObjectRenderView->RemoveAllRepresentations();
  m_GeometryRepresentation->Update();
  m_GeometryRepresentation->AddToView(m_ModelObjectRenderView);
  m_ModelObjectRenderView->AddRepresentation(m_FluorescenceWidgetsRepresentation);
#ifdef IMAGE_PLANE_WIDGETS
  m_ModelObjectRenderView->AddRepresentation(m_ImagePlaneWidgetRepresentation);

  if (m_Simulation && m_Simulation->GetComparisonImageModelObject()) {
    m_ImagePlaneWidgetRepresentation->
      SetInput(m_Simulation->GetComparisonImageModelObject()->GetImageData());
  }
#endif

  m_FluorescenceRenderView->RemoveAllRepresentations();
  m_FluorescenceRepresentation->Update();
  m_FluorescenceRepresentation->AddToView(m_FluorescenceRenderView);
}


void
Visualization
::SetInteractionModeToCamera() {
  m_ModelObjectRenderView->SetInteractorStyle(m_CameraInteractor);  
}


void
Visualization
::SetInteractionModeToActor() {
  m_ModelObjectRenderView->SetInteractorStyle(m_ActorInteractor);
}


void
Visualization
::GetFluorescenceScalarRange(double scalarRange[2]) {
  vtkImageData* image = m_FluorescenceRenderView->GetImage();
  image->Update();

  int components = image->GetNumberOfScalarComponents();
  double min = 1e9;
  double max = -1e9;
  float* scalars = static_cast<float*>(image->GetScalarPointer());
  for (int i = 0; i < image->GetNumberOfPoints(); i++) {
    for (int j = 0; j < components; j++) {
      float scalar = scalars[i*components + j];
      if (scalar < min) min = scalar;
      if (scalar > max) max = scalar;
    }
  }
  
  scalarRange[0] = static_cast<double>(min);
  scalarRange[1] = static_cast<double>(max);
}


void
Visualization
::FocusOnObject(ModelObject* object) {
  vtkPolyDataAlgorithm* geometrySource = object->GetAllGeometryTransformed();
  geometrySource->GetOutput()->Update();

  double* bounds = geometrySource->GetOutput()->GetBounds();
  if (bounds) {
    m_ModelObjectRenderView->GetRenderer()->ResetCamera(bounds);
    m_ModelObjectRenderView->GetRenderer()->ResetCameraClippingRange();
    m_ModelObjectRenderView->Render();
  }
}


float*
Visualization
::GetPointsGradientForModelObjectAtIndex(int i, int& numPoints) {
  return m_FluorescenceRepresentation->GetPointsGradientForRepresentation(i, numPoints);
}
