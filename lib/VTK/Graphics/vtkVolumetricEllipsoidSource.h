// .NAME vtkVolumetricEllipsoidSource - generate an ellipsoid centered at
// the origin
// .SECTION Description
// vtkVolumetricEllipsoidSource creates an unstructured grid in the
// shape of an ellipsoid centered at the origin. The ellipsoid may have
// principal axes with different lengths. The unstructured grid consists
// of tetrahedral cells.
//
// The radius of each dimension of the ellipsoid can be specified, as well as the 
// resolution of the tesselation in theta (longitude) and phi (latitude).
//
// The output unstructured grid can optionally generate scalar data
// consisting of the fields:
// "theta" - longitude parameter (range [0, 2Pi))
// "phi"   - latitude parameter (range [0, pi])
// "r"     - normalized radial distance from the ellipsoid center to the ellipsoid
//           boundary (range [0, 1.0])


#ifndef __vtkVolumetricEllipsoidSource_h
#define __vtkVolumetricEllipsoidSource_h

#include "vtkUnstructuredGridAlgorithm.h"

#include "vtkCell.h" // Needed for VTK_CELL_SIZE

class vtkVolumetricEllipsoidSource : public vtkUnstructuredGridAlgorithm 
{
public:
  static vtkVolumetricEllipsoidSource *New();
  vtkTypeRevisionMacro(vtkVolumetricEllipsoidSource,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the radius of the cylinder. Initial value is 1.
  vtkSetVector3Macro(Radius,double);
  vtkGetVector3Macro(Radius,double);

  // Description:
  // Set the longitudinal resolution of the ellipsoid. Initial value is 8.
  vtkSetClampMacro(ThetaResolution,int,2,VTK_INT_MAX)
  vtkGetMacro(ThetaResolution,int);

  // Description:
  // Set the latitudinal resolution of the ellipsoid. Initial value is 8.
  vtkSetClampMacro(PhiResolution,int,2,VTK_INT_MAX)
  vtkGetMacro(PhiResolution,int);

  // Description:
  // Turn on/off generation of scalar data. Initial value is true.
  vtkSetMacro(GenerateScalars,int);
  vtkGetMacro(GenerateScalars,int);
  vtkBooleanMacro(GenerateScalars,int);

  // Description:
  // Calculate the location of a point given shape parameters.
  void ComputePoint(double theta, double phi, double r, double result[3]);

  // Description:
  // Calculate the object-relative coordinates of a point in Cartesian coordinates.
  // Elements of result are: theta, phi, and r.
  void ComputeObjectCoordinates(double x[3], double result[3]);

  // Description:
  // Calculate partial derivatives of a point location (a velocity)
  // with respect to changes in shape parameters
  // Radius[0] (Radius X), Radius[1] (Radius Y), Radius[2] (Radius Z).
  void ComputeVelocityWRTRadiusX(double theta, double phi, double r, double result[3]);
  void ComputeVelocityWRTRadiusY(double theta, double phi, double r, double result[3]);
  void ComputeVelocityWRTRadiusZ(double theta, double phi, double r, double result[3]);

protected:
  vtkVolumetricEllipsoidSource(int res=8);
  ~vtkVolumetricEllipsoidSource() {};

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  double Radius[3];
  int ThetaResolution;
  int PhiResolution;
  int GenerateScalars;

private:
  vtkVolumetricEllipsoidSource(const vtkVolumetricEllipsoidSource&);  // Not implemented.
  void operator=(const vtkVolumetricEllipsoidSource&);  // Not implemented.
};

#endif
