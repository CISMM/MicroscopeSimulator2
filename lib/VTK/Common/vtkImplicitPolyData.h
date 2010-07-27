/*=========================================================================

Written by: Chris Weigle, University of North Carolina at Chapel Hill
Email Contact: weigle@cs.unc.edu

IN NO EVENT SHALL THE UNIVERSITY OF NORTH CAROLINA AT CHAPEL HILL BE
LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE
OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF NORTH
CAROLINA HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

THE UNIVERSITY OF NORTH CAROLINA SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS
ON AN "AS IS" BASIS, AND THE UNIVERSITY OF NORTH CAROLINA HAS NO
OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
MODIFICATIONS.

=========================================================================*/
#ifndef __vtkImplicitPolyData_h
#define __vtkImplicitPolyData_h

#include <math.h>
#include "vtkDataSetToImageFilter.h"
#include "vtkPolyData.h"
#include "vtkImplicitFunction.h"
#include "vtkTriangleFilter.h"
#include "vtkIdList.h"

#include "vtkCellLocator.h"
#include "vtkOBBTree.h"


class VTK_COMMON_EXPORT vtkImplicitPolyData : public vtkImplicitFunction
{
public:
  vtkTypeRevisionMacro(vtkImplicitPolyData,vtkImplicitFunction);
  static vtkImplicitPolyData *New();
  const char *GetClassName() {return "vtkImplicitPolyData";};
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkImplicitPolyData();
  ~vtkImplicitPolyData();

  // Description:
  // Return the MTime also considering the Input dependency.
  unsigned long GetMTime();

  void SetEvaluateBounds( double eBounds[6] );

  // Description:
  // Evaluate plane equation of nearest triangle to point x[3].
  double EvaluateFunction(double x[3]);

  // Description:
  // Evaluate function gradient of nearest triangle to point x[3].
  void EvaluateGradient(double x[3], double g[3]);

  //BTX
  inline double Evaluate(double x[3], double g[3]) { return sharedEvaluate(x, g); }
  //ETX

  // Description:
  // Set the input polydata used for the implicit function evaluation.
  // Passes input through an internal instance of vtkTriangleFilter to remove
  // vertices and lines, leaving only triangular polygons for evaluation as
  // implicit planes
  void SetInput(vtkPolyData *input);

  // Description:
  // Set / get the function value to use if no input polydata specified.
  vtkSetMacro(NoValue,double);
  vtkGetMacro(NoValue,double);

  vtkGetMacro(ReverseBias,int);
  vtkSetMacro(ReverseBias,int);
  vtkBooleanMacro(ReverseBias,int);

  vtkGetMacro(Tolerance,double);
  vtkSetMacro(Tolerance,double);

  // Description:
  // Set / get the function gradient to use if no input polydata specified.
  vtkSetVector3Macro(NoGradient,double);
  vtkGetVector3Macro(NoGradient,double);

protected:
  double NoValue;
  double NoGradient[3];

  vtkTriangleFilter *tri;
  vtkPolyData *input;
  vtkCellLocator *locator;

  double EvaluateBounds[6];
  int EvaluateBoundsSet;
  int ReverseBias;
  double Tolerance;

  double sharedEvaluate( double x[3], double n[3] );

private:
  vtkImplicitPolyData(const vtkImplicitPolyData&);  // Not implemented.
  void operator=(const vtkImplicitPolyData&);  // Not implemented.
};

#endif
