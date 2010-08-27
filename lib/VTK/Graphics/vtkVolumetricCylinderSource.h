// .NAME vtkVolumetricCylinderSource - generate a cylinder centered at
// the origin.
// .SECTION Description
// vtkVolumetricCylinderSource creates an unstructured grid in the
// shape of a cylinder centered at the origin. The unstructured grid
// consists of tetrahedral cells.
//
// The axis of the cylinder is aligned along the global y-axis.
// The height and radius of the cylinder can be specified, as well as 
// the number of sides.
//
// The output unstructured grid can optionally generate scalar data
// consisting of the fields:
// "t"     - normalized length along the axis of the cylinder (range [0, 1.0])
// "theta" - angle parameter about the axis (range [0, 2Pi))
// "r"     - normalized radial distance from the axis parameter (range [0, 1.0])


#ifndef __vtkVolumetricCylinderSource_h
#define __vtkVolumetricCylinderSource_h

#include "vtkUnstructuredGridAlgorithm.h"

#include "vtkCell.h" // Needed for VTK_CELL_SIZE

class vtkVolumetricCylinderSource : public vtkUnstructuredGridAlgorithm 
{
public:
  static vtkVolumetricCylinderSource *New();
  vtkTypeRevisionMacro(vtkVolumetricCylinderSource,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the height of the cylinder. Initial value is 1.
  vtkSetClampMacro(Height,double,0.0,VTK_DOUBLE_MAX)
  vtkGetMacro(Height,double);

  // Description:
  // Set the radius of the cylinder. Initial value is 0.5
  vtkSetClampMacro(Radius,double,0.0,VTK_DOUBLE_MAX)
  vtkGetMacro(Radius,double);

  // Description:
  // Set the number of facets used to define cylinder. Initial value is 6.
  vtkSetClampMacro(Resolution,int,2,VTK_CELL_SIZE)
  vtkGetMacro(Resolution,int);

  // Description:
  // Turn on/off generation of scalar data. Initial value is true.
  vtkSetMacro(GenerateScalars,int);
  vtkGetMacro(GenerateScalars,int);
  vtkBooleanMacro(GenerateScalars,int);

  // Description:
  // Calculate the location of a point given object-relative coordinates.
  void ComputePoint(double t, double theta, double r, double result[3]);

  // Description:
  // Calculate the object-relative coordinates of a point in Cartesian coordinates.
  // Elements of result are: t, theta, and r.
  void ComputeObjectCoordinates(double x[3], double result[3]);
 
  // Description:
  // Calculate partial derivatives of a point location (a velocity)
  // with respect to changes in shape parameters
  // Height and Radius.
  void ComputeVelocityWRTHeight(double t, double theta, double r, double result[3]);
  void ComputeVelocityWRTRadius(double t, double theta, double r, double result[3]);

protected:
  vtkVolumetricCylinderSource(int res=6);
  ~vtkVolumetricCylinderSource() {};

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  double Height;
  double Radius;
  int Resolution;
  int GenerateScalars;

private:
  vtkVolumetricCylinderSource(const vtkVolumetricCylinderSource&);  // Not implemented.
  void operator=(const vtkVolumetricCylinderSource&);  // Not implemented.
};

#endif
