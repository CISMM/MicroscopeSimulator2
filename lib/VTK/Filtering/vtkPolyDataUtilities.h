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
#ifndef __vtkPolyDataUtilities_h
#define __vtkPolyDataUtilities_h

#include <vtkPolyData.h>

class vtkPriorityQueue;

#include <deque>
//#include <set>
#include <list>

class PDGeometryInfo
{
public:
  unsigned int numVerts;
  unsigned int numEdges;
  unsigned int numFaces;
  vtkIdType initialCell;
  typedef std::deque<int> Loop;
  //  typedef std::set<Loop> LoopSet;
  typedef std::list<Loop> LoopSet;
  typedef LoopSet::iterator LoopSetIter;
  LoopSet boundaryLoops;

  PDGeometryInfo() : numVerts(0), numEdges(0), numFaces(0), initialCell(-1), boundaryLoops() {}
  PDGeometryInfo(const PDGeometryInfo& gi) : numVerts(gi.numVerts), numEdges(gi.numEdges),
    numFaces(gi.numFaces), initialCell(gi.initialCell), boundaryLoops(gi.boundaryLoops) {}
    PDGeometryInfo(unsigned int nV, unsigned int nE, unsigned int nF, vtkIdType iC, LoopSet bL) :
  numVerts(nV), numEdges(nE), numFaces(nF), initialCell(iC), boundaryLoops(bL) {}

  inline unsigned int Genus()
  {
    return (2 + numEdges - NumBoundaries() - numFaces - numVerts) / 2;
  }
  inline unsigned int NumBoundaries() { return static_cast<unsigned int>(boundaryLoops.size()); };
};

void GetPolyDataGeometryInfo(vtkPolyData* const, std::deque<PDGeometryInfo>&);
void FillBoundaryLoops(vtkPolyData*, PDGeometryInfo::LoopSet&);

bool GetAdjacentPoints(vtkPolyData* const, const vtkIdType, vtkIdList*);
bool GetOrderedAdjacentPoints(vtkPolyData* const, const vtkIdType, vtkIdList*);

int CalcGeodesic(vtkPolyData* const, vtkPriorityQueue*, double*, int* = 0, int* = 0);

bool TriangulatePoints(double*, double, double*, double, double*, double*);

#endif
