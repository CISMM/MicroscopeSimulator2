#include <GridBasedFluorophoreProperty.h>

#include <vtkAppendPolyData.h>
#include <vtkArrayCalculator.h>
#include <vtkDataObject.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImplicitModeller.h>
#include <vtkPointData.h>
#include <vtkProgrammableFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridAlgorithm.h>
#include <vtkThresholdPoints.h>

void GridBasedFluorophorePropertyIntensityFallOffFunction(void* arg)
{
  GridBasedFluorophoreProperty::vtkProgrammableFilterUserData* userData = 
    reinterpret_cast<
      GridBasedFluorophoreProperty::vtkProgrammableFilterUserData*>(arg);

  vtkProgrammableFilter* filter = userData->filter;
  double spacing = userData->spacing;
  vtkPolyData* input  = filter->GetPolyDataInput();
  vtkPolyData* output = filter->GetPolyDataOutput();
  output->CopyStructure(input);

  vtkDataArray* inputArray = input->GetPointData()->GetArray(0);
  if (inputArray == NULL) {
    return;
  }

  // Create a new data array to hold the output intensities
  vtkFloatArray* outputArray = vtkFloatArray::New();
  outputArray->SetName("Intensity");
  outputArray->SetNumberOfComponents(1);
  outputArray->SetNumberOfTuples(inputArray->GetNumberOfTuples());
  outputArray->Allocate(inputArray->GetNumberOfTuples());

  // Iterate over the data values and apply the intensity falloff calculation
  double alpha = -log(0.5) / (0.5 * spacing);
  for (int i = 0; i < inputArray->GetNumberOfTuples(); i++) {
    double inValue  = inputArray->GetComponent(i, 0);
    double outValue = exp(-alpha * inValue);
    outputArray->SetComponent(i, 0, outValue);
  }

  output->GetPointData()->AddArray(outputArray);
}


GridBasedFluorophoreProperty
::GridBasedFluorophoreProperty(const std::string& name,
                               vtkUnstructuredGridAlgorithm* gridSource,
                               bool editable, bool optimizable) 
  : FluorophoreModelObjectProperty(name, editable, optimizable) {

  m_SampleSpacing = 50.0;
  m_GridSource = gridSource;

  // Set up the class that performs the distance computation
  // on a structured grid
  m_Voxelizer1 = vtkSmartPointer<vtkImplicitModeller>::New();
  m_Voxelizer1->SetInputConnection(gridSource->GetOutputPort());
  m_Voxelizer1->SetOutputScalarTypeToFloat();

  vtkSmartPointer<vtkThresholdPoints> thold1 =
    vtkSmartPointer<vtkThresholdPoints>::New();
  thold1->ThresholdByLower(sqrt(3*m_SampleSpacing*m_SampleSpacing));
  thold1->SetInputConnection(m_Voxelizer1->GetOutputPort());

  m_Voxelizer2 = vtkSmartPointer<vtkImplicitModeller>::New();
  m_Voxelizer2->SetInputConnection(gridSource->GetOutputPort());
  m_Voxelizer2->SetOutputScalarTypeToFloat();

  vtkSmartPointer<vtkThresholdPoints> thold2 =
    vtkSmartPointer<vtkThresholdPoints>::New();
  thold2->ThresholdByLower(sqrt(3*m_SampleSpacing*m_SampleSpacing));
  thold2->SetInputConnection(m_Voxelizer2->GetOutputPort());

  vtkSmartPointer<vtkAppendPolyData> appender =
    vtkSmartPointer<vtkAppendPolyData>::New();
  appender->AddInputConnection(thold1->GetOutputPort());
  appender->AddInputConnection(thold2->GetOutputPort());

  vtkSmartPointer<vtkProgrammableFilter> intensityCalculator =
    vtkSmartPointer<vtkProgrammableFilter>::New();
  intensityCalculator->SetInputConnection(appender->GetOutputPort());
  m_UserData.filter = intensityCalculator;
  m_UserData.spacing = m_SampleSpacing;
  intensityCalculator->
    SetExecuteMethod(GridBasedFluorophorePropertyIntensityFallOffFunction, &m_UserData);

  m_FluorophoreOutput = intensityCalculator;

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
  // Get bounding box size of the grid source
  double bounds[6];
  m_GridSource->GetOutput()->Update();
  std::cout << "Grid source: " << m_GridSource->GetOutput() << std::endl;
  m_GridSource->GetOutput()->GetBounds(bounds);

  double padding = sqrt(3*m_SampleSpacing*m_SampleSpacing);
  int dims[3];
  for (int i = 0; i < 3; i++) {
    // Pad the boundaries
    bounds[2*i + 0] -= padding;
    bounds[2*i + 1] += padding;

    dims[i] = ceil((bounds[2*i+1] - bounds[2*i]) / m_SampleSpacing);
    if (dims[i] < 2) dims[i] = 2;
  }

  m_Voxelizer1->SetModelBounds(bounds);
  m_Voxelizer1->SetSampleDimensions(dims);

  // Shift bounds for the cell-centers
  for (int i = 0; i < 6; i++) {
    bounds[i] -= 0.5*m_SampleSpacing;
  }

  m_Voxelizer2->SetModelBounds(bounds);
  m_Voxelizer2->SetSampleDimensions(dims);
}


double
GridBasedFluorophoreProperty
::GetDensityScale() {
  return 1.0e-9;
}

