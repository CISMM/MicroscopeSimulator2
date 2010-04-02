#ifndef _VTK_FLUORESCENCE_RENDER_VIEW_
#define _VTK_FLUORESCENCE_RENDER_VIEW_


#include <vtkView.h>
#include <vtkSmartPointer.h>


class vtkAlgorithmOutput;
class vtkImageData;
class vtkInteractorObserver;
class vtkOpenGL3DTexture;
class vtkRenderWindow;
class vtkRenderWindowInteractor;
class vtkTexture;
class vtkTexturedActor2D;
class vtkTransformCoordinateSystems;
class vtkFluorescenceRenderer;

class FluorescenceSimulation;

// .NAME vtkFluorescenceRenderView - A fluorescent view of objects.
//
// .SECTION Description
// This class is used as a view for fluorescent objects.

class vtkFluorescenceRenderView : public vtkView {
 public:
  static vtkFluorescenceRenderView* New();
  vtkTypeRevisionMacro(vtkFluorescenceRenderView, vtkView);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  void SetFluorescenceSimulation(FluorescenceSimulation* simulation);

  // Description:
  // Gets the renderer for this view.
  vtkGetObjectMacro(Renderer, vtkFluorescenceRenderer);
  
  // Description:
  // Get a handle to the render window.
  vtkGetObjectMacro(RenderWindow, vtkRenderWindow);

  // Description:
  // Updates the representations, then calls Render() on the render window
  // associated with this view.
  virtual void Render();
  
  // Description:
  // Updates the representations, then calls ResetCamera() on the renderer
  // associated with this view.
  virtual void ResetCamera();
  
  // Description:
  // Updates the representations, then calls ResetCameraClippingRange() on the renderer
  // associated with this view.
  virtual void ResetCameraClippingRange();

  // Description:
  // Gets a floating-point image containing the contents of the fluorescence
  // render window.
  virtual vtkImageData* GetImage();
  virtual vtkAlgorithmOutput* GetImageOutputPort();
  
protected:
  vtkFluorescenceRenderView();
  ~vtkFluorescenceRenderView();

  FluorescenceSimulation* Simulation;

  vtkImageData* PSFImage;

  vtkSmartPointer<vtkOpenGL3DTexture> PSFTexture;

  // Description:
  // Called by the view when the renderer is about to render.
  virtual void PrepareForRendering();
  
  vtkFluorescenceRenderer* Renderer;
  vtkRenderWindow*         RenderWindow;

private:
  vtkFluorescenceRenderView(const vtkFluorescenceRenderView&);  // Not implemented.
  void operator=(const vtkFluorescenceRenderView&);  // Not implemented.

};

#endif // _VTK_FLUORESCENCE_RENDER_VIEW_
