#ifndef _VTK_FLUORESCENCE_WIDGETS_REPRESENTATION_H_
#define _VTK_FLUORESCENCE_WIDGETS_REPRESENTATION_H_

#include <FluorescenceSimulation.h>

#include <vtkRenderedRepresentation.h>
#include <vtkSmartPointer.h>

// Forward declarations
class vtkActor;
class vtkOutlineSource;
class vtkPolyDataMapper;
class vtkReferenceGridSource;
class vtkRenderView;


class vtkFluorescenceWidgetsRepresentation : public vtkRenderedRepresentation {
 public:
  static vtkFluorescenceWidgetsRepresentation *New();
  vtkTypeRevisionMacro(vtkFluorescenceWidgetsRepresentation, vtkRenderedRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Sets the FluorescenceSimulation
  void SetFluorescenceSimulation(FluorescenceSimulation* simulation);


 protected:
  vtkFluorescenceWidgetsRepresentation();
  virtual ~vtkFluorescenceWidgetsRepresentation();

  FluorescenceSimulation* Simulation;

  vtkSmartPointer<vtkReferenceGridSource> FocalPlaneGrid;
  vtkSmartPointer<vtkPolyDataMapper>      FocalPlaneGridMapper;
  vtkSmartPointer<vtkActor>               FocalPlaneGridActor;

  vtkSmartPointer<vtkOutlineSource>  ImageVolumeOutlineSource;
  vtkSmartPointer<vtkPolyDataMapper> ImageVolumeOutlineMapper;
  vtkSmartPointer<vtkActor>          ImageVolumeOutlineActor;

  virtual void PrepareForRendering(vtkRenderView* view);

  virtual bool AddToView(vtkView* view);
  virtual bool RemoveFromView(vtkView* view);


 private:
  vtkFluorescenceWidgetsRepresentation(const vtkFluorescenceWidgetsRepresentation&);
  void operator=(const vtkFluorescenceWidgetsRepresentation&);

};

#endif // _VTK_FLUORESCENCE_WIDGETS_REPRESENTATION_H_
