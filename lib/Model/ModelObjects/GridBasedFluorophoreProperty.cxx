#include <GridBasedFluorophoreProperty.h>

#include <vtkDataObject.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkPartialVolumeModeller.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridAlgorithm.h>
#include <vtkThresholdPoints.h>
#include <vtkXMLPolyDataWriter.h>


const char* GridBasedFluorophoreProperty::SAMPLE_SPACING_ATT = "sampleSpacing";


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

  m_Threshold = vtkSmartPointer<vtkThresholdPoints>::New();
  m_Threshold->ThresholdByUpper(1e-9);
  m_Threshold->SetInputConnection(m_PartialVolumeVoxelizer->GetOutputPort());

  m_FluorophoreOutput = m_Threshold;

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
  m_FluorophoreOutput->Update();
  vtkPolyData* output = vtkPolyData::SafeDownCast(m_FluorophoreOutput->GetOutputDataObject(0));
  int numPoints = 0;
  if (output) {
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
  m_GridSource->Update();
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
  m_PartialVolumeVoxelizer->Update();
}


void
GridBasedFluorophoreProperty
::GetXMLConfiguration(xmlNodePtr root) {
  FluorophoreModelObjectProperty::GetXMLConfiguration(root);

  char value[256];
  sprintf(value, "%f", GetSampleSpacing());
  xmlNewProp(root, BAD_CAST SAMPLE_SPACING_ATT, BAD_CAST value);
}


void
GridBasedFluorophoreProperty
::RestoreFromXML(xmlNodePtr root) {
  FluorophoreModelObjectProperty::RestoreFromXML(root);

  char* value = (char *) xmlGetProp(root, BAD_CAST SAMPLE_SPACING_ATT);
  if (value) {
    double sampleSpacing = atof(value);
    SetSampleSpacing(sampleSpacing);
  }
}


double
GridBasedFluorophoreProperty
::GetDensityScale() {
  return 1.0e-9;
}

