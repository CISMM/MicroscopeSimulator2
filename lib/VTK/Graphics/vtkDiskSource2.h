// .NAME vtkDiskSource2 - generate a disk centered at the origin
// .SECTION Description
// vtkDiskSource2 creates a vtkPolyData in the shape of a disk centered at the
// origin. The polydata consists of triangle cells. 
//
// The radius of the disk can be specified, as well as the resolution of the
// tesselation about the disk center.
//
// The output unstructured grid can optionally generate scalar data
// consisting of the fields:
// "theta" - angle about the center of the disk
// "r"     - normalized radial distance from the disk's center


#ifndef __vtkDiskSource2_h
#define __vtkDiskSource2_h

#include "vtkPolyDataAlgorithm.h"

#include "vtkCell.h" // Needed for VTK_CELL_SIZE

class vtkDiskSource2 : public vtkPolyDataAlgorithm 
{
public:
  static vtkDiskSource2 *New();
  vtkTypeRevisionMacro(vtkDiskSource2,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the radius of the disk. Initial value is 1.0.
  vtkSetMacro(Radius,double);
  vtkGetMacro(Radius,double);

  // Description:
  // Set the tesselation resolution of the disk. Initial value is 8.
  vtkSetClampMacro(CircumferentialResolution,int,3,VTK_INT_MAX)
  vtkGetMacro(CircumferentialResolution,int);

  // Description:
  // Turn on/off generation of scalar data. Initial value is true.
  vtkSetMacro(GenerateScalars,int);
  vtkGetMacro(GenerateScalars,int);
  vtkBooleanMacro(GenerateScalars,int);

  // Description:
  // Calculate the location of a point given shape parameters.
  void ComputePoint(double theta, double r, double result[3]);

  // Description:
  // Calculate Jacobian of a given point with respect to the shape parameter
  // Radius
  void ComputeJacobianOfRadius(double theta, double r, double result[3]);

protected:
  vtkDiskSource2(int res=8);
  ~vtkDiskSource2() {};

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  double Radius;
  int    CircumferentialResolution;
  int    GenerateScalars;

private:
  vtkDiskSource2(const vtkDiskSource2&);  // Not implemented.
  void operator=(const vtkDiskSource2&);  // Not implemented.
};

#endif
