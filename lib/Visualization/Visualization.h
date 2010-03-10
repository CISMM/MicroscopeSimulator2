#ifndef _VISUALIZATION_H_
#define _VISUALIZATION_H_

#include <FluorescenceImageSource.h>

#include <vtkSmartPointer.h>
#include <vtkCommand.h>
#include <vtkSmartPointer.h>

// Forward declarations
class vtkAlgorithmOutput;
class vtkFluorescenceRenderView;
class vtkFluorescenceWidgetsRepresentation;
class vtkImageData;
class vtkInteractorStyleTrackballCamera;
class vtkInteractorStyleTrackballActor;
class vtkRenderer;
class vtkRenderView;
class vtkRenderWindow;
class vtkVisualizationInteractionObserver;

class FluorescenceRepresentation;
class GeometryRepresentation;
class ModelObject;
class Simulation;
class VisualizationInteractionObserver;
class PointSpreadFunction;


class Visualization : public FluorescenceImageSource {

public:
  Visualization();
  virtual ~Visualization();

  void SetSimulation(Simulation* simulation);

  vtkRenderWindow* GetModelObjectRenderWindow();
  vtkRenderWindow* GetFluorescenceRenderWindow();

  void ModelObjectViewRender();
  void FluorescenceViewRender();

  virtual vtkImageData* GenerateFluorescenceImage();
  virtual vtkImageData* GenerateFluorescenceStackImage();

  void ResetModelObjectCamera();
  void RefreshModelObjectView();

  void SetInteractionModeToCamera();
  void SetInteractionModeToActor();

  // Fluorescence-related stuff
  void GetFluorescenceScalarRange(double scalarRange[2]);

  // Set the PSF
  void SetPSF(PointSpreadFunction* psf);
  void SetPSFImage(vtkImageData* image);

  void FocusOnObject(ModelObject* object);

protected:
  Simulation* m_Simulation;

  GeometryRepresentation*     m_GeometryRepresentation;
  FluorescenceRepresentation* m_FluorescenceRepresentation;
  vtkSmartPointer<vtkFluorescenceWidgetsRepresentation> m_FluorescenceWidgetsRepresentation;

  // Model object window view
  vtkSmartPointer<vtkRenderView> m_ModelObjectRenderView;

  vtkSmartPointer<vtkInteractorStyleTrackballCamera>   m_CameraInteractor;
  vtkSmartPointer<vtkInteractorStyleTrackballActor>    m_ActorInteractor;
  vtkSmartPointer<vtkVisualizationInteractionObserver> m_InteractionObserver;

  // Fluorescence render view
  vtkSmartPointer<vtkFluorescenceRenderView> m_FluorescenceRenderView;

};

#endif // _VISUALIZATION_H_
