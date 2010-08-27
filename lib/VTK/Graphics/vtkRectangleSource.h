// .NAME vtkRectangleSource.h - generate a rectangle centered at the origin
// .SECTION Description
// vtkRectangleSource.h creates a vtkPolyData in the shape of a rectangle centered at the
// origin. The polydata consists of triangle cells. 
//
// The width and height of the rectangle can be specified, as well as the resolution
// of the tesselation in each dimension.
//
// The output unstructured grid can optionally generate scalar data
// consisting of the fields:
// "u" - normalized horizontal position coordinate on the rectangle (range [0.0, 1.0])
// "v" - normalized vertical position coordinate on the rectangle (range [0.0, 1.0])


#ifndef __vtkRectangleSource_h
#define __vtkRectangleSource_h

#include "vtkPolyDataAlgorithm.h"

#include "vtkCell.h" // Needed for VTK_CELL_SIZE

class vtkRectangleSource : public vtkPolyDataAlgorithm 
{
public:
  static vtkRectangleSource *New();
  vtkTypeRevisionMacro(vtkRectangleSource,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the width of the plane. Initial value is 1.0.
  vtkSetMacro(Width,double);
  vtkGetMacro(Width,double);

  // Description:
  // Set the height of the plane. Initial value is 1.0.
  vtkSetMacro(Height,double);
  vtkGetMacro(Height,double);

  // Description:
  // Set the horizontal tesselation resolution of the rectangle. Initial value is 1.
  vtkSetClampMacro(WidthResolution,int,1,VTK_INT_MAX)
  vtkGetMacro(WidthResolution,int);

  // Description:
  // Set the vertical tesselation resolution of the rectangle. Initial value is 1.
  vtkSetClampMacro(HeightResolution,int,1,VTK_INT_MAX)
  vtkGetMacro(HeightResolution,int);

  // Description:
  // Turn on/off generation of scalar data. Initial value is true.
  vtkSetMacro(GenerateScalars,int);
  vtkGetMacro(GenerateScalars,int);
  vtkBooleanMacro(GenerateScalars,int);

  // Description:
  // Calculate the location of a point given shape parameters.
  void ComputePoint(double u, double v, double results[3]);

  // Description:
  // Calculate the object-relative coordinates of a point in Cartesian coordinates.
  // NOTE: points are projected onto the disk plane
  // Elements of result are: u and v
  void ComputeObjectCoordinates(double x[3], double result[2]);

  // Description:
  // Calculate partial derivatives of a point location (a velocity)
  // with respect to changes in shape parameters
  // u and v.
  void ComputeVelocityWRTWidth(double u, double v, double result[3]);
  void ComputeVelocityWRTHeight(double u, double v, double result[3]);

protected:
  vtkRectangleSource(int res=8);
  ~vtkRectangleSource() {};

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  double Width;
  double Height;
  int    WidthResolution;
  int    HeightResolution;
  int    GenerateScalars;

private:
  vtkRectangleSource(const vtkRectangleSource&);  // Not implemented.
  void operator=(const vtkRectangleSource&);  // Not implemented.
};

#endif
