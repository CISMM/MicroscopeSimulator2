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
#include "vtkPolyDataIntersection.h"

#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkPolyData.h>
#include <vtkFieldData.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkPolygon.h>
#include <vtkTriangle.h>
#include <vtkDataArray.h>
#include <vtkIntArray.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkCellArray.h>
#include <vtkOBBTree.h>
#include <vtkDelaunay2D.h>
#include <vtkCleanPolyData.h>
#include <vtkCellDataToPointData.h>
#include <vtkMatrix4x4.h>
#include <vtkMergePoints.h>

#include "tri_tri_overlap.h"

#include <queue>
#include <vector>
#include <map>

vtkCxxRevisionMacro(vtkPolyDataIntersection, "$Revision: 1.2 $");
vtkStandardNewMacro(vtkPolyDataIntersection);

vtkPolyDataIntersection::vtkPolyDataIntersection() : vtkPolyDataAlgorithm()
{
   this->pdA = 0;
   this->pdB = 0;

   this->SplitFirstMesh = 1;
   this->SplitSecondMesh = 1;

   this->SetNumberOfInputPorts(2);
   this->SetNumberOfOutputPorts(3);

   vtkPolyData* output1 = vtkPolyData::New();
   this->GetExecutive()->SetOutputData(1, output1);
   output1->Delete();

   vtkPolyData* output2 = vtkPolyData::New();
   this->GetExecutive()->SetOutputData(2, output2);
   output2->Delete();
}

vtkPolyDataIntersection::~vtkPolyDataIntersection()
{
}

#define COPY_INPUT_TO_OUTPUT_MACRO(IN,OUT) \
{\
      vtkCleanPolyData* clean = vtkCleanPolyData::New();\
      clean->SetInput(IN);\
      clean->ConvertLinesToPointsOff();\
      clean->ConvertPolysToLinesOff();\
      clean->ConvertStripsToPolysOff();\
      clean->PointMergingOn();\
      clean->SetAbsoluteTolerance(0);\
      clean->ToleranceIsAbsoluteOn();\
      clean->GetOutput()->ReleaseDataFlagOn();\
\
      vtkCellDataToPointData* cd2pd = vtkCellDataToPointData::New();\
      cd2pd->SetInput(clean->GetOutput());\
      cd2pd->PassCellDataOn();\
      cd2pd->GetOutput()->ReleaseDataFlagOn();\
      cd2pd->Update();\
\
      OUT->CopyStructure(cd2pd->GetOutput());\
      OUT->GetPointData()->PassData(cd2pd->GetOutput()->GetPointData());\
      OUT->GetCellData()->PassData(cd2pd->GetOutput()->GetCellData());\
      OUT->Squeeze();\
\
      cd2pd->Delete();\
      clean->Delete();\
}

int vtkPolyDataIntersection::RequestData(vtkInformation*        vtkNotUsed(request),
                                         vtkInformationVector** inputVector,
                                         vtkInformationVector*  outputVector)
{
   vtkInformation* inInfo0 = inputVector[0]->GetInformationObject(0);
   vtkInformation* inInfo1 = inputVector[1]->GetInformationObject(0);
   vtkInformation* outInfo = outputVector->GetInformationObject(0);

   if (!inInfo0 || !inInfo1 || !outInfo) return 0;

   vtkPolyData* input0 = vtkPolyData::SafeDownCast(inInfo0->Get(vtkDataObject::DATA_OBJECT()));
   vtkPolyData* input1 = vtkPolyData::SafeDownCast(inInfo1->Get(vtkDataObject::DATA_OBJECT()));
   vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

   vtkPolyData* firstMeshOutput = this->GetFirstMeshOutput();
   vtkPolyData* secondMeshOutput = this->GetSecondMeshOutput();

   if (!input0 || !input1 || !output) return 0;

   vtkPolyData* temp0 = vtkPolyData::New();
   temp0->Allocate(input0);

   vtkPolyData* temp1 = vtkPolyData::New();
   temp1->Allocate(input1);

   vtkPolyData* temp2 = vtkPolyData::New();
   IntersectTriangles(temp0, temp1, input0, input1, temp2);

   output->CopyStructure(temp2);
   output->GetPointData()->PassData(temp2->GetPointData());
   output->GetCellData()->PassData(temp2->GetCellData());
   output->Squeeze();
   temp2->Delete();

   if (this->SplitFirstMesh) COPY_INPUT_TO_OUTPUT_MACRO(temp0, firstMeshOutput);
   temp0->Delete();
   
   if (this->SplitSecondMesh) COPY_INPUT_TO_OUTPUT_MACRO(temp1, secondMeshOutput);
   temp1->Delete();

   return 1;
}

void vtkPolyDataIntersection::PrintSelf(ostream& os, vtkIndent indent)
{
   this->Superclass::PrintSelf(os,indent);
}

int vtkPolyDataIntersection::FillInputPortInformation(int port, vtkInformation *info)
{
   if (!this->Superclass::FillInputPortInformation(port, info)) return 0;
   if (port == 0)
   {
      info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
   }
   else if (port == 1)
   {
      info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
      info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 0);
   }
   return 1;
}

vtkPolyData* vtkPolyDataIntersection::GetFirstMeshOutput()
{
   if (!this->SplitFirstMesh) return 0;
   return vtkPolyData::SafeDownCast(this->GetExecutive()->GetOutputData(1));
}

vtkPolyData* vtkPolyDataIntersection::GetSecondMeshOutput()
{
   if (!this->SplitSecondMesh) return 0;
   return vtkPolyData::SafeDownCast(this->GetExecutive()->GetOutputData(2));
}

void vtkPolyDataIntersection::IntersectTriangles(vtkPolyData* out0, vtkPolyData* out1,
                                                 vtkPolyData* in0, vtkPolyData* in1,
                                                 vtkPolyData* inter)
{
   vtkDebugMacro(<<"Start vtkPolyDataIntersection::MarkTriangles");

   if (in0->GetNumberOfPolys() == 0 || in0->GetNumberOfPoints() == 0)
   {
      vtkErrorMacro(<<"No points/cells to operate on");
      return;
   }

   if (in1->GetNumberOfPolys() == 0 || in1->GetNumberOfPoints() == 0)
   {
      vtkErrorMacro(<<"No points/cells to intersect with");
      return;
   }

   this->pdA = in0;
   this->pdB = in1;

   vtkOBBTree* obbA = vtkOBBTree::New();
   obbA->SetDataSet(in0);
   obbA->SetNumberOfCellsPerNode(10);
   obbA->SetMaxLevel(1e6);// that's a lot of levels, should give 1 cell/bucket typically
   obbA->SetTolerance(1e-6);
   obbA->AutomaticOn();
   obbA->BuildLocator();

   this->obbB = vtkOBBTree::New();
   this->obbB->SetDataSet(in1);
   this->obbB->SetNumberOfCellsPerNode(10);
   this->obbB->SetMaxLevel(1e6);
   this->obbB->SetTolerance(1e-6);
   this->obbB->AutomaticOn();
   this->obbB->BuildLocator();

   this->potIntMap.clear();

   obbA->IntersectWithOBBTree(this->obbB, 0, this->TestAndMark, this);

   obbA->Delete();
   this->obbB->Delete();

   this->intIndMap[0].clear();
   this->intIndMap[1].clear();

   vtkPoints* points = vtkPoints::New();
   vtkCellArray* lines = vtkCellArray::New();
   mapIter it1 = this->potIntMap.begin();
   mapIter it2;
   while (it1 != this->potIntMap.end())
   {
      it2 = this->potIntMap.upper_bound(it1->first);
      double pt0[3], pt1[3];

      //for ( ; it1 != it2; it1++)
      while (it1 != it2)
      {
         if (FindIntersection(it1->first, it1->second, pt0, pt1))
         {
            int ind = points->GetNumberOfPoints();
            lines->InsertNextCell(2);
            lines->InsertCellPoint(points->InsertNextPoint(pt0));
            lines->InsertCellPoint(points->InsertNextPoint(pt1));

            this->intIndMap[0].insert(std::make_pair(it1->first, ind));
            this->intIndMap[1].insert(std::make_pair(it1->second, ind));
         }
         if (it1 != this->potIntMap.end())
            it1++;
      }
   }
//   points->Squeeze();
//   lines->Squeeze();

   inter->SetPoints(points);
   inter->SetLines(lines);
   points->Delete();
   lines->Delete();
   inter->Squeeze();

   this->potIntMap.clear();

   if (this->SplitFirstMesh) SplitTriangles(in0, out0, inter, 0);
   this->intIndMap[0].clear();

   if (this->SplitSecondMesh) SplitTriangles(in1, out1, inter, 1);
   this->intIndMap[1].clear();

   vtkDebugMacro(<<"End vtkPolyDataIntersection::MarkTriangles");
}

void vtkPolyDataIntersection::SplitTriangles(vtkPolyData* in, vtkPolyData* out, vtkPolyData* inter, int AorB)
{
   vtkIdList* toCopy = vtkIdList::New();
   int start = 0;
   mapIter it1 = this->intIndMap[AorB].begin();
   mapIter it2;
   while (it1 != this->intIndMap[AorB].end())
   {
      it2 = this->intIndMap[AorB].upper_bound(it1->first);
      for (int i = start; i < it1->first; i++) toCopy->InsertNextId(i);
      start = it1->first+1;
      //for ( ; it1 != it2; it1++);
      it1 = it2;
   }

   for (int i = start; i < in->GetNumberOfCells(); i++) toCopy->InsertNextId(i);
   out->GetCellData()->CopyAllocate(in->GetCellData(), toCopy->GetNumberOfIds());
   out->CopyCells(in, toCopy, 0);
   toCopy->Reset();

   vtkPoints* points = vtkPoints::New();
   vtkCellArray* lines = vtkCellArray::New();
   vtkPolyData* pd = vtkPolyData::New();

   vtkMergePoints* merge = vtkMergePoints::New();

   int split = 0, splits = 0;
   it1 = this->intIndMap[AorB].begin();
   while (it1 != this->intIndMap[AorB].end())
   {
      it2 = this->intIndMap[AorB].upper_bound(it1->first);
      split++;
      int first = it1->first;

      // make a polydata containing the intersection points as line segments
      points->Reset();
      lines->Reset();
      merge->InitPointInsertion(points, in->GetBounds());

      double p[3];
      vtkIdType pid0, pid1;
      for ( ; it1 != it2; it1++)
      {
         int ind = it1->second;

         inter->GetPoint(ind+0, p);
         merge->InsertUniquePoint(p, pid0);

         inter->GetPoint(ind+1, p);
         merge->InsertUniquePoint(p, pid1);

         if (pid0 != pid1)
         {
            lines->InsertNextCell(2);
            lines->InsertCellPoint(pid0);
            lines->InsertCellPoint(pid1);
         }
      }

      vtkIdType* pts;
      vtkIdType npts;
      in->GetCellPoints(first, npts, pts);
      in->GetPoint(pts[0], p);
      merge->InsertUniquePoint(p, pid0);
      in->GetPoint(pts[1], p);
      merge->InsertUniquePoint(p, pid0);
      in->GetPoint(pts[2], p);
      merge->InsertUniquePoint(p, pid0);

      pd->Initialize();
      pd->SetPoints(points);
      pd->SetLines(lines);

//#define USE_TRIGEN
#ifdef USE_TRIGEN
      vtkTriGen* del = vtkTriGen::New();
      del->SetInput(0, pd);
      del->SetInput(1, pd);
      del->Update();
#else
      vtkDelaunay2D* del = vtkDelaunay2D::New();
      del->SetInput(pd);
      del->SetSource(pd);
      del->SetTolerance(0.);
      del->SetAlpha(0.);
      del->SetOffset(1e6);
      del->SetProjectionPlaneMode(VTK_BEST_FITTING_PLANE);
      del->BoundingTriangulationOff();
      del->Update();
#endif

      vtkPolyData* temp = del->GetOutput();

      toCopy->Reset();
      double n0[3], n1[3];
      if (in->GetCellData()->GetNormals())
         in->GetCellData()->GetNormals()->GetTuple(first, n0);
      else
      {
         in->GetCellPoints(first, npts, pts);
         vtkTriangle::ComputeNormal(in->GetPoints(), npts, pts, n0);
      }
      vtkFloatArray* norm = vtkFloatArray::New();
      norm->SetName("Normals");
      norm->SetNumberOfComponents(3);

      for (int i = 0; i < temp->GetNumberOfCells(); i++)
      {
         if (temp->GetCellType(i) == VTK_TRIANGLE)
         {
            toCopy->InsertNextId(i);
            temp->GetCellPoints(i, npts, pts);
            vtkTriangle::ComputeNormal(temp->GetPoints(), npts, pts, n1);
            if (vtkMath::Dot(n0, n1) < 0.)
            {
               temp->ReverseCell(i);
               //n1[0] = -n1[0];
               //n1[1] = -n1[1];
               //n1[2] = -n1[2];
            }
            if (in->GetCellData()->GetNormals())
               norm->InsertNextTuple(n0);
         }
      }

      if (in->GetCellData()->GetNormals())
      {
         temp->GetCellData()->AddArray(norm);
         temp->GetCellData()->SetActiveNormals("Normals");
      }
      // would be nice if this copied all the point data instead of just creating normals

      splits += toCopy->GetNumberOfIds();
      //if (splits < 3)
      //   std::cerr << "vtkPDI: cell # " << first << " split into " << toCopy->GetNumberOfIds() << " cells" << std::endl;
      out->CopyCells(temp, toCopy, 0);
      //std::cerr << "vtkPDI: " << out->GetNumberOfCells() << " cells in output" << std::endl;

      norm->Delete();
      del->Delete();

      out->Squeeze();
   }
   //std::cerr << "vtkPDI: sanity check!" << std::endl
      //<< "in    : " << in->GetNumberOfCells() << std::endl
      //<< "copy  : " << copied << std::endl
      //<< "split : " << split << std::endl
      //<< "in = copy + split ? " << in->GetNumberOfCells() << " : " << (copied + split) << std::endl
      //<< "splits: " << splits << std::endl
      //<< "out   : " << out->GetNumberOfCells() << std::endl
      //<< "out = copy + splits ? " << out->GetNumberOfCells() << " : " << (copied + splits) << std::endl;

   toCopy->Delete();
   lines->Delete();
   points->Delete();
   merge->Delete();
   pd->Delete();

   out->Squeeze();
}

int vtkPolyDataIntersection::TestAndMark(vtkOBBNode* nodeA, vtkOBBNode* nodeB,
                                         vtkMatrix4x4* xform, void* arg)
{
   vtkPolyDataIntersection* self = reinterpret_cast<vtkPolyDataIntersection*>(arg);

   int numCellsA = nodeA->Cells->GetNumberOfIds();
   int numCellsB = nodeB->Cells->GetNumberOfIds();
   int retval = 0;

   for (int i = 0; i < numCellsA; i++)
   {
      int cellIdA = nodeA->Cells->GetId(i),
          typeA = self->pdA->GetCellType(cellIdA);

      if (typeA == VTK_TRIANGLE)
      {
         vtkIdType nPts;
         vtkIdType* aPts;
         self->pdA->GetCellPoints(cellIdA, nPts, aPts);
         double aPt[3][3];
         self->pdA->GetPoint(aPts[0], aPt[0]);
         self->pdA->GetPoint(aPts[1], aPt[1]);
         self->pdA->GetPoint(aPts[2], aPt[2]);

         if (self->obbB->TriangleIntersectsNode(nodeB, aPt[0], aPt[1], aPt[2], xform))
            for (int j = 0; j < numCellsB; j++)
            {
               int cellIdB = nodeB->Cells->GetId(j),
                     typeB = self->pdB->GetCellType(cellIdB);

               if (typeB == VTK_TRIANGLE)
               {
                  self->potIntMap.insert(std::make_pair(cellIdA, cellIdB));
                  retval++;
               }
            }
      }
   }
   return retval;
}

int vtkPolyDataIntersection::FindIntersection(int cellIdA, int cellIdB, double pt0[3], double pt1[3])
{
   if (this->pdA->GetCellType(cellIdA) != VTK_TRIANGLE) return 0;
   if (this->pdB->GetCellType(cellIdB) != VTK_TRIANGLE) return 0;

   vtkIdType* aPts;
   vtkIdType nPts;
   this->pdA->GetCellPoints(cellIdA, nPts, aPts);
   double aPt[3][3];
   this->pdA->GetPoint(aPts[0], aPt[0]);
   this->pdA->GetPoint(aPts[1], aPt[1]);
   this->pdA->GetPoint(aPts[2], aPt[2]);

   vtkIdType* bPts;
   this->pdB->GetCellPoints(cellIdB, nPts, bPts);
   double bPt[3][3];
   this->pdB->GetPoint(bPts[0], bPt[0]);
   this->pdB->GetPoint(bPts[1], bPt[1]);
   this->pdB->GetPoint(bPts[2], bPt[2]);

   int coplanar = 0;
//   int inter = tri_tri_intersect_with_isectline(aPt[0], aPt[1], aPt[2],
//                                                bPt[0], bPt[1], bPt[2],
//                                                coplanar, pt0, pt1);
   int inter = tri_tri_intersection_test_3d(aPt[0], aPt[1], aPt[2],
                                            bPt[0], bPt[1], bPt[2],
                                            &coplanar, pt0, pt1);

   if (coplanar) vtkDebugMacro("Coplanar!? Unhandled!");
   // so, the intersection routine is pretty robust, and can detect coplanar cases
   // (to within some error tollerances). but it doesn't return the intersection of
   // two overlapping, coplanar triangles, it just sets a flag. the data i had while
   // developing this code had no coplanar regions ... so i never fixed this up
   // either

   return inter;
}
