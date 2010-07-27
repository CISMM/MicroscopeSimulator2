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

#ifndef __vtkPrincipalCurvatures_h
#define __vtkPrincipalCurvatures_h

#include <vtkPolyDataAlgorithm.h>

class VTK_FILTERING_EXPORT vtkPrincipalCurvatures : public vtkPolyDataAlgorithm
{
   public:
      vtkTypeRevisionMacro(vtkPrincipalCurvatures, vtkPolyDataAlgorithm);
      void PrintSelf(ostream& os, vtkIndent indent);

      static vtkPrincipalCurvatures *New();

      vtkSetMacro(ReplaceNormals, int);
      vtkGetMacro(ReplaceNormals, int);
      vtkBooleanMacro(ReplaceNormals, int);

      vtkSetClampMacro(FeatureScale, double, 0., VTK_DOUBLE_MAX);
      vtkGetMacro(FeatureScale, double);

      vtkSetClampMacro(FeatureAngle, double, 0., 90.);
      vtkGetMacro(FeatureAngle, double);

   protected:
      vtkPrincipalCurvatures();
      ~vtkPrincipalCurvatures();

      int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*);

      void ComputeTriangleAreas(vtkPolyData*);
      void ComputePrincipalCurvatures(vtkPolyData*);

      double* AreaWeights;

      int ReplaceNormals;
      double FeatureScale;
      double FeatureAngle;

   private:
      vtkPrincipalCurvatures(const vtkPrincipalCurvatures&); // not implemented
      void operator=(const vtkPrincipalCurvatures&); // not implemented
};

#endif
