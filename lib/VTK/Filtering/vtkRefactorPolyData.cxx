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
#include "vtkRefactorPolyData.h"

#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <vtkDataArray.h>
#include <vtkIntArray.h>
#include <vtkDoubleArray.h>
#include <vtkIdList.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkCleanPolyData.h>
#include <vtkMergePoints.h>
#include <vtkPolyDataNormals.h>

#include "vtkImplicitPolyData.h"
#include "vtkPolyDataIntersection.h"
#include "vtkPolyDataDistance.h"

#include <set>
#include <queue>

vtkCxxRevisionMacro(vtkRefactorPolyData, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkRefactorPolyData);

vtkRefactorPolyData::vtkRefactorPolyData() : vtkPolyDataAlgorithm()
{
   this->Tolerance = 1e-9;

   this->SetNumberOfInputPorts(2);
   this->SetNumberOfOutputPorts(3);

   vtkPolyData* output1 = vtkPolyData::New();
   this->GetExecutive()->SetOutputData(1, output1);
   output1->Delete();

   vtkPolyData* output2 = vtkPolyData::New();
   this->GetExecutive()->SetOutputData(2, output2);
   output2->Delete();
}

vtkRefactorPolyData::~vtkRefactorPolyData()
{
}

void vtkRefactorPolyData::SortPolyData(vtkPolyData* mesh, vtkPolyData* inter,
                                       vtkIdList* inList, vtkIdList* outList)
{
   int numCells = mesh->GetNumberOfCells();

   // initialize cells to -1 (unknown), later mark as inside (0), or outside (1)
   int* label = new int[numCells];
   for (int i = 0; i < numCells; i++) label[i] = -1;

   double* dist = (double*)mesh->GetPointData()->GetArray("Distance")->WriteVoidPointer(0, 0);
   double dRange[2];
   mesh->GetPointData()->GetArray("Distance")->GetRange(dRange);
   dRange[0] = fabs(dRange[0]);
   dRange[1] = fabs(dRange[1]);
   dRange[0] = dRange[0] < dRange[1] ? dRange[1] : dRange[0];

   vtkIdType npts;
   vtkIdType* pts;

   // walk thru cuts, make set of edges for faster lookup
   // have to convert points to mesh point ids ... need a point locator for that
   vtkMergePoints* locator = vtkMergePoints::New();
   vtkPoints* points = vtkPoints::New();
   locator->InitPointInsertion(points, mesh->GetBounds());
   locator->AutomaticOn();
   for (int i = 0; i < mesh->GetNumberOfPoints(); i++)
      locator->InsertUniquePoint(mesh->GetPoint(i), npts);

   std::set< std::pair<int,int> > edgeSet;
   inter->GetLines()->InitTraversal();
   while (inter->GetLines()->GetNextCell(npts, pts))
   {
      if (npts <= 1) { std::cerr << "1 point lines!" << std::endl; continue; }
      for (int i = 0; i < npts-1; i++)
      {
         int u = locator->IsInsertedPoint(inter->GetPoint(pts[i])),
             v = locator->IsInsertedPoint(inter->GetPoint(pts[(i+1)%npts]));
         edgeSet.insert(std::make_pair(u < v ? u : v, u < v ? v : u));
      }
   }
   locator->Delete();
   points->Delete();

   // build a priority queue of cells by |min dist|, which lets me easily assign in/out
   std::set< std::pair<double,int> > priQ;
   for (int i = 0; i < numCells; i++)
   {
      mesh->GetCellPoints(i, npts, pts);
      double mindist = VTK_DOUBLE_MAX;
      for (int j = 0; j < npts; j++)
      {
         int u = pts[j];
         mindist = fabs(dist[u]) < mindist ? fabs(dist[u]) : mindist;
      }
      priQ.insert(std::make_pair(dRange[0] - mindist, i));
   }

   vtkIdList* cellIds = vtkIdList::New();
   while (!priQ.empty())
   {
   // find point with max distance (regardless of sign)
   // this will only be ambiguous if the intersection is large relative to the connected component
      int cid = priQ.begin()->second;
      priQ.erase(priQ.begin());
      if (label[cid] != -1) continue;

   // determine in/out based on sign of distance @ points, put in queue
      mesh->GetCellPoints(cid, npts, pts);
      int outCount = 0, inCount = 0, onCount = 0;
      for (int i = 0; i < npts; i++)
      {
         outCount += dist[pts[i]] > 1e-6;
         inCount  += dist[pts[i]] < -1e-6;
         onCount  += abs(dist[pts[i]]) <= 1e-6;
      }

      if (outCount + onCount == npts) label[cid] = 1;
      else if (inCount + onCount == npts) label[cid] = 0;
      else
      {
         // are these the extra, non-manifold tris I don't want?
         mesh->GetCellEdgeNeighbors(cid, pts[0], pts[1], cellIds);
         if (cellIds->GetNumberOfIds() > 1) continue;
         mesh->GetCellEdgeNeighbors(cid, pts[1], pts[2], cellIds);
         if (cellIds->GetNumberOfIds() > 1) continue;
         mesh->GetCellEdgeNeighbors(cid, pts[2], pts[0], cellIds);
         if (cellIds->GetNumberOfIds() > 1) continue;

         // have to try to get a good guess ...
         // test edge neighbors & edge types (intersection, non-manifold, etc ...)
         label[cid] = outCount > inCount ? 1 : 0;
      }

      std::queue<int> waveQ;
      waveQ.push(cid);

   // while the queue is populated
      while (!waveQ.empty())
      {
   //    get top of queue
         int cid = waveQ.front();
         waveQ.pop();

         mesh->GetCellPoints(cid, npts, pts);
         for (int i = 0; i < npts; i++)
         {
            int u = pts[i], v = pts[(i+1)%npts];
            if (v < u) { int tmp = u; u = v; v = tmp; }

   //    get cell edge neighbors, if we can tell what kind of edge we're crossing, mark the neighbor
            mesh->GetCellEdgeNeighbors(cid, u, v, cellIds);
            if (cellIds->GetNumberOfIds() != 1) continue;
            for (int j = 0; j < cellIds->GetNumberOfIds(); j++)
            {
               int id = cellIds->GetId(j);
               if (label[id] == -1)
               {
                  if (edgeSet.find(std::make_pair(u, v)) != edgeSet.end())
                     label[id] = 1 - label[cid];
                  else
                     label[id] = label[cid];
                  waveQ.push(id);
               }
            }
         }
      }
   }

   cellIds->Delete();

   for (int i = 0; i < numCells; i++)
   {
      if (label[i] == 1)
         outList->InsertNextId(i);
      else if (label[i] == 0)
         inList->InsertNextId(i);
      else
         // std::cerr << i << " : unlabeled!" << std::endl;
         ; // unwanted tris
   }

   delete[] label;
}

int vtkRefactorPolyData::RequestData(vtkInformation*        vtkNotUsed(request),
                                     vtkInformationVector** inputVector,
                                     vtkInformationVector*  outputVector)
{
   vtkInformation* inInfo0 = inputVector[0]->GetInformationObject(0);
   vtkInformation* inInfo1 = inputVector[1]->GetInformationObject(0);
   vtkInformation* outInfo = outputVector->GetInformationObject(0);

   if (!inInfo0 || !inInfo1 || !outInfo) return 0;

   vtkPolyData* input0 = vtkPolyData::SafeDownCast(inInfo0->Get(vtkDataObject::DATA_OBJECT()));
   vtkPolyData* input1 = vtkPolyData::SafeDownCast(inInfo1->Get(vtkDataObject::DATA_OBJECT()));
   vtkPolyData* output0 = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
   vtkPolyData* output1 = this->GetInsideOutput();
   vtkPolyData* output2 = this->GetIntersectionOutput();

   if (!input0 || !input1 || !output0 || !output1 || !output2) return 0;

   // get intersected versions
   vtkPolyDataIntersection* pdi = vtkPolyDataIntersection::New();
   pdi->SetInput(0, input0);
   pdi->SetInput(1, input1);
   pdi->SplitFirstMeshOn();
   pdi->SplitSecondMeshOn();
   pdi->Update();

   output2->CopyStructure(pdi->GetOutput());
   output2->GetPointData()->PassData(pdi->GetOutput()->GetPointData());
   output2->GetCellData()->PassData(pdi->GetOutput()->GetCellData());

   // compute distances
   vtkPolyDataDistance* dist = vtkPolyDataDistance::New();
   dist->SetInput(0, pdi->GetFirstMeshOutput());
   dist->SetInput(1, pdi->GetSecondMeshOutput());
   dist->ComputeSecondDistanceOn();
   dist->Update();
   pdi->Delete();

   vtkPolyData* pd0 = dist->GetOutput();
   vtkPolyData* pd1 = dist->GetSecondDistanceOutput();

   pd0->BuildCells();
   pd0->BuildLinks();
   pd1->BuildCells();
   pd1->BuildLinks();

   // label sources for each cell
   vtkIntArray* label0 = vtkIntArray::New();
   label0->SetNumberOfComponents(1);
   label0->SetName("CellSource");
   label0->SetNumberOfValues(pd0->GetNumberOfCells());
   int* data = label0->GetPointer(0);
   for (int i = 0; i < pd0->GetNumberOfCells(); i++) data[i] = 1;
   pd0->GetCellData()->AddArray(label0);
   label0->Delete();

   vtkIntArray* label1 = vtkIntArray::New();
   label1->SetNumberOfComponents(1);
   label1->SetName("CellSource");
   label1->SetNumberOfValues(pd1->GetNumberOfCells());
   data = label1->GetPointer(0);
   for (int i = 0; i < pd1->GetNumberOfCells(); i++) data[i] = 2;
   pd1->GetCellData()->AddArray(label1);
   label1->Delete();

   // sort inside/outside
   vtkIdList* inList = vtkIdList::New();
   vtkIdList* outList = vtkIdList::New();

   SortPolyData(pd0, output2, inList, outList);

   vtkPolyData* temp0 = vtkPolyData::New();
   temp0->Allocate(pd0);
   temp0->GetPointData()->CopyAllocate(pd0->GetPointData());
   temp0->GetCellData()->CopyAllocate(pd0->GetCellData(), outList->GetNumberOfIds());

   vtkPolyData* temp1 = vtkPolyData::New();
   temp1->Allocate(pd0);
   temp1->GetPointData()->CopyAllocate(pd0->GetPointData());
   temp1->GetCellData()->CopyAllocate(pd0->GetCellData(), inList->GetNumberOfIds());

   temp0->CopyCells(pd0, outList);
   temp1->CopyCells(pd0, inList);

   vtkIntArray* label2 = vtkIntArray::New();
   label2->SetNumberOfComponents(1);
   label2->SetName("PointSource");
   label2->SetNumberOfValues(temp0->GetNumberOfPoints());
   data = label2->GetPointer(0);
   for (int i = 0; i < temp0->GetNumberOfPoints(); i++) data[i] = 1;

   vtkIntArray* label3 = vtkIntArray::New();
   label3->SetNumberOfComponents(1);
   label3->SetName("PointSource");
   label3->SetNumberOfValues(temp1->GetNumberOfPoints());
   data = label3->GetPointer(0);
   for (int i = 0; i < temp1->GetNumberOfPoints(); i++) data[i] = 1;

   inList->Reset();
   outList->Reset();

   SortPolyData(pd1, output2, inList, outList);

   temp0->CopyCells(pd1, outList);
   temp1->CopyCells(pd1, inList);

   for (int i = label2->GetNumberOfTuples(); i < temp0->GetNumberOfPoints(); i++) label2->InsertNextValue(2);
   for (int i = label3->GetNumberOfTuples(); i < temp1->GetNumberOfPoints(); i++) label3->InsertNextValue(2);
   temp0->GetPointData()->AddArray(label2);
   label2->Delete();
   temp1->GetPointData()->AddArray(label3);
   label3->Delete();

   outList->Delete();
   inList->Delete();

   dist->Delete();

   vtkPolyDataNormals* norm0 = vtkPolyDataNormals::New();
   norm0->SetInput(temp0);
   norm0->AutoOrientNormalsOff();
   norm0->ConsistencyOn();
   norm0->SplittingOff();
   norm0->ComputeCellNormalsOn();
   norm0->ComputePointNormalsOn();
   norm0->Update();

   output0->CopyStructure(norm0->GetOutput());
   output0->GetPointData()->PassData(norm0->GetOutput()->GetPointData());
   output0->GetCellData()->PassData(norm0->GetOutput()->GetCellData());
   norm0->Delete();

   vtkPolyDataNormals* norm1 = vtkPolyDataNormals::New();
   norm1->SetInput(temp1);
   norm1->AutoOrientNormalsOff();
   norm1->ConsistencyOn();
   norm1->SplittingOff();
   norm1->ComputeCellNormalsOn();
   norm1->ComputePointNormalsOn();
   norm1->Update();

   output1->CopyStructure(norm1->GetOutput());
   output1->GetPointData()->PassData(norm1->GetOutput()->GetPointData());
   output1->GetCellData()->PassData(norm1->GetOutput()->GetCellData());
   norm1->Delete();

   output0->Squeeze();
   output0->GetPointData()->Squeeze();
   output0->GetCellData()->Squeeze();

   output1->Squeeze();
   output1->GetPointData()->Squeeze();
   output1->GetCellData()->Squeeze();

   temp0->Delete();
   temp1->Delete();

   return 1;
}

void vtkRefactorPolyData::PrintSelf(ostream& os, vtkIndent indent)
{
   this->Superclass::PrintSelf(os,indent);
}

int vtkRefactorPolyData::FillInputPortInformation(int port, vtkInformation *info)
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

vtkPolyData* vtkRefactorPolyData::GetOutsideOutput()
{
   return vtkPolyData::SafeDownCast(this->GetExecutive()->GetOutputData(0));
}

vtkPolyData* vtkRefactorPolyData::GetInsideOutput()
{
   return vtkPolyData::SafeDownCast(this->GetExecutive()->GetOutputData(1));
}

vtkPolyData* vtkRefactorPolyData::GetIntersectionOutput()
{
   return vtkPolyData::SafeDownCast(this->GetExecutive()->GetOutputData(2));
}
