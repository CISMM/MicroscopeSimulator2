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
#include "vtkImplicitPolyData.h"

#include "vtkObjectFactory.h"
#include "vtkPolygon.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkGenericCell.h"

#include "vtkMath.h"

// Historical note: why is this class named so? I started with vtkImplicitPolyData
// from David.Pont@ForestResearch.co.nz, but slowly changed the class to the current
// version, which uses a different approach. It should probably be renamed ...

vtkCxxRevisionMacro(vtkImplicitPolyData, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkImplicitPolyData);

// Constructor
vtkImplicitPolyData::vtkImplicitPolyData()
{
	this->NoGradient[0] = 0.0;
	this->NoGradient[1] = 0.0;
	this->NoGradient[2] = 1.0;

	this->tri = NULL;
	this->input = NULL;
   this->locator = NULL;
   this->EvaluateBoundsSet=0;
   this->ReverseBias = 0;
   this->Tolerance = -1;
}

void vtkImplicitPolyData::SetInput(vtkPolyData* input)
{
   if (this->input != input)
   {
      vtkDebugMacro(<<" setting Input to " << (void *)input);

      // use a tringle filter on the polydata input
      // this is done to filter out lines and vertices to leave only
      // polygons which are required by this algorithm for cell normals
      if (this->tri == NULL)
      {
		   this->tri = vtkTriangleFilter::New();
		   this->tri->PassVertsOff();
		   this->tri->PassLinesOff();
      }
      this->tri->SetInput(input);
      this->tri->Update();

      this->input = this->tri->GetOutput();

      this->input->BuildLinks();
      this->NoValue = this->input->GetLength();
      if (this->Tolerance < 0)
	      this->Tolerance = this->NoValue * 1e-12;

      if (this->locator != NULL) this->locator->Delete();
      this->locator = vtkCellLocator::New();
      this->locator->SetDataSet(this->input);
      this->locator->SetTolerance(this->Tolerance);
      this->locator->SetNumberOfCellsPerBucket(10);
      this->locator->CacheCellBoundsOn();
      this->locator->AutomaticOn();
      this->locator->BuildLocator();
   }
}

void vtkImplicitPolyData::SetEvaluateBounds(double eBounds[6])
{
   int i;
   for(i=0; i<6; i++)
   {
      this->EvaluateBounds[i] = eBounds[i];
   }
   this->EvaluateBoundsSet = 1;
}

unsigned long vtkImplicitPolyData::GetMTime()
{
   unsigned long mTime=this->vtkImplicitFunction::GetMTime();
   unsigned long inputMTime;

   if (this->input != NULL)
   {
      this->input->Update();
      inputMTime = this->input->GetMTime();
      mTime = (inputMTime > mTime ? inputMTime : mTime);
   }

   return mTime;
}

vtkImplicitPolyData::~vtkImplicitPolyData()
{
   if (this->tri     != NULL) this->tri->Delete();
   if (this->locator != NULL) this->locator->Delete();
}

double vtkImplicitPolyData::EvaluateFunction(double x[3])
{
	double n[3];
	return sharedEvaluate(x, n);	// get distance value returned, normal not used
}

void vtkImplicitPolyData::EvaluateGradient(double x[3], double n[3])
{
	sharedEvaluate(x, n);	// get normal, returned distance value not used
}

double vtkImplicitPolyData::sharedEvaluate(double x[3], double n[3])
{
   int i;
	double ret=this->NoValue;
   for( i=0; i<3; i++ ) n[i] = this->NoGradient[i];

	// See if data set with polygons has been specified
	if (this->input == NULL || input->GetNumberOfCells() == 0)
   {
      vtkErrorMacro(<<"No polygons to evaluate function!");
    	return ret;
   }

   double p[3];
   vtkIdType cellId;
   int subId;
   double vlen2;

   vtkDataArray* cnorms = 0;
   if (this->input->GetCellData() && this->input->GetCellData()->GetNormals())
      cnorms = this->input->GetCellData()->GetNormals();

   vtkGenericCell* cell = vtkGenericCell::New();
   // get point id of closest point in data set
	this->locator->FindClosestPoint(x, p, cell, cellId, subId, vlen2);

	if (cellId != -1)	// point located
   {
      // dist = | point - x |
      ret = sqrt(vlen2);
      // grad = (point - x) / dist
      for (int i = 0; i < 3; i++) n[i] = (p[i] - x[i]) / (ret == 0. ? 1. : ret);

      double dist2, weights[3], pcoords[3], awnorm[3] = {0, 0, 0};
      cell->EvaluatePosition(p, 0, subId, pcoords, dist2, weights);

      vtkIdList* idList = vtkIdList::New();
      int count = 0;
      for (int i = 0; i < 3; i++) count += pcoords[i] == 0;
      // if pcoords contains no 0s
      if (count == 0)
      {
         // ... one face ... easy, use face normal
         if (cnorms)
         {
            cnorms->GetTuple(cellId, awnorm);
         }
         else
         {
            vtkPolygon::ComputeNormal(cell->Points, awnorm);
         }
      }
      // if pcoords contains 1 0s
      else if (count == 1)
      {
      // ... edge ... get two adjacent faces, compute average normal
         int a, b;
         if (pcoords[0] == 0)
         {
            a = cell->PointIds->GetId(1);
            b = cell->PointIds->GetId(2);
         }
         else if (pcoords[1] == 0)
         {
            a = cell->PointIds->GetId(0);
            b = cell->PointIds->GetId(2);
         }
         else
         {
            a = cell->PointIds->GetId(0);
            b = cell->PointIds->GetId(1);
         }
         this->input->GetCellEdgeNeighbors(0, a, b, idList);
         for (int i = 0; i < idList->GetNumberOfIds(); i++)
         {
            double norm[3];
            if (cnorms)
            {
               cnorms->GetTuple(idList->GetId(i), norm);
            }
            else
            {
               vtkPolygon::ComputeNormal(this->input->GetCell(idList->GetId(i))->GetPoints(), norm);
            }
            awnorm[0] += norm[0];
            awnorm[1] += norm[1];
            awnorm[2] += norm[2];
         }
         vtkMath::Normalize(awnorm);
      }
      // if pcoords contains 2 0s
      else if (count == 2)
      {
      // ... vertex ... this is the expensive case, get all adjacent faces and compute sum(a_i * n_i)
      // Angle-Weighted Pseudo Normals, J. Andreas Bærentzen and Henrik Aanæs
         int a;
         for (int i = 0; i < 3; i++)
            if (pcoords[i] == 1)
               a = cell->PointIds->GetId(i);
         this->input->GetPointCells(a, idList);
         for (int i = 0; i < idList->GetNumberOfIds(); i++)
         {
            double norm[3];
            if (cnorms)
            {
               cnorms->GetTuple(idList->GetId(i), norm);
            }
            else
            {
               vtkPolygon::ComputeNormal(this->input->GetCell(idList->GetId(i))->GetPoints(), norm);
            }
            // compute angle at point a
            int b = this->input->GetCell(idList->GetId(i))->GetPointId(0),
                c = this->input->GetCell(idList->GetId(i))->GetPointId(1);
            if (a == b)
               b = this->input->GetCell(idList->GetId(i))->GetPointId(2);
            else if (a == c)
               c = this->input->GetCell(idList->GetId(i))->GetPointId(2);

            double pa[3], pb[3], pc[3];
            this->input->GetPoint(a, pa);
            this->input->GetPoint(b, pb);
            this->input->GetPoint(c, pc);
            for (int j = 0; j < 3; j++) { pb[j] -= pa[j]; pc[j] -= pa[j]; }
            vtkMath::Normalize(pb);
            vtkMath::Normalize(pc);
            double alpha = acos(vtkMath::Dot(pb, pc));
            awnorm[0] += alpha * norm[0];
            awnorm[1] += alpha * norm[1];
            awnorm[2] += alpha * norm[2];
         }
         vtkMath::Normalize(awnorm);
      }
      idList->Delete();

      // sign(dist) = dot(grad, cell normal)
      if (ret == 0) for (int i = 0; i < 3; i++) n[i] = awnorm[i];
      ret *= (vtkMath::Dot(n, awnorm) < 0.) ? 1. : -1.;
      if (ret > 0.) for (int i = 0; i < 3; i++) n[i] = -n[i];
   }
   cell->Delete();

   return ret;
}

void vtkImplicitPolyData::PrintSelf(ostream& os, vtkIndent indent)
{
   vtkImplicitFunction::PrintSelf(os,indent);

   os << indent << "No polydata Value: " << this->NoValue << "\n";
   os << indent << "No polydata Gradient: (" << this->NoGradient[0] << ", "
      << this->NoGradient[1] << ", " << this->NoGradient[2] << ")\n";

   if (this->input)
   {
      os << indent << "Input : " << this->input << "\n";
   }
   else
   {
      os << indent << "Input : (none)\n";
   }
}

