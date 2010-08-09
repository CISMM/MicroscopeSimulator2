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
#include "vtkPolyDataUtilities.h"

#include <vtkGenericCell.h>
#include <vtkEdgeTable.h>
#include <vtkPriorityQueue.h>
#include <vtkMath.h>
#include <vtkLine.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>
#include <vtkDoubleArray.h>

#include <map>
#include <set>

#ifdef WITH_PARAMETERIZATION
#include <comprow_double.h>
#include <compcol_double.h>
#include <diagpre_double.h>
#include <bicgstab.h>
#endif

// find all the points in a PolyData connected to pid
bool GetAdjacentPoints(vtkPolyData* const poly, const vtkIdType pid, vtkIdList* list)
{
   vtkIdList* cellIds = vtkIdList::New();
   std::set<int> pointSet;
   typedef std::set<int>::iterator IntSetIterator;

   poly->GetPointCells(pid, cellIds);
   //std::cerr << pid << " on " << cellIds->GetNumberOfIds() << " cells" << std::endl;

   list->Reset();
   for (int i = 0; i < cellIds->GetNumberOfIds(); i++)
   {
      int id = cellIds->GetId(i);
      vtkIdType npts;
      vtkIdType* pts;
      poly->GetCellPoints(id, npts, pts);
      for (int j = 0; j < npts; j++)
         if (pts[j] != pid) pointSet.insert(pts[j]);
   }

   list->SetNumberOfIds(pointSet.size());
   int j = 0;
   while (!pointSet.empty())
   {
      list->SetId(j++, *(pointSet.begin()));
      pointSet.erase(pointSet.begin());
   }

   cellIds->Delete();

   return true;
}

// same as GetAdjacentPoints, but the list is ordered (assuming no non-manifold geometry).
// the order isn't guaranteed or checked, but given a PolyData that's been thru the normals
// filter, it should be the same orientation for any pid
bool GetOrderedAdjacentPoints(vtkPolyData* const poly, const vtkIdType pid, vtkIdList* list)
{
   vtkIdList* cellIds = vtkIdList::New();
   std::set<int> pointSet;
   typedef std::set<int>::iterator IntSetIterator;

   poly->GetPointCells(pid, cellIds);
   //std::cerr << pid << " on " << cellIds->GetNumberOfIds() << " cells" << std::endl;

   list->Reset();
   for (int i = 0; i < cellIds->GetNumberOfIds(); i++)
   {
      int id = cellIds->GetId(i);
      vtkIdType npts;
      vtkIdType* pts;
      poly->GetCellPoints(id, npts, pts);
      for (int j = 0; j < npts; j++)
         if (pts[j] != pid) pointSet.insert(pts[j]);
   }
   //std::cerr << pid << " adjacent to " << pointSet.size() << " points" << std::endl;
   //if (cellIds->GetNumberOfIds() != pointSet.size())
      //std::cerr << pid << " has non-manifold triangulation" << std::endl;

   if (!pointSet.empty() && cellIds->GetNumberOfIds())
   {
      IntSetIterator it = pointSet.begin();
      while (it != pointSet.end())
      {
         poly->GetCellEdgeNeighbors(-1, pid, *it, cellIds);
         //std::cerr << pid << " : " << *it << " -> share " << cellIds->GetNumberOfIds() << " tris" << std::endl;
         //for (int i = 0; i < cellIds->GetNumberOfIds(); i++)
         //{
         //   int npts;
         //   vtkIdType* pts;
         //   poly->GetCellPoints(cellIds->GetId(i), npts, pts);
         //   std::cerr << pts[0] << " , " << pts[1] << " , " << pts[2] << std::endl;
         //}
         if (cellIds->GetNumberOfIds() == 1)
         {
            pointSet.erase(it);
            it = pointSet.begin();
         }
         else
            it++;
      }

      if (pointSet.empty()) { cellIds->Delete(); return false; }

      it = pointSet.begin();
      poly->GetCellEdgeNeighbors(-1, pid, *it, cellIds);
      while (it != pointSet.end() && cellIds->GetNumberOfIds() != 2)
      {
         it++;
         poly->GetCellEdgeNeighbors(-1, pid, *it, cellIds);
      }
      if (it == pointSet.end()) { cellIds->Delete(); return false; }

      int id = cellIds->GetId(0);

      vtkIdType npts;
      vtkIdType *pts;
      poly->GetCellPoints(id, npts, pts);

      std::deque<int> boundList;
      int u = -1;

      if (pts[0] == pid)
      {
         boundList.push_back(pts[1]);
         boundList.push_back(pts[2]);
         pointSet.erase(pts[1]);
         pointSet.erase(pts[2]);
         u = pts[2];
      }
      else if (pts[1] == pid)
      {
         boundList.push_back(pts[2]);
         boundList.push_back(pts[0]);
         pointSet.erase(pts[2]);
         pointSet.erase(pts[0]);
         u = pts[0];
      }
      else
      {
         boundList.push_back(pts[0]);
         boundList.push_back(pts[1]);
         pointSet.erase(pts[0]);
         pointSet.erase(pts[1]);
         u = pts[1];
      }

      it = pointSet.begin();
      while (it != pointSet.end())
      {
         int v = *it;
         if (poly->IsTriangle(pid, u, v))
         {
            boundList.push_back(v);
            u = v;
            pointSet.erase(it);
            it = pointSet.begin();
         }
         else
            it++;
      }

      if (!pointSet.empty())
      {
         it = pointSet.begin();
         u = boundList.front();
         while (it != pointSet.end())
         {
            int v = *it;
            if (poly->IsTriangle(pid, u, v))
            {
               boundList.push_front(v);
               u = v;
               pointSet.erase(it);
               it = pointSet.begin();
            }
            else
               it++;
         }
      }

      if (!pointSet.empty())
         std::cerr << static_cast<unsigned int>(pointSet.size()) << " points stranded when ordering adjacent list" << std::endl;

      list->SetNumberOfIds(boundList.size());
      int i = 0;
      while (!boundList.empty())
      {
         list->SetId(i++, boundList.front());
         boundList.pop_front();
      }

      cellIds->Delete();

      return true;
   }

   cellIds->Delete();

   return false;
}

// get number of points, edges, cells, holes, boundary edges, connected components
void GetPolyDataGeometryInfo(vtkPolyData* input, std::deque<PDGeometryInfo>& infoList)
{
   int numPoints = input->GetNumberOfPoints(), 
       numCells = input->GetNumberOfCells();

   std::multimap<int, int> boundaryEdges;
   typedef std::multimap<int, int>::iterator IntMultiMapIter;

   // initialize all cells to not visited
   bool* visitedCells = new bool[numCells];
   bool* visitedPoints = new bool[numPoints];
   for (int i = 0; i < numCells; i++) visitedCells[i] = false;
   for (int i = 0; i < numPoints; i++) visitedPoints[i] = false;

   vtkEdgeTable* edges = vtkEdgeTable::New();
   edges->InitEdgeInsertion(numPoints);

   // for each cell
   vtkIdList* cellIds = vtkIdList::New();
   std::set<int> Q;
   for (int i = 0; i < numCells; i++)
   {
   //    if cell visited, continue
      if (visitedCells[i]) continue;

   //    new PDGInfo
      PDGeometryInfo info;
      info.initialCell = i;
   //    push cell onto queue
      Q.insert(i);

   //    while !queue.empty
      while (!Q.empty())
      {
   //       cell <- queue.pop
         int cid = *(Q.begin());
         Q.erase(cid);
         if (visitedCells[cid]) continue;
   //       count cell & verts
         visitedCells[cid] = true;
         info.numFaces++;

   //       get cell points
         vtkIdType npts;
         vtkIdType* pts;
         input->GetCellPoints(cid, npts, pts);

   //       for each edge
         for (int j = 0; j < npts; j++)
         {
            vtkIdType u = pts[j],
                      v = pts[(j+1)%npts];

            if (edges->IsEdge(u, v) != -1) continue;
            edges->InsertEdge(u, v);
            info.numEdges++;

            if (!visitedPoints[u]) { visitedPoints[u] = true; info.numVerts++; }
   //          get edge neighbors
            input->GetCellEdgeNeighbors(cid, u, v, cellIds);
   //          if neighbors exist
            if (cellIds->GetNumberOfIds() > 0)
            {
   //             if neighbor not visited
               for (int k = 0; k < cellIds->GetNumberOfIds(); k++)
               {
   //                push onto queue
                  vtkIdType id = cellIds->GetId(k);
                  if (!visitedCells[id]) Q.insert(id);
               }
            }
   //          else
            else
            {
   //             mark as boundary edge
               boundaryEdges.insert(std::make_pair(u, v));
            }
         }
      }

      while (!boundaryEdges.empty())
      {
         bool looped = false;
         std::deque<int> loop;

         IntMultiMapIter edge = boundaryEdges.begin();
         loop.push_back(edge->second);
         loop.push_back(edge->first);
         boundaryEdges.erase(edge);

         edge = boundaryEdges.find(loop.front());
         while (edge != boundaryEdges.end())
         {
            if (edge->second != loop.back()) // catch the doubling of the initial point!
               loop.push_front(edge->second);
            else
               looped = true;
            boundaryEdges.erase(edge);
            edge = boundaryEdges.find(loop.front());
         }

         //if (!looped)
         //   for (edge = boundaryEdges.begin(); edge != boundaryEdges.end(); edge++)
         //   {
         //      if (edge->second == loop.back())
         //      {
         //         loop.push_back(edge->first);
         //         boundaryEdges.erase(edge);
         //         edge = boundaryEdges.begin();
         //      }
         //   }

         if (loop.size() == 2)
            std::cerr << "Isolated edge ( " << input << " ): " << loop.back() << " , " << loop.front() << std::endl;

         info.boundaryLoops.insert(info.boundaryLoops.end(), loop);
      }
      //    push PDGInfo onto set
      infoList.push_back(info);
   }
   cellIds->Delete();
   edges->Delete();
}

// puts a trivial cap over any boundary holes in the geometry
void FillBoundaryLoops(vtkPolyData* mesh, PDGeometryInfo::LoopSet& boundaryLoops)
{
   PDGeometryInfo::LoopSetIter it = boundaryLoops.begin();
   while (it != boundaryLoops.end())
   {
      double fillPoint[3] = {0,0,0};
      for (unsigned int j = 0; j < it->size(); j++)
      {
         double p[3];
         mesh->GetPoint((*it)[j], p);
         for (int k = 0; k < 3; k++) fillPoint[k] += p[k];
      }
      for (int k = 0; k < 3; k++) fillPoint[k] *= 1./(double)it->size();
      int u = mesh->InsertNextLinkedPoint(fillPoint, it->size());
      for (unsigned int j = 0; j < it->size(); j++)
      {
         vtkIdType newPts[3] = {u, (*it)[j], (*it)[(j+1)%it->size()]};
         mesh->InsertNextLinkedCell(VTK_TRIANGLE, 3, newPts);
      }
      it++;
   }
}

// uses Dijkstra's SSSP to propogate geodesic distances.
// can also fill shortest path back to source.
// multiple approximations available, but law of cosines is most accurate
int CalcGeodesic(vtkPolyData* const mesh, vtkPriorityQueue* queue, double* geodesic, int* path, int* region)
{
   bool* relaxed = new bool[mesh->GetNumberOfPoints()];
   for (int i = 0; i < mesh->GetNumberOfPoints(); i++) relaxed[i] = false;
   vtkIdType u;
   vtkIdList* adjacent = vtkIdList::New();
   vtkIdList* upwind = vtkIdList::New();
   vtkIdList* downwind = vtkIdList::New();
   while (queue->GetNumberOfItems())
   {
      u = queue->Pop(0);
      relaxed[u] = true;

      GetAdjacentPoints(mesh, u, adjacent);

      double pu[3];
      mesh->GetPoint(u, pu);
      for (int j = 0; j < adjacent->GetNumberOfIds(); j++)
      {
         vtkIdType v = adjacent->GetId(j);
         if (relaxed[v]) continue;
         double pv[3];
         mesh->GetPoint(v, pv);

#define LAW_OF_COSINES_APPROXIMATION

#ifdef DIJKSTRA_APPROXIMATION
#undef CLOSEST_POINTS_APPROXIMATION
#undef LAW_OF_COSINES_APPROXIMATION
#endif

#ifdef CLOSEST_POINTS_APPROXIMATION
#undef DIJKSTRA_APPROXIMATION
#undef LAW_OF_COSINES_APPROXIMATION
#endif

#ifdef LAW_OF_COSINES_APPROXIMATION
#undef DIJKSTRA_APPROXIMATION
#undef CLOSEST_POINTS_APPROXIMATION
#endif

#if defined(DIJKSTRA_APPROXIMATION)
         double wuv = geodesic[u] + sqrt(vtkMath::Distance2BetweenPoints(pu, pv));

         if (geodesic[v] > wuv)
         {
            geodesic[v] = wuv;
            queue->DeleteId(v);
            queue->Insert(wuv, v);
            if (region) region[v] = region[u];
            if (path) path[v] = u;
         }
#endif
#if defined(CLOSEST_POINTS_APPROXIMATION)
         mesh->GetCellEdgeNeighbors(-1, u, v, upwind);
         for (int k = 0; k < upwind->GetNumberOfIds(); k++)
         {
            vtkIdType cid = upwind->GetId(k);
            vtkIdType npts;
            vtkIdType *pts;
            mesh->GetCellPoints(cid, npts, pts);

            vtkIdType w = pts[0];
            w = (w == u || w == v) ? pts[1] : w;
            w = (w == u || w == v) ? pts[2] : w;

            double pw[3];
            mesh->GetPoint(w, pw);

            mesh->GetCellEdgeNeighbors(cid, u, w, downwind);
            for (int l = 0; l < downwind->GetNumberOfIds(); l++)
            {
               vtkIdType id = downwind->GetId(l);
               mesh->GetCellPoints(id, npts, pts);

               vtkIdType x = pts[0];
               x = (x == u || x == w) ? pts[1] : x;
               x = (x == u || x == w) ? pts[2] : x;

               double wuv = geodesic[u] + sqrt(vtkMath::Distance2BetweenPoints(pu, pv));
               if (relaxed[x])
               {
                  double px[3], t, pc0[3], pc1[3];
                  mesh->GetPoint(x, px);
                  vtkLine::DistanceToLine(px, pu, pw, t, pc0);
                  vtkLine::DistanceToLine(pv, pu, pw, t, pc1);
                  pc0[0] += pc1[0]; pc0[1] += pc1[1]; pc0[2] += pc1[2];
                  pc0[0] *= .5; pc0[1] *= .5; pc0[2] *= .5;
                  double alt = geodesic[x]
                             + sqrt(vtkMath::Distance2BetweenPoints(px, pc0))
                             + sqrt(vtkMath::Distance2BetweenPoints(pv, pc0));
                  wuv = alt < wuv ? alt : wuv;
               }

               if (geodesic[v] > wuv)
               {
                  geodesic[v] = wuv;
                  queue->DeleteId(v);
                  queue->Insert(wuv, v);
                  if (region) region[v] = region[u];
                  if (path) path[v] = u;
               }
            }
         }
#endif
#if defined(LAW_OF_COSINES_APPROXIMATION)
         mesh->GetCellEdgeNeighbors(-1, u, v, upwind);
         for (int k = 0; k < upwind->GetNumberOfIds(); k++)
         {
            vtkIdType cid = upwind->GetId(k);
            vtkIdType npts;
            vtkIdType *pts;
            mesh->GetCellPoints(cid, npts, pts);

            vtkIdType w = pts[0];
            w = (w == u || w == v) ? pts[1] : w;
            w = (w == u || w == v) ? pts[2] : w;

            double pw[3];
            mesh->GetPoint(w, pw);

            mesh->GetCellEdgeNeighbors(cid, u, w, downwind);
            for (int l = 0; l < downwind->GetNumberOfIds(); l++)
            {
               vtkIdType id = downwind->GetId(l);
               mesh->GetCellPoints(id, npts, pts);

               vtkIdType x = pts[0];
               x = (x == u || x == w) ? pts[1] : x;
               x = (x == u || x == w) ? pts[2] : x;

               double wuv = geodesic[u] + sqrt(vtkMath::Distance2BetweenPoints(pu, pv));
               if (relaxed[x])
               {
                  double px[3];
                  mesh->GetPoint(x, px);

                  // check vuw+wux < 180.
                  double vu[3] = {pv[0]-pu[0],pv[1]-pu[1],pv[2]-pu[2]},
                         wu[3] = {pw[0]-pu[0],pw[1]-pu[1],pw[2]-pu[2]},
                         xu[3] = {px[0]-pu[0],px[1]-pu[1],px[2]-pu[2]},
                         lvu = vtkMath::Normalize(vu),
                         //lwu = vtkMath::Normalize(wu),
                         lxu = vtkMath::Normalize(xu),
                         acosvuw = acos(vtkMath::Dot(vu,wu)),
                         acoswux = acos(vtkMath::Dot(wu,xu));

                  if (acosvuw + acoswux - vtkMath::Pi() <= -1e-4)
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
                     if (acosvwu + acosuwx - vtkMath::Pi() <= -1e-4)
                     {
                        double alt = geodesic[x]
                                   + sqrt(lvu*lvu + lxu*lxu - 2.*lvu*lxu*vtkMath::Dot(vu,xu));
                        wuv = alt < wuv ? alt : wuv;
                     }
                  }
               }

               if (geodesic[v] > wuv)
               {
                  geodesic[v] = wuv;
                  queue->DeleteId(v);
                  queue->Insert(wuv, v);
                  if (region) region[v] = region[u];
                  if (path) path[v] = u;
               }
            }
         }
#endif
      }
   }
   adjacent->Delete();
   upwind->Delete();
   downwind->Delete();
   delete[] relaxed;
   return u;
}

// given two points and distance to third point, local third point
bool TriangulatePoints(double* x0, double r0, double* x1, double r1, double* x2, double* x3)
{
   double dx = x1[0] - x0[0],
          dy = x1[1] - x0[1],
          d = sqrt((dy*dy) + (dx*dx));

   if (d > (r0 + r1)) return false;
   if (d < abs(r0 - r1)) return false;

   double a = ((r0*r0) - (r1*r1) + (d*d)) / (2.0 * d),
          h = sqrt((r0*r0) - (a*a)),
          p[2] = { x0[0] + (dx * a/d), x0[1] + (dy * a/d) },
          r[2] = { -dy * (h/d), dx * (h/d) };

   x2[0] = p[0] + r[0];
   x2[1] = p[1] + r[1];

   x3[0] = p[0] - r[0];
   x3[1] = p[1] - r[1];

   return true;
};
