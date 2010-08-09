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
#include "vtkPolyDataTexturizer.h"

#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkPoints.h>
#include <vtkPriorityQueue.h>
#include <vtkIdList.h>
#include <vtkGenericCell.h>
#include <vtkDoubleArray.h>
#include <vtkMath.h>
#include <vtkCellArray.h>
#include <vtkLine.h>

#include "vtkPolyDataUtilities.h"

#include <set>
#include <map>

vtkCxxRevisionMacro(vtkPolyDataTexturizer, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkPolyDataTexturizer);

vtkPolyDataTexturizer::vtkPolyDataTexturizer() : vtkPolyDataAlgorithm()
{
}

vtkPolyDataTexturizer::~vtkPolyDataTexturizer()
{
}

int vtkPolyDataTexturizer::RequestData(vtkInformation*        vtkNotUsed(request),
                                       vtkInformationVector** inputVector,
                                       vtkInformationVector*  outputVector)
{
   vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
   vtkInformation* outInfo = outputVector->GetInformationObject(0);

   if (!inInfo || !outInfo) return 0;

   vtkPolyData* input = vtkPolyData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
   vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

   if (!input || !output) return 0;

   vtkPolyData* mesh = vtkPolyData::New();
   mesh->CopyStructure(input);
   mesh->GetPointData()->PassData(input->GetPointData());
   mesh->GetCellData()->PassData(input->GetCellData());

   mesh->BuildCells();
   mesh->BuildLinks();

   vtkDoubleArray* geodesic = vtkDoubleArray::New();
   geodesic->SetName("Geodesic");
   geodesic->SetNumberOfComponents(1);
   geodesic->SetNumberOfTuples(mesh->GetNumberOfPoints());
   double* geo = geodesic->GetPointer(0);

   int* pointRegion = new int[mesh->GetNumberOfPoints()];

   double target = SeedTexturePatches(mesh, geo, pointRegion);
   geodesic->Modified();

   vtkIntArray* patch = vtkIntArray::New();
   patch->SetName("PatchId");
   patch->SetNumberOfComponents(1);
   patch->SetNumberOfTuples(mesh->GetNumberOfCells());
   patch->FillComponent(0, -1.);
   int* region = patch->GetPointer(0);

   GrowTexturePatches(mesh, geo, pointRegion, region);

   delete[] pointRegion;

   mesh->GetPointData()->AddArray(geodesic);
   mesh->GetCellData()->AddArray(patch);

   geodesic->Delete();
   patch->Delete();
   mesh->Squeeze();

   CutTexturePatches(output, mesh);
   mesh->Delete();

   ParameterizeTexturePatches(output, target * .95);
   output->Squeeze();

   output->GetPointData()->SetActiveTCoords("PCoords");

   return 1;
}

// initialze shortest geodesic distance from multiple start points
int InitMultiSourceGeodesic(vtkPolyData* const mesh, std::set<int>& srcs, double* geodesic, int* region, double target)
{
   int numPts = mesh->GetNumberOfPoints();
   for (int i = 0; i < numPts; i++)
   {
      geodesic[i] = VTK_DOUBLE_MAX;
      region[i] = -1;
   }

   vtkPriorityQueue* queue = vtkPriorityQueue::New();
   if (srcs.empty())
   {
      queue->Insert(0., 0);
      geodesic[0] = 0.;
      region[0] = 0;
   }
   else
   {
      std::set<int>::iterator it = srcs.begin();
      while (it != srcs.end())
      {
         queue->Insert(target, *it);
         geodesic[*it] = target;
         region[*it] = *it;
         it++;
      }
   }

   int u = CalcGeodesic(mesh, queue, geodesic, 0, region);
   queue->Delete();

   return u;
}

// update shortest geodesic distance from multiple start points by adding new start point
int UpdateMultiSourceGeodesic(vtkPolyData* const mesh, int src, double* geodesic, int* region)
{
   vtkPriorityQueue* queue = vtkPriorityQueue::New();
   queue->Insert(0., src);
   geodesic[src] = 0.;
   region[src] = src;

   int u = CalcGeodesic(mesh, queue, geodesic, 0, region);

   queue->Delete();

   return u;
}

// use geodesic distances to compute voronoi like partitioning of surface into patches
double vtkPolyDataTexturizer::SeedTexturePatches(vtkPolyData* output, double* geo, int* pointRegion)
{
   std::deque<PDGeometryInfo> geomInfo;
   GetPolyDataGeometryInfo(output, geomInfo);

   std::set<int> srcs;
   for (unsigned int i = 0; i < geomInfo.size(); i++)
   {
      PDGeometryInfo::LoopSetIter it;
      for (it = geomInfo[i].boundaryLoops.begin(); it != geomInfo[i].boundaryLoops.end(); it++)
      {
         while (!it->empty())
         {
            srcs.insert(it->front());
            it->pop_front();
         }
      }
   }

   double target = output->GetLength() * .03;
   double maxlen = 0.;
   for (int i = 0; i < output->GetNumberOfCells(); i++)
   {
      double len = sqrt(output->GetCell(i)->GetLength2());
      maxlen = (maxlen < len) ? len : maxlen;
   }
   maxlen *= 1.414;
   target = (target < maxlen) ? maxlen : target;

   int next = InitMultiSourceGeodesic(output, srcs, geo, pointRegion, target * .9);

   if (srcs.empty())
      srcs.insert(0);
   else
      srcs.clear();

   double g = geo[next];
   while (g >= target)
   {
      srcs.insert(next);
      next = UpdateMultiSourceGeodesic(output, next, geo, pointRegion);
      g = 0.;//geo[next];

      if (g < target) // could be sped up if a priority queue of distances >= target were kept
      {
         for (int i = 0; i < output->GetNumberOfPoints(); i++)
         {
            double ng = geo[i];
            g = (g < ng) ? ng : g;
            next = (g == ng) ? i : next;
            //if (g >= target) break;
         }
      }
   }

   return target;
}

// identify which cells belong to which texture patches
// border cells with points labeled with multiple seeds makes this non-trivial
void vtkPolyDataTexturizer::GrowTexturePatches(vtkPolyData* output, double* geo, int* pointRegion, int* region)
{
   // if closest source is the same as the majority source, select that label
   for (int i = 0; i < output->GetNumberOfCells(); i++)
   {
      vtkIdType npts;
      vtkIdType* pts;
      output->GetCellPoints(i, npts, pts);

      bool unanimous = true;
      double mingeo = VTK_DOUBLE_MAX;
      int minreg = -1;
      for (int j = 0; j < npts; j++)
      {
         unanimous &= pointRegion[pts[j]] == pointRegion[pts[(j+1)%npts]];
         mingeo = (mingeo >= geo[pts[j]]) ? geo[pts[j]] : mingeo;
         minreg = (mingeo == geo[pts[j]]) ? pointRegion[pts[j]] : minreg;
      }
      if (unanimous) region[i] = minreg;
   }

   // some stragglers left
   vtkIdList* cellIds = vtkIdList::New();
   bool fixup = true;
   std::map<int,int>::iterator it;
   std::map<int,int> assign;
   while (fixup)
   {
      fixup = false;
   // pick the majority source cell's edge neighbors (unlabeled and boundary are non-votes)
      for (int i = 0; i < output->GetNumberOfCells(); i++)
      {
         if (region[i] != -1) continue;

         vtkIdType npts;
         vtkIdType* pts;
         output->GetCellPoints(i, npts, pts);

         std::map<int,int> counts;
         for (int j = 0; j < npts; j++)
         {
            vtkIdType u = pts[j], v = pts[(j+1)%npts];
            output->GetCellEdgeNeighbors(i, u, v, cellIds);
            for (int k = 0; k < cellIds->GetNumberOfIds(); k++) counts[region[cellIds->GetId(k)]]++;
         }
         int majority = -1;
         int maxCount = 0;
         for (it = counts.begin(); it != counts.end(); it++)
         {
            if (it->first == -1) continue;
            maxCount = (it->second > maxCount) ? it->second : maxCount;
            majority = (it->second == maxCount) ? it->first : majority;
         }
         if (majority != -1 && maxCount > 1)
         {
            assign[i] = majority;
            fixup = true;
         }
      }
      for (std::map<int,int>::iterator it = assign.begin(); it != assign.end(); it++)
         region[it->first] = it->second;
      assign.clear();
   }

   fixup = true;
   while(fixup)
   {
      fixup = false;
   // pick edge neighbor with lowest geodesic sum
      for (int i = 0; i < output->GetNumberOfCells(); i++)
      {
         if (region[i] != -1) continue;

         vtkIdType npts;
         vtkIdType* pts;
         output->GetCellPoints(i, npts, pts);

         std::map<double,int> sums;
         for (int j = 0; j < npts; j++)
         {
            vtkIdType u = pts[j], v = pts[(j+1)%npts];
            output->GetCellEdgeNeighbors(i, u, v, cellIds);
            for (int k = 0; k < cellIds->GetNumberOfIds(); k++)
            {
               if (region[cellIds->GetId(k)] == -1) continue;
               sums[(geo[u]+geo[v])] = region[cellIds->GetId(k)];
            }
         }
         if (!sums.empty())
         {
            assign[i] = sums.begin()->second;
            fixup = true;
         }
      }
      for (std::map<int,int>::iterator it = assign.begin(); it != assign.end(); it++)
         region[it->first] = it->second;
      assign.clear();
   }

#define PATCH_BOUNDARY_FLIPPING
#ifdef PATCH_BOUNDARY_FLIPPING
   for (int i = 0; i < output->GetNumberOfCells(); i++)
   {
      vtkIdType npts;
      vtkIdType* pts;
      output->GetCellPoints(i, npts, pts);

      std::map<int,int> counts;
      for (int j = 0; j < npts; j++)
      {
         vtkIdType u = pts[j], v = pts[(j+1)%npts];
         output->GetCellEdgeNeighbors(i, u, v, cellIds);
         for (int k = 0; k < cellIds->GetNumberOfIds(); k++) counts[region[cellIds->GetId(k)]]++;
      }
      int majority = -1;
      int maxCount = 0;
      for (it = counts.begin(); it != counts.end(); it++)
      {
         maxCount = (it->second > maxCount) ? it->second : maxCount;
         majority = (it->second == maxCount) ? it->first : majority;
      }
      if (majority != region[i] && maxCount > 1)
         region[i] = majority;
   }
#endif

   cellIds->Delete();
}

// insert new geometry to separate the patches from eachother
void vtkPolyDataTexturizer::CutTexturePatches(vtkPolyData* output, vtkPolyData* input)
{
   int numCells = input->GetNumberOfCells();
   output->Allocate(input, input->GetNumberOfCells());

   vtkPoints* newPoints = vtkPoints::New();
   output->SetPoints(newPoints);
   newPoints->Delete();

   output->GetPointData()->CopyAllocate(input->GetPointData());
   output->GetCellData()->CopyAllocate(input->GetCellData());

   std::map<std::pair<int,int>,int> pointMap; // <<original point id, cell region>, new point id>

#undef DEBUG_PATCH_COPYING
#ifdef DEBUG_PATCH_COPYING
   std::set<int> pids;
   std::set<int> zeros;
   for (int i = 0; i < input->GetNumberOfCells(); i++)
   {
      int pid = input->GetCellData()->GetArray("PatchId")->GetTuple1(i);
      pids.insert(pid);
   }
   for (int i = 0; i < input->GetNumberOfPoints(); i++)
   {
      double geo = input->GetPointData()->GetArray("Geodesic")->GetTuple1(i);
      if (geo == 0.) zeros.insert(i);
   }
   std::cerr << pids.size() << " :: " << zeros.size() << std::endl;
   while (pids.size() && zeros.size())
   {
      std::cerr << *pids.begin() << " : " << *zeros.begin() << std::endl;
      pids.erase(pids.begin());
      zeros.erase(zeros.begin());
   }
   std::cerr << pids.size() << " :: " << zeros.size() << std::endl;
#endif

   for (int i = 0; i < numCells; i++)
   {
      int reg = input->GetCellData()->GetArray("PatchId")->GetTuple1(i);

      vtkIdType npts;
      vtkIdType* pts;
      input->GetCellPoints(i, npts, pts);

      vtkIdType newpts[3];
      for (int j = 0; j < npts; j++)
      {
         std::pair<int,int> p = std::make_pair(pts[j], reg);
         if (pointMap.find(p) == pointMap.end())
         {
            int pid = output->GetPoints()->InsertNextPoint(input->GetPoint(pts[j]));
            pointMap[p] = pid;            
            output->GetPointData()->CopyData(input->GetPointData(), pts[j], pid);
         }
         newpts[j] = pointMap[p];
      }

      output->InsertNextCell(VTK_TRIANGLE, 3, newpts);
      output->GetCellData()->CopyData(input->GetCellData(), i, i);
   }

   for (int i = 0; i < numCells; i++)
   {
      int pid = output->GetCellData()->GetArray("PatchId")->GetTuple1(i);
      output->GetCellData()->GetArray("PatchId")->SetTuple1(i, pointMap[std::make_pair(pid, pid)]);
   }

#ifdef DEBUG_PATCH_COPYING
   pids.clear();
   zeros.clear();
   for (int i = 0; i < output->GetNumberOfCells(); i++)
   {
      int pid = output->GetCellData()->GetArray("PatchId")->GetTuple1(i);
      pids.insert(pid);
   }
   for (int i = 0; i < output->GetNumberOfPoints(); i++)
   {
      double geo = output->GetPointData()->GetArray("Geodesic")->GetTuple1(i);
      if (geo == 0.) zeros.insert(i);
   }
   std::cerr << pids.size() << " :: " << zeros.size() << std::endl;
   while (pids.size() && zeros.size())
   {
      std::cerr << *pids.begin() << " : " << *zeros.begin() << std::endl;
      pids.erase(pids.begin());
      zeros.erase(zeros.begin());
   }
   std::cerr << pids.size() << " :: " << zeros.size() << std::endl;
#endif
}

class state
{
   public:
      double geo;
      int point;
      int tri;
      int e0;
      int e1;
      state() : geo(VTK_DOUBLE_MAX), point(-1), tri(-1), e0(-1), e1(-1) {}
      state(const state& s) : geo(s.geo), point(s.point), tri(s.tri), e0(s.e0), e1(s.e1) {}
      state(double g, int p, int t, int p0, int p1) : geo(g), point(p), tri(t), e0(p0), e1(p1) {}
      inline state* operator=(const state& s)
      {
         if (this == &s) return this;
         geo = s.geo;
         point = s.point;
         tri = s.tri;
         e0 = s.e0;
         e1 = s.e1;
         return this;
      }
      inline bool operator<(const state& s) const
      {
         return (geo < s.geo) || ((geo == s.geo) && point < s.point);
      }
      inline bool operator==(const state& s) const
      {
         return (point == s.point) && (tri == s.tri);
      }
};

// compute a locally linear embedding (LLE) on each patch as the texture coordinates
void UnfoldPatch(vtkPolyData* mesh, int src, double* tc, double target = 1.0)
{
#undef DEBUG_PATCH_TIMING
#ifdef DEBUG_PATCH_TIMING
   wxStopWatch sw;
#endif
   double r_target = 1./target;

   // starting with the src, get adjacent points until we've identified all points in the patch
   // map them to 0-(n-1) as well
   std::set<int> visited;
   std::map<int,int> pointMap;
   std::deque<int> queue;
   queue.push_back(src);
   vtkIdList* list = vtkIdList::New();
   while (!queue.empty())
   {
      int u = queue.front();
      queue.pop_front();

      if (visited.find(u) != visited.end()) continue;
      pointMap[(int)visited.size()] = u;
      visited.insert(u);

      GetAdjacentPoints(mesh, u, list);
      for (int i = 0; i < list->GetNumberOfIds(); i++)
      {
         u = list->GetId(i);
         if (visited.find(u) == visited.end()) queue.push_back(u);
      }
   }
#ifdef DEBUG_PATCH_TIMING
   sw.Pause();
   std::cout << "Patch time: " << sw.Time() << " ms" << std::endl;
   std::cout << "Patch size: " << visited.size() << " points" << std::endl;
   sw.Start();
#endif

   int sizeD = (int)pointMap.size();
   double** D = new double*[sizeD];
   double** v = new double*[sizeD];
   for (int i = 0; i < sizeD; i++)
   {
      D[i] = new double[sizeD];
      v[i] = new double[sizeD];
   }

   // I want a geodesic distance here, but a) using the one in PolyDataUtilities would be
   // pretty inefficient because I need all sources distance ...
   for (int i = 0; i < sizeD; i++)
   {
      int pMi = pointMap[i];
      D[i][i] = 0.;
      for (int j = i+1; j < sizeD; j++)
      {
         int pMj = pointMap[j];
         D[i][j] = D[j][i] = VTK_DOUBLE_MAX;
         if (mesh->IsEdge(pMi, pMj))
         {
            double pi[3], pj[3];
            mesh->GetPoint(pMi, pi);
            mesh->GetPoint(pMj, pj);
            double dij = sqrt(vtkMath::Distance2BetweenPoints(pi, pj));
            D[i][j] = dij;
            D[j][i] = dij;
         }
         else
         {
            double px[3], pv[3];
            mesh->GetPoint(pMi, px);
            mesh->GetPoint(pMj, pv);

            GetAdjacentPoints(mesh, pMi, list);
            vtkIdList* list2 = vtkIdList::New();
            GetAdjacentPoints(mesh, pMj, list2);
            list->IntersectWith(*list2);
            for (int _k = 0; _k < list->GetNumberOfIds()-1; _k++)
            {
               int k = list->GetId(_k);
               double pu[3];
               mesh->GetPoint(k, pu);
               for (int _l = _k; _l < list->GetNumberOfIds(); _l++)
               {
                  int l = list->GetId(_l);
                  if (!mesh->IsTriangle(pMi, l, k) || !mesh->IsTriangle(pMj, l, k)) continue;
              
                  double pw[3];
                  mesh->GetPoint(l, pw);

                  // check vuw+wux < 180.
                  double vu[3] = {pv[0]-pu[0],pv[1]-pu[1],pv[2]-pu[2]},
                         wu[3] = {pw[0]-pu[0],pw[1]-pu[1],pw[2]-pu[2]},
                         xu[3] = {px[0]-pu[0],px[1]-pu[1],px[2]-pu[2]},
                         lvu = vtkMath::Normalize(vu),
                         //lwu = vtkMath::Normalize(wu),
                         lxu = vtkMath::Normalize(xu),
                         acosvuw = acos(vtkMath::Dot(vu,wu)),
                         acoswux = acos(vtkMath::Dot(wu,xu));

                  if (acosvuw + acoswux - vtkMath::Pi() <= 0.)
                  {
                     // check vwu+uwx < 180.
                     double vw[3] = {pv[0]-pw[0],pv[1]-pw[1],pv[2]-pw[2]},
                            xw[3] = {px[0]-pw[0],px[1]-pw[1],px[2]-pw[2]},
                            uw[3] = {-wu[0],-wu[1],-wu[2]};
                     vtkMath::Normalize(vw);
                     vtkMath::Normalize(xw);
                     double acosvwu = acos(vtkMath::Dot(vw,uw)),
                            acosuwx = acos(vtkMath::Dot(uw,xw));

                     // lvu*lvu + lxu*lxu - 2.*lvu*lxu*(vu.xu)
                     if (acosvwu + acosuwx - vtkMath::Pi() <= 0.)
                     {
                        double dij = sqrt(lvu*lvu + lxu*lxu - 2.*lvu*lxu*vtkMath::Dot(vu,xu));
                        D[i][j] = dij;
                        D[j][i] = dij;
                     }
                  }
               }
            }
            list2->Delete();
         }
      }
   }
#ifdef DEBUG_PATCH_TIMING
   sw.Pause();
   std::cout << "Init time: " << sw.Time() << " ms" << std::endl;
   sw.Start();
#endif

   // floyd-warshall magic ...
   for (int i = 0; i < sizeD; i++)
      for (int j = i; j < sizeD; j++)
         for (int k = 0; k < sizeD; k++)
            if (D[i][j] > D[i][k] + D[k][j]) D[j][i] = D[i][j] = D[i][k] + D[k][j];
#ifdef DEBUG_PATCH_TIMING
   sw.Pause();
   std::cout << "Fill time: " << sw.Time() << " ms" << std::endl;
   sw.Start();
#endif

   // Double centering of distance matrix
   double gMean = 0.;
   double* rcMean = new double[sizeD];
   for (int i = 0; i < sizeD; i++)
   {
      rcMean[i] = 0.;
      for (int j = 0; j < sizeD; j++) rcMean[i] += D[i][j]*D[i][j];
      rcMean[i] /= (double)sizeD;
      gMean += rcMean[i];
   }
   gMean /= (double)sizeD;

   for (int i = 0; i < sizeD; i++)
   {
      for (int j = 0; j < sizeD; j++)
      {
         D[i][j] = -.5 * (D[i][j]*D[i][j] - rcMean[i] - rcMean[j] + gMean);
      }
   }
   delete[] rcMean;
#ifdef DEBUG_PATCH_TIMING
   sw.Pause();
   std::cout << "Double-center time: " << sw.Time() << " ms" << std::endl;
   sw.Start();
#endif

   double* w = new double[sizeD];
   vtkMath::JacobiN(D, sizeD, w, v);
#ifdef DEBUG_PATCH_TIMING
   sw.Pause();
   std::cout << "Jacobi time: " << sw.Time() << " ms" << std::endl;
   sw.Start();
#endif

   double xOrig = sqrt(w[0]) * v[0][0],
          yOrig = sqrt(w[1]) * v[0][1];
   for (int i = 0; i < sizeD; i++)
   {
      int u = pointMap[i];
      tc[u*2 + 0] = (sqrt(w[0]) * v[i][0] - xOrig) * r_target;
      tc[u*2 + 1] = (sqrt(w[1]) * v[i][1] - yOrig) * r_target;
   }
#ifdef DEBUG_PATCH_TIMING
   sw.Pause();
   std::cout << "Parameterizer time: " << sw.Time() << " ms" << std::endl;
#endif
   // if curvature directions were already defined, use them to align the texture coordinates
   if (mesh->GetPointData()->GetArray("Max_Curvature_Direction"))
   {
      double pd1[3], pd2[3];
      mesh->GetPointData()->GetArray("Max_Curvature_Direction")->GetTuple(src, pd1);
      mesh->GetPointData()->GetArray("Min_Curvature_Direction")->GetTuple(src, pd2);

      // pick an adjacent point (try largest geodesic distance to minimize numerical error)
      GetAdjacentPoints(mesh, src, list);
      double maxgeo = VTK_DOUBLE_MAX;
      int maxu = -1;
      for (int i = 0; i < list->GetNumberOfIds(); i++)
      {
         int u = list->GetId(i);
         double geo = mesh->GetPointData()->GetArray("Geodesic")->GetTuple1(u);
         maxgeo = (geo < maxgeo) ? geo : maxgeo;
         maxu = (geo == maxgeo) ? u : maxu;
      }

      // direc <- direction vector
      double psrc[3], pmaxu[3];
      mesh->GetPoint(src, psrc);
      mesh->GetPoint(maxu, pmaxu);
      double direc[3] = { pmaxu[0] - psrc[0], pmaxu[1] - psrc[1], pmaxu[2] - psrc[2] };

      // param <- direction vector in parameter space
      double param[3] = { tc[maxu*2 + 0], tc[maxu*2 + 1], 0. };
      vtkMath::Normalize(param);

      // curve <- compute param from curvatures (see texmap2curvatures)
      double curve[3] = { vtkMath::Dot(direc, pd2), vtkMath::Dot(direc, pd1), 0. };
      vtkMath::Normalize(curve);

      // rotate <- compute rotation from param to curve
      double cospc = vtkMath::Dot(curve, param),
             sinpc = sqrt(1. - cospc*cospc),
             cross[3];

      vtkMath::Cross(curve, param, cross);
      sinpc = cross[2] < 0. ? -sinpc : (cross[2] > 0. ? sinpc : 0.);

      // apply rotate to all points in patch
      for (int i = 0; i < sizeD; i++)
      {
         int u = pointMap[i];
         double temp[2] = {  cospc * tc[u*2 + 0] + sinpc * tc[u*2 + 1],
                            -sinpc * tc[u*2 + 0] + cospc * tc[u*2 + 1] };
         tc[u*2 + 0] = temp[0];
         tc[u*2 + 1] = temp[1];
      }
   }
   // if they aren't defined, it would be most efficient to only compute them for the seed
   // TODO: implement that

   for (int i = 0; i < sizeD; i++)
   {
      int u = pointMap[i];
      tc[u*2 + 0] += .5;
      tc[u*2 + 1] += .5;
   }

   delete[] w;
   for (int i = 0; i < sizeD; i++)
   {
      delete[] D[i];
      delete[] v[i];
   }
   delete[] D;
   delete[] v;

   list->Delete();
}

void vtkPolyDataTexturizer::ParameterizeTexturePatches(vtkPolyData* mesh, double target)
{
   int* patch = (int*)(mesh->GetCellData()->GetArray("PatchId")->WriteVoidPointer(0, 0));

   mesh->BuildCells();
   mesh->BuildLinks();

   int numPoints = mesh->GetNumberOfPoints(),
       numCells = mesh->GetNumberOfCells();

   vtkDoubleArray* tcoords = vtkDoubleArray::New();
   tcoords->SetName("PCoords");
   tcoords->SetNumberOfComponents(2);
   tcoords->SetNumberOfTuples(numPoints);

   std::set<int> unique;
   for (int i = 0; i < numCells; i++)
   {
      if (unique.find(patch[i]) == unique.end())
      {
         unique.insert(patch[i]);
         UnfoldPatch(mesh, patch[i], tcoords->GetPointer(0), target);
      }
   }

   mesh->GetPointData()->AddArray(tcoords);

   std::deque<PDGeometryInfo> patches;
   GetPolyDataGeometryInfo(mesh, patches);
   PDGeometryInfo::LoopSet allLoops;
   for (unsigned int i = 0; i < patches.size(); i++)
   {
      while (patches[i].boundaryLoops.size())
      {
         allLoops.insert(allLoops.end(), *(patches[i].boundaryLoops.begin()));
         patches[i].boundaryLoops.erase(patches[i].boundaryLoops.begin());
      }
   }

   mesh->DeleteCells();
   mesh->DeleteLinks();
   tcoords->Delete();
}
