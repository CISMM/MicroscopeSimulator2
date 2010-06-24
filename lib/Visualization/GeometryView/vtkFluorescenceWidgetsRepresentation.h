#ifndef _VTK_FLUORESCENCE_WIDGETS_REPRESENTATION_H_
#define _VTK_FLUORESCENCE_WIDGETS_REPRESENTATION_H_

#include <FluorescenceSimulation.h>

#include <vtkRenderedRepresentation.h>
#include <vtkSmartPointer.h>

// Forward declarations
class vtkActor;
class vtkImageShiftScale;
class vtkMatrix4x4;
class vtkOutlineSource;
class vtkPlaneSource;
class vtkPolyDataMapper;
class vtkReferenceGridSource;
class vtkRenderView;
class vtkTexture;


class vtkFluorescenceWidgetsRepresentation : public vtkRenderedRepresentation {
 public:
  static vtkFluorescenceWidgetsRepresentation *New();
  vtkTypeRevisionMacro(vtkFluorescenceWidgetsRepresentation, vtkRenderedRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Sets the FluorescenceSimulation
  void SetFluorescenceSimulation(FluorescenceSimulation* simulation);

  int RequestData(vtkInformation* request, vtkInformationVector** inputVector,
                  vtkInformationVector* outputVector);

 protected:
  vtkFluorescenceWidgetsRepresentation();
  virtual ~vtkFluorescenceWidgetsRepresentation();

  FluorescenceSimulation* Simulation;

  vtkSmartPointer<vtkMatrix4x4>               ShearTransformMatrix;

  vtkSmartPointer<vtkImageShiftScale>         SimulatedFocalPlaneImageShiftScale;
  vtkSmartPointer<vtkTexture>                 SimulatedFocalPlaneTexture;
  vtkSmartPointer<vtkPlaneSource>             SimulatedFocalPlaneSource;
  vtkSmartPointer<vtkPolyDataMapper>          SimulatedFocalPlaneMapper;
  vtkSmartPointer<vtkActor>                   SimulatedFocalPlaneActor;

  vtkSmartPointer<vtkImageShiftScale>         ComparisonFocalPlaneImageShiftScale;
  vtkSmartPointer<vtkTexture>                 ComparisonFocalPlaneTexture;
  vtkSmartPointer<vtkPlaneSource>             ComparisonFocalPlaneSource;
  vtkSmartPointer<vtkPolyDataMapper>          ComparisonFocalPlaneMapper;
  vtkSmartPointer<vtkActor>                   ComparisonFocalPlaneActor;

  vtkSmartPointer<vtkReferenceGridSource>     FocalPlaneGrid;
  vtkSmartPointer<vtkPolyDataMapper>          FocalPlaneGridMapper;
  vtkSmartPointer<vtkActor>                   FocalPlaneGridActor;

  vtkSmartPointer<vtkOutlineSource>           ImageVolumeOutlineSource;
  vtkSmartPointer<vtkPolyDataMapper>          ImageVolumeOutlineMapper;
  vtkSmartPointer<vtkActor>                   ImageVolumeOutlineActor;

  virtual void PrepareForRendering(vtkRenderView* view);

  virtual bool AddToView(vtkView* view);
  virtual bool RemoveFromView(vtkView* view);


 private:
  vtkFluorescenceWidgetsRepresentation(const vtkFluorescenceWidgetsRepresentation&);
  void operator=(const vtkFluorescenceWidgetsRepresentation&);

};

#endif // _VTK_FLUORESCENCE_WIDGETS_REPRESENTATION_H_
