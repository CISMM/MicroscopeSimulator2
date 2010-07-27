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
#ifndef __vtkRefactorPolyData_h
#define __vtkRefactorPolyData_h

#include <vtkPolyDataAlgorithm.h>

class vtkIdList;
class vtkImplicitPolyData;

class VTK_FILTERING_EXPORT vtkRefactorPolyData : public vtkPolyDataAlgorithm
{
   public:
      vtkTypeRevisionMacro(vtkRefactorPolyData, vtkPolyDataAlgorithm);
      void PrintSelf(ostream& os, vtkIndent indent);

      vtkPolyData* GetOutsideOutput();
      vtkPolyData* GetInsideOutput();
      vtkPolyData* GetIntersectionOutput();

      vtkSetMacro(Tolerance, double);
      vtkGetMacro(Tolerance, double);

      static vtkRefactorPolyData *New();

   protected:
      vtkRefactorPolyData();
      ~vtkRefactorPolyData();

      double Tolerance;

      void SortPolyData(vtkPolyData*, vtkPolyData*, vtkIdList*, vtkIdList*);

      int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*);
      int FillInputPortInformation(int, vtkInformation*);

   private:
      vtkRefactorPolyData(const vtkRefactorPolyData&); // no implementation
      void operator=(const vtkRefactorPolyData&);      // no implementation
};

#endif
