#include <GridBasedFluorophoreProperty.h>

#include <vtkDataObject.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkPartialVolumeModeller.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridAlgorithm.h>
#include <vtkThresholdPoints.h>


GridBasedFluorophoreProperty
::GridBasedFluorophoreProperty(const std::string& name,
                               vtkUnstructuredGridAlgorithm* gridSource,
                               bool editable, bool optimizable) 
  : FluorophoreModelObjectProperty(name, editable, optimizable) {

  m_SampleSpacing = 50.0;
  m_GridSource = gridSource;

  // Set up the class that computes partial volume effects
  m_PartialVolumeVoxelizer = vtkSmartPointer<vtkPartialVolumeModeller>::New();
  m_PartialVolumeVoxelizer->SetInputConnection(gridSource->GetOutputPort());
  m_PartialVolumeVoxelizer->SetOutputScalarTypeToFloat();

  vtkSmartPointer<vtkThresholdPoints> thold =
    vtkSmartPointer<vtkThresholdPoints>::New();
  thold->ThresholdByUpper(1e-9);
  thold->SetInputConnection(m_PartialVolumeVoxelizer->GetOutputPort());

  //m_FluorophoreOutput = intensityCalculator;
  m_FluorophoreOutput = thold;

  this->Update();
}


GridBasedFluorophoreProperty
::~GridBasedFluorophoreProperty() {

}


void
GridBasedFluorophoreProperty
::SetSampleSpacing(double spacing) {
  m_SampleSpacing = spacing;
}


double
GridBasedFluorophoreProperty
::GetSampleSpacing() {
  return m_SampleSpacing;
}


int
GridBasedFluorophoreProperty
::GetNumberOfFluorophores() {
  vtkPolyData* output = vtkPolyData::SafeDownCast(m_FluorophoreOutput->GetOutputDataObject(0));
  int numPoints = 0;
  if (output) {
    output->Update();
    numPoints = output->GetNumberOfPoints();
  }

  return numPoints;
}


void
GridBasedFluorophoreProperty
::Update() {
  int dims[3];
  if (!this->GetEnabled()) {
    // Set the sample dimensions to 2 in case the voxelizers get updated
    dims[0] = dims[1] = dims[2] = 2;
    m_PartialVolumeVoxelizer->SetSampleDimensions(dims);
    return;
  }

  // Get bounding box size of the grid source
  double bounds[6];
  m_GridSource->GetOutput()->Update();
  m_GridSource->GetOutput()->GetBounds(bounds);

  for (int i = 0; i < 3; i++) {
    // Pad the boundaries by half a voxel in each dimension
    bounds[2*i + 0] -= m_SampleSpacing;
    bounds[2*i + 1] += m_SampleSpacing;

    // Snap the bounds to the nearest multiple of the grid spacing
    // just outside the unsnapped bounds. This makes objective functions
    // better behaved.
    bounds[2*i + 0] = floor(bounds[2*i + 0] / m_SampleSpacing) * m_SampleSpacing;
    bounds[2*i + 1] = ceil (bounds[2*i + 1] / m_SampleSpacing) * m_SampleSpacing;

    dims[i] = ceil((bounds[2*i+1] - bounds[2*i]) / m_SampleSpacing);
    if (dims[i] < 1) dims[i] = 1;
    dims[i]++;
  }

  m_PartialVolumeVoxelizer->SetModelBounds(bounds);
  m_PartialVolumeVoxelizer->SetSampleDimensions(dims);
}


double
GridBasedFluorophoreProperty
::GetDensityScale() {
  return 1.0e-9;
}

