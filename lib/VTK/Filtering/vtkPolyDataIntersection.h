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
#ifndef __vtkPolyDataIntersection_h
#define __vtkPolyDataIntersection_h

#include <vtkPolyDataAlgorithm.h>

#include <map>

class vtkOBBTree;
class vtkOBBNode;
class vtkMatrix4x4;

class vtkPolyDataIntersection : public vtkPolyDataAlgorithm
{
   public:
      vtkTypeRevisionMacro(vtkPolyDataIntersection, vtkPolyDataAlgorithm);
      void PrintSelf(ostream& os, vtkIndent indent);

      vtkGetMacro(SplitFirstMesh, int);
      vtkSetMacro(SplitFirstMesh, int);
      vtkBooleanMacro(SplitFirstMesh, int);

      vtkGetMacro(SplitSecondMesh, int);
      vtkSetMacro(SplitSecondMesh, int);
      vtkBooleanMacro(SplitSecondMesh, int);

      vtkPolyData* GetFirstMeshOutput();
      vtkPolyData* GetSecondMeshOutput();

      static vtkPolyDataIntersection *New();

   protected:
      vtkPolyDataIntersection();
      ~vtkPolyDataIntersection();

      //BTX
      int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*);
      int FillInputPortInformation(int, vtkInformation*);

      void IntersectTriangles(vtkPolyData*, vtkPolyData*, vtkPolyData*, vtkPolyData*, vtkPolyData*);
      static int TestAndMark(vtkOBBNode*, vtkOBBNode*, vtkMatrix4x4*, void*);
      int FindIntersection(int, int, double[3], double[3]);
      void SplitTriangles(vtkPolyData*, vtkPolyData*, vtkPolyData*, int);

      int SplitFirstMesh;
      int SplitSecondMesh;

      vtkPolyData* pdA;
      vtkPolyData* pdB;
      vtkOBBTree* obbB;

      std::multimap<int, int> potIntMap;
      std::multimap<int, int> intIndMap[2];
      typedef std::multimap<int, int>::iterator mapIter;
      //ETX

   private:
      vtkPolyDataIntersection(const vtkPolyDataIntersection&); // no implementation
      void operator=(const vtkPolyDataIntersection&);          // no implementation
};

#endif
