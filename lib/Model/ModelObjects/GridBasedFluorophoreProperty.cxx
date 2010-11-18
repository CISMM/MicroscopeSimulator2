#include <GridBasedFluorophoreProperty.h>

#include <vtkAppendPolyData.h>
#include <vtkArrayCalculator.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImplicitModeller.h>
#include <vtkPointData.h>
#include <vtkProgrammableFilter.h>
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

  // Create a new data array to hold the output intensities
  vtkFloatArray* outputArray = vtkFloatArray::New();
  outputArray->SetName("Intensity");
  outputArray->SetNumberOfComponents(1);
  outputArray->SetNumberOfTuples(inputArray->GetNumberOfTuples());
  outputArray->Allocate(inputArray->GetNumberOfTuples());

  // Iterate over the data values and apply the intensity falloff calculation
  double alpha = -log(0.5) / (0.25 * spacing);
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
  
  double sampleSpacing = 50.0;
  double boxSize = 2000.0;
  int dims[3];
  for (int i = 0; i < 3; i++) {
    dims[i] = ceil(boxSize / sampleSpacing);
  }

  // Set up the class that performs the distance computation
  // on a structured grid
  vtkSmartPointer<vtkImplicitModeller> vox1 = 
    vtkSmartPointer<vtkImplicitModeller>::New();
  vox1->SetInputConnection(gridSource->GetOutputPort());
  vox1->SetOutputScalarTypeToFloat();
  vox1->SetModelBounds(-0.5*boxSize, 0.5*boxSize, -0.5*boxSize, 0.5*boxSize,
                       -0.5*boxSize, 0.5*boxSize);
  vox1->SetSampleDimensions(dims);
  vox1->Update();
  vox1->GetOutput()->GetPointData()->GetArray(0)->SetName("Distance");

  vtkSmartPointer<vtkThresholdPoints> thold1 =
    vtkSmartPointer<vtkThresholdPoints>::New();
  thold1->ThresholdByLower(sqrt(3*sampleSpacing*sampleSpacing));
  thold1->SetInputConnection(vox1->GetOutputPort());

  vtkSmartPointer<vtkImplicitModeller> vox2 = 
    vtkSmartPointer<vtkImplicitModeller>::New();
  vox2->SetInputConnection(gridSource->GetOutputPort());
  vox2->SetOutputScalarTypeToFloat();
  vox2->SetModelBounds(-0.5*(boxSize+sampleSpacing), 0.5*(boxSize-sampleSpacing), 
                       -0.5*(boxSize+sampleSpacing), 0.5*(boxSize-sampleSpacing),
                       -0.5*(boxSize+sampleSpacing), 0.5*(boxSize-sampleSpacing));
  vox2->SetSampleDimensions(dims);
  vox2->Update();
  vox2->GetOutput()->GetPointData()->GetArray(0)->SetName("Distance");


  vtkSmartPointer<vtkThresholdPoints> thold2 =
    vtkSmartPointer<vtkThresholdPoints>::New();
  thold2->ThresholdByLower(sqrt(3*sampleSpacing*sampleSpacing));
  thold2->SetInputConnection(vox2->GetOutputPort());

  vtkSmartPointer<vtkAppendPolyData> appender =
    vtkSmartPointer<vtkAppendPolyData>::New();
  appender->AddInputConnection(thold1->GetOutputPort());
  appender->AddInputConnection(thold2->GetOutputPort());

  vtkSmartPointer<vtkProgrammableFilter> intensityCalculator =
    vtkSmartPointer<vtkProgrammableFilter>::New();
  intensityCalculator->SetInputConnection(appender->GetOutputPort());
  m_UserData.filter = intensityCalculator;
  m_UserData.spacing = sampleSpacing;
  intensityCalculator->
    SetExecuteMethod(GridBasedFluorophorePropertyIntensityFallOffFunction, &m_UserData);

  m_FluorophoreOutput = intensityCalculator;
}


GridBasedFluorophoreProperty
::~GridBasedFluorophoreProperty() {

}


int
GridBasedFluorophoreProperty
::GetNumberOfFluorophores() {
  return 20*20*20+(19*19*19);
}


double
GridBasedFluorophoreProperty
::GetDensityScale() {
  return 1.0e-9;
}
