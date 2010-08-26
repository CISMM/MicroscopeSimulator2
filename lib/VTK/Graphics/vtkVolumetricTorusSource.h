// .NAME vtkVolumetricTorusSource - generate a torus centered at
// the origin
// .SECTION Description
// vtkVolumetricTorusSource creates an unstructured grid in the
// shape of a torus centered at the origin. The unstructured grid consists of
// tetrahedral cells. 
//
// The radius of each dimension of the torus can be specified, as well as the 
// resolution of the tesselation in theta (longitude) and phi (latitude).
//
// The output unstructured grid can optionally generate scalar data
// consisting of the fields:
// "theta" - longitude parameter (range [0, 2 pi))
// "phi"   - latitude parameter (range [0, 2 pi])
// "r"     - normalized radial distance from the circle defining the medial axis
//           of the torus


#ifndef __vtkVolumetricTorusSource_h
#define __vtkVolumetricTorusSource_h

#include "vtkUnstructuredGridAlgorithm.h"

#include "vtkCell.h" // Needed for VTK_CELL_SIZE

class vtkVolumetricTorusSource : public vtkUnstructuredGridAlgorithm 
{
public:
  static vtkVolumetricTorusSource *New();
  vtkTypeRevisionMacro(vtkVolumetricTorusSource,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the cross-section radius of the torus. Initial value is 0.1.
  vtkSetMacro(CrossSectionRadius,double);
  vtkGetMacro(CrossSectionRadius,double);

  // Description:
  // Set the ring radius of the torus. Initial value is 1.0.
  vtkSetMacro(RingRadius,double);
  vtkGetMacro(RingRadius,double);

  // Description:
  // Set the longitudinal resolution of the torus. Initial value is 8.
  vtkSetClampMacro(ThetaResolution,int,2,VTK_INT_MAX)
  vtkGetMacro(ThetaResolution,int);

  // Description:
  // Set the latitudinal resolution of the sphere. Initial value is 8.
  vtkSetClampMacro(PhiResolution,int,2,VTK_INT_MAX)
  vtkGetMacro(PhiResolution,int);

  // Description:
  // Turn on/off generation of scalar data. Initial value is true.
  vtkSetMacro(GenerateScalars,int);
  vtkGetMacro(GenerateScalars,int);
  vtkBooleanMacro(GenerateScalars,int);


protected:
  vtkVolumetricTorusSource(int res=8);
  ~vtkVolumetricTorusSource() {};

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  double CrossSectionRadius;
  double RingRadius;
  int    ThetaResolution;
  int    PhiResolution;
  int    GenerateScalars;

private:
  vtkVolumetricTorusSource(const vtkVolumetricTorusSource&);  // Not implemented.
  void operator=(const vtkVolumetricTorusSource&);  // Not implemented.
};

#endif
