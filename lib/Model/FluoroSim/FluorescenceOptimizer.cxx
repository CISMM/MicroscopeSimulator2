#include <FluorescenceOptimizer.h>

#include <vtkImageData.h>

#include <FluorescenceImageSource.h>
#include <FluorescenceSimulation.h>
#include <ModelObjectList.h>

// TEMP
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkImageShiftScale.h>
#include <vtkPNGWriter.h>
// END TEMP

FluorescenceOptimizer
::FluorescenceOptimizer() {

  
}


FluorescenceOptimizer
::~FluorescenceOptimizer() {


}


void
FluorescenceOptimizer
::SetFluorescenceSimulation(FluorescenceSimulation* simulation) {
  m_FluoroSim = simulation;
}


void
FluorescenceOptimizer
::SetModelObjectList(ModelObjectList* list) {
  m_ModelObjectList = list;
}


void
FluorescenceOptimizer
::Optimize() {
  for (double x = 0; x < 3000.0; x += 100) {
    double params[1];
    params[0]= x;
    SetParameters(params);
  }
}


void
FluorescenceOptimizer
::SetParameters(double params[]) {
  if (!m_ModelObjectList)
    return;

  m_FluoroSim->GetFluorescenceImageSource()->SetParameters(params);
}
