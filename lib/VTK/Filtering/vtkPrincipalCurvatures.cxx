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

#include "vtkPrincipalCurvatures.h"

#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkCell.h>
#include <vtkDoubleArray.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkPointLocator.h>
#include <vtkPolyData.h>
#include <vtkMath.h>
#include <vtkTriangle.h>

vtkCxxRevisionMacro(vtkPrincipalCurvatures, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkPrincipalCurvatures);

vtkPrincipalCurvatures::vtkPrincipalCurvatures() : vtkPolyDataAlgorithm()
{
   this->AreaWeights = 0;
   this->ReplaceNormals = 0;
   this->FeatureScale = 1;
   this->FeatureAngle = 180;
}

void vtkPrincipalCurvatures::PrintSelf(ostream& os, vtkIndent indent)
{
   vtkPolyDataAlgorithm::PrintSelf(os, indent);
   os << indent << "ReplaceNormals: " << this->ReplaceNormals << "\n";
   os << indent << "FeatureScale: " << this->FeatureScale << "\n";
   os << indent << "FeatureAngle: " << this->FeatureAngle << "\n";
}

int vtkPrincipalCurvatures::RequestData(vtkInformation*        vtkNotUsed(request),
                                        vtkInformationVector** inputVector,
                                        vtkInformationVector*  outputVector)
{
   vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
   vtkInformation* outInfo = outputVector->GetInformationObject(0);

   if (!inInfo || !outInfo) return 0;

   vtkPolyData* input = vtkPolyData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
   vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

   if (!input || !output) return 0;

   output->CopyStructure(input);
   output->GetPointData()->PassData(input->GetPointData());
   output->GetCellData()->PassData(input->GetCellData());
   //output->GetCellData()->DeepCopy(input->GetCellData());

   this->AreaWeights = new double[output->GetNumberOfCells()];
   this->ComputeTriangleAreas(output);
   this->ComputePrincipalCurvatures(output);
   delete[] this->AreaWeights;
   this->AreaWeights = 0;

   output->Squeeze();

   return 1;
}

vtkPrincipalCurvatures::~vtkPrincipalCurvatures()
{
   if (this->AreaWeights) delete[] this->AreaWeights;
}

void vtkPrincipalCurvatures::ComputePrincipalCurvatures(vtkPolyData* mesh)
{
   vtkDebugMacro(<<"Start vtkPrincipalCurvatures::ComputePrincipalCurvatures");

   mesh->BuildCells();
   mesh->BuildLinks();

   // For each vert Vi
   int numPoints = mesh->GetPoints()->GetNumberOfPoints();

   // control for large angle differences between triangles
   double cosFeatureAngle = cos(vtkMath::RadiansFromDegrees(this->FeatureAngle));
   double featureSize = 0;
   for (int i = 0; i < mesh->GetNumberOfCells(); i++)
   {
      if (mesh->GetCellType(i) == VTK_TRIANGLE)
         featureSize += mesh->GetCell(i)->GetLength2();
   }
   // control size of neighborhood used
   featureSize = this->FeatureScale * 2.5
	       * sqrt(featureSize / (double)mesh->GetNumberOfCells());

   vtkDoubleArray* pmin = vtkDoubleArray::New();
   pmin->SetName("Min_Curvature");
   pmin->SetNumberOfComponents(1);
   pmin->SetNumberOfTuples(numPoints);

   vtkDoubleArray* pmax = vtkDoubleArray::New();
   pmax->SetName("Max_Curvature");
   pmax->SetNumberOfComponents(1);
   pmax->SetNumberOfTuples(numPoints);

   vtkDoubleArray* norm = vtkDoubleArray::New();
   norm->SetName("Normals");
   norm->SetNumberOfComponents(3);
   norm->SetNumberOfTuples(numPoints);

   vtkDoubleArray* pdirmin = vtkDoubleArray::New();
   pdirmin->SetName("Min_Curvature_Direction");
   pdirmin->SetNumberOfComponents(3);
   pdirmin->SetNumberOfTuples(numPoints);

   vtkDoubleArray* pdirmax = vtkDoubleArray::New();
   pdirmax->SetName("Max_Curvature_Direction");
   pdirmax->SetNumberOfComponents(3);
   pdirmax->SetNumberOfTuples(numPoints);

   vtkIdList* ringN0 = vtkIdList::New();
   vtkIdList* ringN1 = vtkIdList::New();
   for (int i = 0; i < numPoints; i++)
   {
      this->UpdateProgress(.5 + .5 * (double)i / (double)numPoints);

      ringN0->Reset();
      ringN0->InsertNextId(i);

      double vi[3], ni[3];
      mesh->GetPoint(i, vi);
      mesh->GetPointData()->GetNormals()->GetTuple(i, ni);

      ringN1->Reset();
      mesh->GetCellNeighbors(-1, ringN0, ringN1);
      ringN0->Reset();

      vtkIdType npts;
      vtkIdType *pts;
      for (int j = 0; j < ringN1->GetNumberOfIds(); j++)
      {
         int id = ringN1->GetId(j);
         mesh->GetCellPoints(id, npts, pts);
         for (int k = 0; k < npts; k++) if (pts[k] != i) ringN0->InsertUniqueId(pts[k]);
      }

      double *w = new double[ringN0->GetNumberOfIds()];
      double sa = 0;
      double n[] = {0, 0, 0};

//    for j in neighborhood around i - compute wj, smooth ni, compute sai
      vtkIdList* cellList = vtkIdList::New();
      for (int j = 0; j < ringN0->GetNumberOfIds(); j++)
      {
         w[j] = 0;
//       if nj dot ni > some angle requirement
         double vj[3], nj[3];
         mesh->GetPoint(ringN0->GetId(j), vj);
         mesh->GetPointData()->GetNormals()->GetTuple(ringN0->GetId(j), nj);

         if (fabs(vtkMath::Dot(ni, nj)) > cosFeatureAngle)
         {
//          c <- cells including vj
            mesh->GetPointCells(ringN0->GetId(j), cellList);
//          wj <- surface area of c
            for (int k = 0; k < cellList->GetNumberOfIds(); k++)
            {
               w[j] += this->AreaWeights[cellList->GetId(k)];
            }
            double falloff = (featureSize-sqrt(vtkMath::Distance2BetweenPoints(vi, vj))) / featureSize;
            w[j] = falloff*falloff*falloff;
//          sa += wj // sum of all considered weights
            sa += w[j];
//          n += wj * nj;
            n[0] += w[j] * nj[0]; n[1] += w[j] * nj[1]; n[2] += w[j] * nj[2];
         }
      }
      cellList->Delete();

//    normalize n
      vtkMath::Normalize(n);

      double m[3][3];
      for (int j = 0; j < 3; j++) for (int i = 0; i < 3; i++) m[j][i] = 0;

//    for j in neighborhood around i
      for (int j = 0; j < ringN0->GetNumberOfIds(); j++)
      {
//       if nj dot ni > some angle requirement
         double nj[3];
         mesh->GetPointData()->GetNormals()->GetTuple(ringN0->GetId(j), nj);
         if ((i != ringN0->GetId(j)) && (fabs(vtkMath::Dot(n, nj)) > cosFeatureAngle))
         {
//          compute contribution at vi of tangent space projection of vjvi
//             Compute Tj = ViVj - projection of ViVj onto Ni
//             Compute Kj = 2 Ni dot ViVj / ViVj dot ViVj
            double vj[3];
            mesh->GetPoint(ringN0->GetId(j), vj);
            double vivj[] = {vj[0]-vi[0], vj[1]-vi[1], vj[2]-vi[2]},
                   dvv2 = vtkMath::Dot(vivj, vivj), dvn = vtkMath::Dot(vivj, n),
                   t[] = {vivj[0]-dvn*n[0], vivj[1]-dvn*n[1], vivj[2]-dvn*n[2]},
                   k = 2 * dvn / dvv2, wk = k * w[j] / sa;

            vtkMath::Normalize(t);

//          M += Wj Kj Tj * Tj
            double a = wk * t[0]*t[0], b = wk * t[0]*t[1], c = wk * t[0]*t[2],
                                       d = wk * t[1]*t[1], e = wk * t[1]*t[2],
                                                           f = wk * t[2]*t[2]; 

            m[0][0] += a; m[0][1] += b; m[0][2] += c;
            m[1][0] += b; m[1][1] += d; m[1][2] += e;
            m[2][0] += c; m[2][1] += e; m[2][2] += f;
         }
      }

//    Compute Eigen values EW & vectors EV of M
      double ew[3], ev[3][3], *pm[3], *pev[3];

      pm[0] = m[0]; pm[1] = m[1]; pm[2] = m[2];
      pev[0] = ev[0]; pev[1] = ev[1]; pev[2] = ev[2];

      vtkMath::Jacobi(pm, ew, pev);
      vtkMath::Transpose3x3(ev, m);

      int p[] = {0, 1, 2};
/*
//    Sort EV & EW ... this shouldn't be needed, but I was being overly
//    cautious when I wrote this
      if (fabs(vtkMath::Dot(ni, m[p[2]])) < fabs(vtkMath::Dot(ni, m[p[0]])))
      {
         int t = p[2];
         p[2] = p[0];
         p[0] = t;
      }
      if (fabs(vtkMath::Dot(ni, m[p[2]])) < fabs(vtkMath::Dot(ni, m[p[1]])))
      {
         int t = p[2];
         p[2] = p[1];
         p[1] = t;
      }
      if (fabs(ew[p[0]]) < fabs(ew[p[1]]))
      {
         int t = p[1];
         p[1] = p[0];
         p[0] = t;
      }
*/
//    Check if need to flip signs (EV2dotNi < 0)
      if (vtkMath::Dot(ni, m[p[2]]) < 0)
      {
         ew[p[2]] = -ew[p[2]];
         for (int j = 0; j < 3; j++)
         {
            m[p[j]][0] = -m[p[j]][0];
            m[p[j]][1] = -m[p[j]][1];
            m[p[j]][2] = -m[p[j]][2];
         }
      }

//    Scalars <- EW
      pmax->SetTuple1(i, ew[p[0]]);
      pmin->SetTuple1(i, ew[p[1]]);
//    Vectors <- EV0,1
      pdirmax->SetTuple3(i, m[p[0]][0], m[p[0]][1], m[p[0]][2]);
      pdirmin->SetTuple3(i, m[p[1]][0], m[p[1]][1], m[p[1]][2]);
//    Normals <- EV2
      norm->SetTuple3(i, m[p[2]][0], m[p[2]][1], m[p[2]][2]);

      delete[] w;
   }
   ringN0->Delete();
   ringN1->Delete();

   if (this->ReplaceNormals)
   {
      mesh->GetPointData()->SetNormals(norm);
   }

   mesh->GetPointData()->AddArray(pmin);
   mesh->GetPointData()->AddArray(pmax);
   mesh->GetPointData()->AddArray(pdirmin);
   mesh->GetPointData()->AddArray(pdirmax);
   mesh->GetPointData()->SetActiveScalars(pmax->GetName());
   mesh->GetPointData()->SetActiveVectors(pdirmax->GetName());

   norm->Delete();
   pmin->Delete();
   pmax->Delete();
   pdirmin->Delete();
   pdirmax->Delete();

   mesh->DeleteLinks();

   vtkDebugMacro(<<"End vtkPrincipalCurvatures::ComputePrincipalCurvatures");
}

void vtkPrincipalCurvatures::ComputeTriangleAreas(vtkPolyData* mesh)
{
   vtkDebugMacro(<<"Start vtkPrincipalCurvatures::ComputeTriangleAreas");

   int numCells = mesh->GetNumberOfCells(),
       cellId;

   double x[3][3];

   vtkIdList* ptIds = vtkIdList::New();
   for (cellId = 0; cellId < numCells; cellId++)
   {
      this->UpdateProgress(.5 * (double)cellId / (double)numCells);

      mesh->GetCellPoints(cellId, ptIds);
      if (ptIds->GetNumberOfIds() < 3) continue;

      // store current vertex (x,y,z) coordinates ...
      //
      for (int i = 0; i < 3; i++) mesh->GetPoint(ptIds->GetId(i), x[i]);
      this->AreaWeights[cellId] = vtkTriangle::TriangleArea(x[0], x[1], x[2]);
   }
   ptIds->Delete();
   vtkDebugMacro(<<"End vtkPrincipalCurvatures::ComputeTriangleAreas");
}
