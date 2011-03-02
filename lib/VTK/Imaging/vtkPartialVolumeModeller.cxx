/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPartialVolumeModeller.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPartialVolumeModeller.h"

#include "vtkCell.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiThreader.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkPointData.h"
#include "vtkBoxClipDataSet.h"
#include "vtkUnstructuredGrid.h"
#include "vtkTetra.h"

#include <math.h>

vtkCxxRevisionMacro(vtkPartialVolumeModeller, "1.0");
vtkStandardNewMacro(vtkPartialVolumeModeller);

struct vtkPartialVolumeModellerThreadInfo
{
  vtkPartialVolumeModeller *Modeller;
  vtkDataSet               *Input;
};

// Construct an instance of vtkPartialVolumeModeller with its sample dimensions
// set to (50,50,50), and so that the model bounds are
// automatically computed from its input. The maximum distance is set to 
// examine the whole grid. This could be made much faster, and probably
// will be in the future.
vtkPartialVolumeModeller::vtkPartialVolumeModeller()
{
  this->MaximumDistance = 1.0;

  this->ModelBounds[0] = 0.0;
  this->ModelBounds[1] = 0.0;
  this->ModelBounds[2] = 0.0;
  this->ModelBounds[3] = 0.0;
  this->ModelBounds[4] = 0.0;
  this->ModelBounds[5] = 0.0;

  this->SampleDimensions[0] = 50;
  this->SampleDimensions[1] = 50;
  this->SampleDimensions[2] = 50;

  this->OutputScalarType = VTK_DOUBLE;

  this->Threader        = vtkMultiThreader::New();
  this->NumberOfThreads = this->Threader->GetNumberOfThreads();
}

//----------------------------------------------------------------------------
vtkPartialVolumeModeller::~vtkPartialVolumeModeller()
{
  if (this->Threader)
    {
    this->Threader->Delete();
    }
}

//----------------------------------------------------------------------------
// Specify the position in space to perform the voxelization.
void vtkPartialVolumeModeller::SetModelBounds(double bounds[6])
{
  this->SetModelBounds(bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]);
}

//----------------------------------------------------------------------------
void vtkPartialVolumeModeller::SetModelBounds(double xmin, double xmax, double ymin,
                                      double ymax, double zmin, double zmax)
{
  if (this->ModelBounds[0] != xmin || this->ModelBounds[1] != xmax ||
      this->ModelBounds[2] != ymin || this->ModelBounds[3] != ymax ||
      this->ModelBounds[4] != zmin || this->ModelBounds[5] != zmax )
    {
    this->Modified();
    this->ModelBounds[0] = xmin;
    this->ModelBounds[1] = xmax;
    this->ModelBounds[2] = ymin;
    this->ModelBounds[3] = ymax;
    this->ModelBounds[4] = zmin;
    this->ModelBounds[5] = zmax;
    }
}

//----------------------------------------------------------------------------
int vtkPartialVolumeModeller::RequestInformation (
  vtkInformation * vtkNotUsed(request),
  vtkInformationVector ** vtkNotUsed( inputVector ),
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  int i;
  double ar[3], origin[3];
  
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
               0, this->SampleDimensions[0]-1,
               0, this->SampleDimensions[1]-1,
               0, this->SampleDimensions[2]-1);
  
  for (i=0; i < 3; i++)
    {
    origin[i] = this->ModelBounds[2*i];
    if ( this->SampleDimensions[i] <= 1 )
      {
      ar[i] = 1;
      }
    else
      {
      ar[i] = (this->ModelBounds[2*i+1] - this->ModelBounds[2*i])
              / (this->SampleDimensions[i] - 1);
      }
    }
  outInfo->Set(vtkDataObject::ORIGIN(),origin,3);
  outInfo->Set(vtkDataObject::SPACING(),ar,3);

  vtkDataObject::SetPointDataActiveScalarInfo(outInfo, this->OutputScalarType, 1);
  return 1;
}

//----------------------------------------------------------------------------
static VTK_THREAD_RETURN_TYPE vtkPartialVolumeModeller_ThreadedExecute( void *arg )
{
  int threadId = ((vtkMultiThreader::ThreadInfo *)(arg))->ThreadID;
  int threadCount = ((vtkMultiThreader::ThreadInfo *)(arg))->NumberOfThreads;
  vtkPartialVolumeModellerThreadInfo *userData = (vtkPartialVolumeModellerThreadInfo *)
    (((vtkMultiThreader::ThreadInfo *)(arg))->UserData);

  vtkDataSet *input = userData->Input;
  vtkImageData *output = userData->Modeller->GetOutput();
  double *spacing = output->GetSpacing();
  double *origin = output->GetOrigin();

  int *sampleDimensions = userData->Modeller->GetSampleDimensions();
  if (!output->GetPointData()->GetScalars())
    {
    vtkGenericWarningMacro("No output scalars defined.");
    return VTK_THREAD_RETURN_VALUE;
    }

  // break up into slabs based on threadId and threadCount
  int slabSize = sampleDimensions[2] / threadCount;
  int slabSizeExtra = slabSize + 1; // For slabs with an extra layer
  int numThreadsWithExtraLayer = sampleDimensions[2] % threadCount;

  int slabMin, slabMax;
  if (threadId < numThreadsWithExtraLayer)
    {
    slabMin = threadId * slabSizeExtra;
    slabMax = slabMin + slabSizeExtra - 1;
    }
  else
    {
    slabMin = numThreadsWithExtraLayer * slabSizeExtra +
      (threadId - numThreadsWithExtraLayer) * slabSize;
    slabMax = slabMin + slabSize - 1;
    }
  if (slabMin >= sampleDimensions[2])
    {
    return VTK_THREAD_RETURN_VALUE;
    }

  double *weights = new double[input->GetMaxCellSize()];
  vtkDataArray *newScalars = output->GetPointData()->GetScalars();

  //
  // Voxel widths are 1/2 the height, width, length of a voxel
  //
  double voxelHalfWidth[3];
  for (int i=0; i < 3; i++)
    {
    voxelHalfWidth[i] = spacing[i] / 2.0;
    }
  
  // Set up the box clipping filter
  vtkBoxClipDataSet *clipper = vtkBoxClipDataSet::New();
  clipper->SetInput(input);
  clipper->SetOrientation(0);
  
  // Compute the volume of a filled voxel.
  double fullVoxelVolume = spacing[0]*spacing[1]*spacing[2];

  //
  // Traverse all voxels, computing partial volume intersecton on all points.
  //
  int jkFactor = sampleDimensions[0]*sampleDimensions[1];
  for (int k = slabMin; k <= slabMax; k++)
    {
    double zmin = (static_cast<double>(k) - 0.5)*spacing[2] + origin[2];
    double zmax = (static_cast<double>(k) + 0.5)*spacing[2] + origin[2];
    for (int j = 0; j < sampleDimensions[1]; j++)
      {
      double ymin = (static_cast<double>(j) - 0.5)*spacing[1] + origin[1];
      double ymax = (static_cast<double>(j) + 0.5)*spacing[1] + origin[1];
      for (int i = 0; i < sampleDimensions[0]; i++)
        {
        double xmin = (static_cast<double>(i) - 0.5)*spacing[0] + origin[0];
        double xmax = (static_cast<double>(i) + 0.5)*spacing[0] + origin[0];
        
        // Update the box dimensions in the clipper
        clipper->SetBoxClip(xmin, xmax, ymin, ymax, zmin, zmax);
        clipper->Update();
        
        // Get the output tetrahedra from the clipper and compute the sum of their volumes
        vtkUnstructuredGrid* intersection = clipper->GetOutput();
        int numCells = intersection->GetNumberOfCells();
        double volume = 0.0;
        for (int cellNum = 0; cellNum < numCells; cellNum++)
          {
          vtkCell *cell = intersection->GetCell(cellNum);
          if (cell->GetCellType() == VTK_TETRA)
            {
            vtkTetra* tet = vtkTetra::SafeDownCast(cell);
            vtkPoints* pts = tet->GetPoints();
            double p0[3], p1[3], p2[3], p3[3];
            pts->GetPoint(0, p0);
            pts->GetPoint(1, p1);
            pts->GetPoint(2, p2);
            pts->GetPoint(3, p3);
            volume += vtkTetra::ComputeVolume(p0, p1, p2, p3);
            }
          else
            {
            vtkGenericWarningMacro( << "A non-tetrahedral element was encountered.");
            }
          }

        int idx = jkFactor*k + sampleDimensions[0]*j + i;
        newScalars->SetComponent(idx, 0, volume / fullVoxelVolume);
        }
      }
    }
  clipper->Delete();

  delete [] weights;

  return VTK_THREAD_RETURN_VALUE;
}

//----------------------------------------------------------------------------
int vtkPartialVolumeModeller::RequestData(
  vtkInformation* vtkNotUsed( request ),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // get the input
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  // get the output
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkImageData *output = vtkImageData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // We need to allocate our own scalars since we are overriding
  // the superclasses "Execute()" method.
  output->SetExtent(output->GetWholeExtent());
  output->AllocateScalars();

  double origin[3], spacing[3];
  double maxDistance = this->ComputeModelBounds(origin, spacing);
  outInfo->Set(vtkDataObject::ORIGIN(), origin, 3);

  vtkPartialVolumeModellerThreadInfo info;
  info.Modeller = this;
  info.Input = input;

  // Set the number of threads to use,
  // then set the execution method and do it.
  this->Threader->SetNumberOfThreads( this->NumberOfThreads );
  this->Threader->SetSingleMethod( vtkPartialVolumeModeller_ThreadedExecute,
    (void *)&info);
  this->Threader->SingleMethodExecute();

  return 1;
}

//----------------------------------------------------------------------------
// Compute the ModelBounds based on the input geometry.
double vtkPartialVolumeModeller::ComputeModelBounds(double origin[3], 
                                            double spacing[3])
{
  double *bounds, maxDist;
  int i, adjustBounds=0;

  // compute model bounds if not set previously
  if ( this->ModelBounds[0] >= this->ModelBounds[1] ||
       this->ModelBounds[2] >= this->ModelBounds[3] ||
       this->ModelBounds[4] >= this->ModelBounds[5] )
    {
    adjustBounds = 1;
    vtkDataSet *ds = vtkDataSet::SafeDownCast(this->GetInput());
    // ds better be non null otherwise something is very wrong here
    bounds = ds->GetBounds();
    }
  else
    {
    bounds = this->ModelBounds;
    }

  for (maxDist=0.0, i=0; i<3; i++)
    {
    if ( (bounds[2*i+1] - bounds[2*i]) > maxDist )
      {
      maxDist = bounds[2*i+1] - bounds[2*i];
      }
    }
  maxDist *= this->MaximumDistance;

  // adjust bounds so model fits strictly inside (only if not set previously)
  if ( adjustBounds )
    {
    for (i=0; i<3; i++)
      {
      this->ModelBounds[2*i] = bounds[2*i] - maxDist;
      this->ModelBounds[2*i+1] = bounds[2*i+1] + maxDist;
      }
    }

  // Set volume origin and data spacing
  for (i=0; i<3; i++)
    {
    origin[i] = this->ModelBounds[2*i];
    spacing[i] = (this->ModelBounds[2*i+1] - this->ModelBounds[2*i])/
      (this->SampleDimensions[i] - 1);
    }

  return maxDist;  
}

//----------------------------------------------------------------------------
// Set the i-j-k dimensions on which to sample the distance function.
void vtkPartialVolumeModeller::SetSampleDimensions(int i, int j, int k)
{
  int dim[3];

  dim[0] = i;
  dim[1] = j;
  dim[2] = k;

  this->SetSampleDimensions(dim);
}

//----------------------------------------------------------------------------
void vtkPartialVolumeModeller::SetSampleDimensions(int dim[3])
{
  int dataDim, i;

  vtkDebugMacro(<< " setting SampleDimensions to (" << dim[0] << "," << dim[1]
                << "," << dim[2] << ")");

  if ( dim[0] != this->SampleDimensions[0] ||
       dim[1] != this->SampleDimensions[1] ||
       dim[2] != this->SampleDimensions[2] )
    {
    if ( dim[0]<1 || dim[1]<1 || dim[2]<1 )
      {
      vtkErrorMacro (<< "Bad Sample Dimensions, retaining previous values");
      return;
      }
    for (dataDim=0, i=0; i<3 ; i++)
      {
      if (dim[i] > 1)
        {
        dataDim++;
        }
      }
    if ( dataDim  < 3 )
      {
      vtkErrorMacro(<<"Sample dimensions must define a volume!");
      return;
      }

    for ( i=0; i<3; i++)
      {
      this->SampleDimensions[i] = dim[i];
      }
    this->Modified();
    }
}

//----------------------------------------------------------------------------
int vtkPartialVolumeModeller::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

//----------------------------------------------------------------------------
void vtkPartialVolumeModeller::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Maximum Distance: " << this->MaximumDistance << "\n";
  os << indent << "Sample Dimensions: (" << this->SampleDimensions[0] << ", "
               << this->SampleDimensions[1] << ", "
               << this->SampleDimensions[2] << ")\n";
  os << indent << "Model Bounds: \n";
  os << indent << "  Xmin,Xmax: (" << this->ModelBounds[0] << ", "
     << this->ModelBounds[1] << ")\n";
  os << indent << "  Ymin,Ymax: (" << this->ModelBounds[2] << ", "
     << this->ModelBounds[3] << ")\n";
  os << indent << "  Zmin,Zmax: (" << this->ModelBounds[4] << ", "
     << this->ModelBounds[5] << ")\n";
  os << indent << "OutputScalarType: " << this->OutputScalarType << endl;
}
