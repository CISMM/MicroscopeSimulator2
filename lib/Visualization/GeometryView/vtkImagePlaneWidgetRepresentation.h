#ifndef _VTK_IMAGE_PLANE_WIDGET_REPRESENTATION_H_
#define _VTK_IMAGE_PLANE_WIDGET_REPRESENTATION_H_

#include <vtkRenderedRepresentation.h>
#include <vtkSmartPointer.h>

// Forward declarations
class vtkImagePlaneWidget;
class vtkAbstractPicker;
class Simulation;


class vtkImagePlaneWidgetRepresentation : public vtkRenderedRepresentation {
 public:
  static vtkImagePlaneWidgetRepresentation *New();
  vtkTypeMacro(vtkImagePlaneWidgetRepresentation, vtkRenderedRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

 protected:
  vtkImagePlaneWidgetRepresentation();
  virtual ~vtkImagePlaneWidgetRepresentation();

  vtkSmartPointer<vtkAbstractPicker> Picker;

  vtkSmartPointer<vtkImagePlaneWidget> XWidget;
  vtkSmartPointer<vtkImagePlaneWidget> YWidget;
  vtkSmartPointer<vtkImagePlaneWidget> ZWidget;

  virtual void PrepareForRendering(vtkRenderView* view);

  virtual bool AddToView(vtkView* view);
  virtual bool RemoveFromView(vtkView* view);

 private:
  vtkImagePlaneWidgetRepresentation(const vtkImagePlaneWidgetRepresentation&);
  void operator=(const vtkImagePlaneWidgetRepresentation&);

};


#endif // _VTK_IMAGE_PLANE_WIDGET_REPRESENTATION_H_
