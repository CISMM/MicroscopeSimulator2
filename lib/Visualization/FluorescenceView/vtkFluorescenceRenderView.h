#ifndef _VTK_FLUORESCENCE_RENDER_VIEW_
#define _VTK_FLUORESCENCE_RENDER_VIEW_


#include <vtkView.h>
#include <vtkSmartPointer.h>


class vtkAlgorithmOutput;
class vtkFluorescencePointsGradientRenderer;
class vtkFramebufferObjectTexture;
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
class Simulation;

// .NAME vtkFluorescenceRenderView - A fluorescent view of objects.
//
// .SECTION Description
// This class is used as a view for fluorescent objects.

class vtkFluorescenceRenderView : public vtkView {
 public:
  static vtkFluorescenceRenderView* New();
  vtkTypeMacro(vtkFluorescenceRenderView, vtkView);
  void PrintSelf(ostream& os, vtkIndent indent);

  void SetBlendingTo16Bit();
  void SetBlendingTo32Bit();
  
  void SetSimulation(Simulation* simulation);

  // Description:
  // Gets the fluorescence renderer for this view.
  vtkGetObjectMacro(Renderer, vtkFluorescenceRenderer);

  // Description:
  // Gets the points gradient renderer for this view.
  vtkGetObjectMacro(GradientRenderer, vtkFluorescencePointsGradientRenderer);
  
  // Description:
  // Get a handle to the render window.
  vtkGetObjectMacro(RenderWindow, vtkRenderWindow);

  // Description:
  // Enable/disable computing gradients
  vtkSetMacro(ComputePointGradients,int);
  vtkGetMacro(ComputePointGradients,int);
  vtkBooleanMacro(ComputePointGradients,int);

  // Description:
  // Enable/disable clearing of the gradient buffer prior to rendering
  vtkSetMacro(ClearPointsGradientBuffers,int);
  vtkGetMacro(ClearPointsGradientBuffers,int);
  vtkBooleanMacro(ClearPointsGradientBuffers,int);

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

  Simulation*             Sim;
  FluorescenceSimulation* FluoroSim;

  // Holds the single-channel PSF texture.
  vtkSmartPointer<vtkOpenGL3DTexture> PSFTexture;

  // Holds the 3-component gradient of the PSF
  vtkSmartPointer<vtkOpenGL3DTexture> PSFGradientTexture;

  // Holds the single-channel comparison image.
  vtkSmartPointer<vtkOpenGL3DTexture> ExperimentalImageTexture;

  // If true, the render view will compute point gradients
  int ComputePointGradients;

  // Description:
  // Flag that indicates whether the gradient mappers should clear their
  // gradient buffer prior to rendering.
  int ClearPointsGradientBuffers;

  // Description:
  // Called by the view when the renderer is about to render.
  virtual void PrepareForRendering();
  
  vtkFluorescenceRenderer*               Renderer;
  vtkFluorescencePointsGradientRenderer* GradientRenderer;
  vtkRenderWindow*                       RenderWindow;

  vtkFramebufferObjectTexture*           SyntheticImageTexture;

private:
  vtkFluorescenceRenderView(const vtkFluorescenceRenderView&);  // Not implemented.
  void operator=(const vtkFluorescenceRenderView&);  // Not implemented.

};

#endif // _VTK_FLUORESCENCE_RENDER_VIEW_
