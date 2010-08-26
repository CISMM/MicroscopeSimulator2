// .NAME vtkVolumetricCylinderSource - generate a cylinder centered at
// a given location.
// .SECTION Description
// vtkVolumetricCylinderSource creates an unstructured grid in the
// shape of a cylinder centered at Center. The unstructured grid
// consists of tetrahedral cells.
//
// The axis of the cylinder is aligned along the global y-axis.
// The height and radius of the cylinder can be specified, as well as 
// the number of sides.
//
// The output unstructured grid can optionally generate scalar data
// consisting of the fields:
// "t"     - parameter along the axis of the cylinder (range [0, Height])
// "theta" - angle parameter about the axis (range [0, 2Pi))
// "r"     - radial distance from the axis parameter (range [0, Radius])


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
  // Set/Get cylinder center. Initial value is (0.0,0.0,0.0)
  vtkSetVector3Macro(Center,double);
  vtkGetVectorMacro(Center,double,3);

  // Description:
  // Set the number of facets used to define cylinder. Initial value is 6.
  vtkSetClampMacro(Resolution,int,2,VTK_CELL_SIZE)
  vtkGetMacro(Resolution,int);

  // Description:
  // Turn on/off generation of scalar data. Initial value is true.
  vtkSetMacro(GenerateScalars,int);
  vtkGetMacro(GenerateScalars,int);
  vtkBooleanMacro(GenerateScalars,int);


protected:
  vtkVolumetricCylinderSource(int res=6);
  ~vtkVolumetricCylinderSource() {};

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  double Height;
  double Radius;
  double Center[3];
  int Resolution;
  int GenerateScalars;

private:
  vtkVolumetricCylinderSource(const vtkVolumetricCylinderSource&);  // Not implemented.
  void operator=(const vtkVolumetricCylinderSource&);  // Not implemented.
};

#endif
