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
#ifndef __vtkPolyDataDistance_h
#define __vtkPolyDataDistance_h

#include <vtkPolyDataAlgorithm.h>

class VTK_FILTERING_EXPORT vtkPolyDataDistance : public vtkPolyDataAlgorithm {
   public:
      vtkTypeRevisionMacro(vtkPolyDataDistance, vtkPolyDataAlgorithm);
      void PrintSelf(ostream& os, vtkIndent indent);

      static vtkPolyDataDistance *New();

      vtkSetMacro(SignedDistance,int);
      vtkGetMacro(SignedDistance,int);
      vtkBooleanMacro(SignedDistance,int);

      vtkSetMacro(InvertDistance,int);
      vtkGetMacro(InvertDistance,int);
      vtkBooleanMacro(InvertDistance,int);

      vtkSetMacro(ComputeSecondDistance,int);
      vtkGetMacro(ComputeSecondDistance,int);
      vtkBooleanMacro(ComputeSecondDistance,int);

      vtkPolyData* GetSecondDistanceOutput();

   protected:
      vtkPolyDataDistance();
      ~vtkPolyDataDistance();

      int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*);
      int FillInputPortInformation(int, vtkInformation*);

      void GetPolyDataDistance(vtkPolyData*, vtkPolyData*);

      int SignedDistance;
      int InvertDistance;
      int ComputeSecondDistance;

   private:
      vtkPolyDataDistance(const vtkPolyDataDistance&); // no implementation
      void operator=(const vtkPolyDataDistance&);      // no implementation
};

#endif
