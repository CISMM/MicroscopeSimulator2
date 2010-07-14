#ifndef _VISUALIZATION_H_
#define _VISUALIZATION_H_

#include <vtkSmartPointer.h>
#include <vtkCommand.h>
#include <vtkSmartPointer.h>

// Forward declarations
class vtkAlgorithmOutput;
class vtkFluorescenceRenderView;
class vtkFluorescenceWidgetsRepresentation;
class vtkGeometryRenderView;
class vtkImageData;
class vtkImagePlaneWidgetRepresentation;
class vtkInteractorStyleTrackballCamera;
class vtkInteractorStyleTrackballActor;
class vtkRenderer;
class vtkRenderView;
class vtkRenderWindow;
class vtkVisualizationInteractionObserver;

class FluorescenceRepresentation;
class GeometryRepresentation;
class ModelObject;
class PointSpreadFunction;
class Simulation;
class VisualizationInteractionObserver;
class VisualizationFluorescenceImageSource;


class Visualization {

public:
  Visualization();
  virtual ~Visualization();

  void SetSimulation(Simulation* simulation);
  Simulation* GetSimulation();

  vtkRenderWindow* GetModelObjectRenderWindow();
  vtkRenderWindow* GetFluorescenceRenderWindow();

  void ModelObjectViewRender();
  void FluorescenceViewRender();

  void SetShowFluorophoresInModelObjectRenderer(bool flag);
  void SetShowOrientationWidgetInModelObjectRenderer(bool flag);

  virtual vtkImageData* GenerateFluorescenceImage();
  virtual vtkImageData* GenerateFluorescenceStackImage();

  void ComputeFluorescencePointsGradient();

  void ResetModelObjectCamera();
  void RefreshModelObjectView();

  void SetInteractionModeToCamera();
  void SetInteractionModeToActor();

  // Fluorescence-related stuff
  void GetFluorescenceScalarRange(double scalarRange[2]);

  void FocusOnObject(ModelObject* object);

  float* GetPointsGradientForModelObjectAtIndex(int i, int& numPoints);

protected:
  Simulation* m_Simulation;

  VisualizationFluorescenceImageSource* m_ImageSource;

  GeometryRepresentation*     m_GeometryRepresentation;
  FluorescenceRepresentation* m_FluorescenceRepresentation;
  vtkSmartPointer<vtkFluorescenceWidgetsRepresentation> m_FluorescenceWidgetsRepresentation;
  vtkSmartPointer<vtkImagePlaneWidgetRepresentation>    m_ImagePlaneWidgetRepresentation;

  // Model object window view
  //vtkSmartPointer<vtkRenderView> m_ModelObjectRenderView;
  vtkSmartPointer<vtkGeometryRenderView> m_ModelObjectRenderView;

  vtkSmartPointer<vtkInteractorStyleTrackballCamera>   m_CameraInteractor;
  vtkSmartPointer<vtkInteractorStyleTrackballActor>    m_ActorInteractor;
  vtkSmartPointer<vtkVisualizationInteractionObserver> m_InteractionObserver;

  bool ShowFluorophoresInModelObjectRenderer;

  // Fluorescence render view
  vtkSmartPointer<vtkFluorescenceRenderView> m_FluorescenceRenderView;

};

#endif // _VISUALIZATION_H_
