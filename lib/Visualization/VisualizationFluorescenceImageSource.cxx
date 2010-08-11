#include <VisualizationFluorescenceImageSource.h>

#include <FluorescenceSimulation.h>
#include <ModelObject.h>
#include <ModelObjectList.h>
#include <ModelObjectProperty.h>
#include <ModelObjectPropertyList.h>
#include <Simulation.h>
#include <Visualization.h>

#include <vtkPolyDataCollection.h>


VisualizationFluorescenceImageSource
::VisualizationFluorescenceImageSource() {
  m_Visualization = NULL;
}


VisualizationFluorescenceImageSource
::~VisualizationFluorescenceImageSource() {

}


void
VisualizationFluorescenceImageSource
::SetVisualization(Visualization* vis) {
  m_Visualization = vis;
}


vtkImageData*
VisualizationFluorescenceImageSource
::GenerateFluorescenceImage() {
  if (!m_Visualization)
    return NULL;

  return m_Visualization->GenerateFluorescenceImage();
}


vtkImageData*
VisualizationFluorescenceImageSource
::GenerateFluorescenceStackImage() {
  if (!m_Visualization)
    return NULL;

  return m_Visualization->GenerateFluorescenceStackImage();
}


void
VisualizationFluorescenceImageSource
::ComputePointsGradient() {
  m_Visualization->ComputeFluorescencePointsGradient();
}


vtkPolyDataCollection*
VisualizationFluorescenceImageSource
::GetPointGradientsForModelObject(int objectIndex) {
  if (!m_Visualization) return 0;

  return m_Visualization->GetPointGradientsForModelObject(objectIndex);
}


int
VisualizationFluorescenceImageSource
::GetNumberOfParameters() {
  if (!m_Visualization) return 0;

  // Count up active parameters
  ModelObjectList* mol = m_Visualization->GetSimulation()->
    GetModelObjectList();
  if (!mol) return 0;

  int activeCount = 0;
  for (unsigned int i = 0; i < mol->GetSize(); i++) {
    ModelObject* mo = mol->GetModelObjectAtIndex(i);

    ModelObjectPropertyList* mopl = mo->GetPropertyList();
    for (int prop = 0; prop < mopl->GetSize(); prop++) {
      ModelObjectProperty* property = mopl->GetProperty(prop);
      if (property->IsOptimizable() && property->GetOptimize())
        activeCount++;
    }
  }

  return activeCount;
}


void
VisualizationFluorescenceImageSource
::SetParameters(double* params) {
  if (!m_Visualization) return;

  ModelObjectList* mol = m_Visualization->GetSimulation()->
    GetModelObjectList();
  if (!mol) return;

  int index = 0;
  for (unsigned int i = 0; i < mol->GetSize(); i++) {
    ModelObject* mo = mol->GetModelObjectAtIndex(i);
    bool paramChanged = false;

    ModelObjectPropertyList* mopl = mo->GetPropertyList();
    for (int prop = 0; prop < mopl->GetSize(); prop++) {
      ModelObjectProperty* property = mopl->GetProperty(prop);
      if (property->IsOptimizable() && property->GetOptimize()) {
        property->SetDoubleValue(params[index++]);
        paramChanged = true;
      }
    }

    // Sully the model object if a parameter has been changed
    if (paramChanged)
      mo->Sully();
  }
}


void
VisualizationFluorescenceImageSource
::GetParameters(double* params) {
  if (!m_Visualization) return;

  ModelObjectList* mol = m_Visualization->GetSimulation()->
    GetModelObjectList();
  if (!mol) return;

  int index = 0;
  for (unsigned int i = 0; i < mol->GetSize(); i++) {
    ModelObject* mo = mol->GetModelObjectAtIndex(i);

    ModelObjectPropertyList* mopl = mo->GetPropertyList();
    for (int prop = 0; prop < mopl->GetSize(); prop++) {
      ModelObjectProperty* property = mopl->GetProperty(prop);
      if (property->IsOptimizable() && property->GetOptimize()) {
        params[index++] = property->GetDoubleValue();
        mo->Sully();
      }
    }
  }
}


int*
VisualizationFluorescenceImageSource
::GetDimensions() {
  FluorescenceSimulation* fluoroSim = m_Visualization->GetSimulation()->
    GetFluorescenceSimulation();

  m_Dimensions[0] = static_cast<int>(fluoroSim->GetImageWidth());
  m_Dimensions[1] = static_cast<int>(fluoroSim->GetImageHeight());
  m_Dimensions[2] = static_cast<int>(fluoroSim->GetNumberOfFocalPlanes());

  return m_Dimensions;
}


double*
VisualizationFluorescenceImageSource
::GetSpacing() {
  FluorescenceSimulation* fluoroSim = m_Visualization->GetSimulation()->
    GetFluorescenceSimulation();

  m_Spacing[0] = fluoroSim->GetPixelSize();
  m_Spacing[1] = fluoroSim->GetPixelSize();
  m_Spacing[2] = fluoroSim->GetFocalPlaneSpacing();

  return m_Spacing;
}
