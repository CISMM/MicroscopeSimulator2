#ifndef _VTK_GEOMETRY_RENDER_VIEW_
#define _VTK_GEOMETRY_RENDER_VIEW_


#include <vtkRenderView.h>
#include <vtkSmartPointer.h>

class AFMSimulation;
class Simulation;

class vtkActor;
class vtkAlgorithmOutput;
class vtkFramebufferObjectTexture;
class vtkFramebufferObjectRenderer;
class vtkImageData;
class vtkInteractorObserver;
class vtkOpenGL3DTexture;
class vtkOrientationMarkerWidget;
class vtkPlaneSource;
class vtkPolyDataMapper;
class vtkRenderer;
class vtkRenderWindow;
class vtkRenderWindowInteractor;
class vtkTexture;
class vtkTexturedActor2D;


// .NAME vtkGeometryRenderView - A solid view of objects.
//
// .SECTION Description
// This class is used as a view for model objects.

class vtkGeometryRenderView : public vtkRenderView {
 public:
  static vtkGeometryRenderView* New();
  vtkTypeRevisionMacro(vtkGeometryRenderView, vtkRenderView);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/get the Simulation.
  void SetSimulation(Simulation* simulation);
  Simulation* GetSimulation();

  void SetShowOrientationWidget(bool show);

  // Description:
  // Gets a floating-point image containing the contents of the fluorescence
  // render window.
  //virtual vtkImageData* GetImage();
  //virtual vtkAlgorithmOutput* GetImageOutputPort();
  
protected:
  vtkGeometryRenderView();
  ~vtkGeometryRenderView();

  AFMSimulation* AFMSim;
  Simulation*    Sim;

  vtkSmartPointer<vtkOrientationMarkerWidget> OrientationWidget;

  vtkSmartPointer<vtkFramebufferObjectTexture>  AFMTexture;
  vtkSmartPointer<vtkFramebufferObjectRenderer> AFMRenderer;

  vtkSmartPointer<vtkPlaneSource>    AFMPlaneSource;
  vtkSmartPointer<vtkPolyDataMapper> AFMPlaneMapper;
  vtkSmartPointer<vtkActor>          AFMPlaneActor;

  virtual void PrepareForRendering();

private:
  vtkGeometryRenderView(const vtkGeometryRenderView&);  // Not implemented.
  void operator=(const vtkGeometryRenderView&);  // Not implemented.

};

#endif // _VTK_GEOMETRY_RENDER_VIEW_
