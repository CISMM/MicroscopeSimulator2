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
#include "vtkPolyDataDistance.h"

#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkTriangle.h>
#include <vtkDoubleArray.h>
#include <vtkFieldData.h>

#include "vtkImplicitPolyData.h"

vtkCxxRevisionMacro(vtkPolyDataDistance, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkPolyDataDistance);

vtkPolyDataDistance::vtkPolyDataDistance() : vtkPolyDataAlgorithm()
{
   this->SignedDistance = 1;
   this->InvertDistance = 0;
   this->ComputeSecondDistance = 0;

   this->SetNumberOfInputPorts(2);
   this->SetNumberOfOutputPorts(2);

   vtkPolyData* output1 = vtkPolyData::New();
   this->GetExecutive()->SetOutputData(1, output1);
   output1->Delete();
}

vtkPolyDataDistance::~vtkPolyDataDistance()
{
}

int vtkPolyDataDistance::RequestData(vtkInformation*        vtkNotUsed(request),
                                     vtkInformationVector** inputVector,
                                     vtkInformationVector*  outputVector)
{
   vtkInformation* inInfo0 = inputVector[0]->GetInformationObject(0);
   vtkInformation* inInfo1 = inputVector[1]->GetInformationObject(0);
   vtkInformation* outInfo = outputVector->GetInformationObject(0);

   if (!inInfo0 || !inInfo1 || !outInfo) return 0;

   vtkPolyData *input0 = vtkPolyData::SafeDownCast(inInfo0->Get(vtkDataObject::DATA_OBJECT()));
   vtkPolyData *input1 = vtkPolyData::SafeDownCast(inInfo1->Get(vtkDataObject::DATA_OBJECT()));
   vtkPolyData* output0 = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
   vtkPolyData* output1 = this->GetSecondDistanceOutput();

   if (!input0 || !input1 || !output0 || !output1) return 0;

   output0->CopyStructure(input0);
   output0->GetPointData()->PassData(input0->GetPointData());
   output0->GetCellData()->PassData(input0->GetCellData());
   GetPolyDataDistance(output0, input1);

   if (this->ComputeSecondDistance)
   {
      output1->CopyStructure(input1);
      output1->GetPointData()->PassData(input1->GetPointData());
      output1->GetCellData()->PassData(input1->GetCellData());
      GetPolyDataDistance(output1, input0);
   }

   return 1;
}

void vtkPolyDataDistance::PrintSelf(ostream& os, vtkIndent indent)
{
   this->Superclass::PrintSelf(os,indent);
   os << indent << "SignedDistance: " << this->SignedDistance << "\n";
   os << indent << "InvertDistance: " << this->InvertDistance << "\n";
   os << indent << "ComputeSecondDistance: " << this->ComputeSecondDistance << "\n";
}

void vtkPolyDataDistance::GetPolyDataDistance(vtkPolyData* mesh, vtkPolyData* src)
{
   vtkDebugMacro(<<"Start vtkPolyDataDistance::GetPolyDataDistance");

   if (mesh->GetNumberOfPolys() == 0 || mesh->GetNumberOfPoints() == 0)
   {
      vtkErrorMacro(<<"No points/cells to operate on");
      return;
   }

   if (src->GetNumberOfPolys() == 0 || src->GetNumberOfPoints() == 0)
   {
      vtkErrorMacro(<<"No points/cells to difference from");
      return;
   }

   int numPts = mesh->GetNumberOfPoints();

   vtkDoubleArray* da = vtkDoubleArray::New();
   da->SetName("Distance");
   da->SetNumberOfComponents(1);
   da->SetNumberOfTuples(numPts);

   double* dist = da->GetPointer(0);

   double bound0[6], bound1[6], bounds[6];
   src->GetBounds(bound0);
   mesh->GetBounds(bound1);
   bounds[0] = (bound0[0] < bound1[0] ? bound0[0] : bound1[0]) - 1.;
   bounds[1] = (bound0[1] > bound1[1] ? bound0[1] : bound1[1]) + 1.;
   bounds[2] = (bound0[2] < bound1[2] ? bound0[2] : bound1[2]) - 1.;
   bounds[3] = (bound0[3] > bound1[3] ? bound0[3] : bound1[3]) + 1.;
   bounds[4] = (bound0[4] < bound1[4] ? bound0[4] : bound1[4]) - 1.;
   bounds[5] = (bound0[5] > bound1[5] ? bound0[5] : bound1[5]) + 1.;

   vtkImplicitPolyData* imp = vtkImplicitPolyData::New();
   imp->SetInput(src);
   imp->SetReverseBias(InvertDistance);
   imp->SetEvaluateBounds(bounds);

   for (int i = 0; i < numPts; i++)
   {
      double pt[3];
      mesh->GetPoint(i, pt);
      double val = imp->EvaluateFunction(pt);
      dist[i] = SignedDistance ? (InvertDistance ? -val : val) : fabs(val);
   }

   mesh->GetPointData()->AddArray(da);
   mesh->GetPointData()->SetActiveScalars("Distance");

   imp->Delete();
   da->Delete();

   vtkDebugMacro(<<"End vtkPolyDataDistance::GetPolyDataDistance");
}

vtkPolyData* vtkPolyDataDistance::GetSecondDistanceOutput()
{
   if (!this->ComputeSecondDistance) return 0;
   return vtkPolyData::SafeDownCast(this->GetExecutive()->GetOutputData(1));
}

int vtkPolyDataDistance::FillInputPortInformation(int port, vtkInformation *info)
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