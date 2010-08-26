// .NAME vtkVolumetricHollowCylinderSource - generate a hollow cylinder centered at
// a given location.
// .SECTION Description
// vtkVolumetricHollowCylinderSource creates an unstructured grid in the
// shape of a hollow cylinder centered at Center. The unstructured grid
// consists of tetrahedral cells.
//
// The axis of the hollow cylinder is aligned along the global y-axis.
// The height and radius of the cylinder can be specified, as well as 
// the number of sides.
//
// The output unstructured grid can optionally generate scalar data
// consisting of the fields:
// "t"     - parameter along the axis of the cylinder (range [0, Height])
// "theta" - angle parameter about the axis (range [0, 2Pi))
// "r"     - radial distance from the axis parameter (range [0, Radius])


#ifndef __vtkVolumetricHollowCylinderSource_h
#define __vtkVolumetricHollowCylinderSource_h

#include "vtkUnstructuredGridAlgorithm.h"

#include "vtkCell.h" // Needed for VTK_CELL_SIZE

class vtkVolumetricHollowCylinderSource : public vtkUnstructuredGridAlgorithm 
{
public:
  static vtkVolumetricHollowCylinderSource *New();
  vtkTypeRevisionMacro(vtkVolumetricHollowCylinderSource,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the height of the cylinder. Initial value is 1.
  vtkSetClampMacro(Height,double,0.0,VTK_DOUBLE_MAX)
  vtkGetMacro(Height,double);

  // Description:
  // Set the inner radius of the cylinder. This defines the radius of the
  // hole in the center of the cylinder. Initial value is 0.1.
  vtkSetClampMacro(InnerRadius,double,0.0,this->OuterRadius)
  vtkGetMacro(InnerRadius,double);

  // Description:
  // Set the outer radius of the cylinder. Initial value is 0.5.
  vtkSetClampMacro(OuterRadius,double,this->InnerRadius,VTK_DOUBLE_MAX)
  vtkGetMacro(OuterRadius,double);

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
  vtkVolumetricHollowCylinderSource(int res=6);
  ~vtkVolumetricHollowCylinderSource() {};

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  double Height;
  double InnerRadius;
  double OuterRadius;
  double Center[3];
  int Resolution;
  int GenerateScalars;

private:
  vtkVolumetricHollowCylinderSource(const vtkVolumetricHollowCylinderSource&);  // Not implemented.
  void operator=(const vtkVolumetricHollowCylinderSource&);  // Not implemented.
};

#endif
