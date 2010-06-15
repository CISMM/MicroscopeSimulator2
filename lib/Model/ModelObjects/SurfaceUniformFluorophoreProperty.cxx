#include <SurfaceUniformFluorophoreProperty.h>

#include <vtkGlyph3D.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkProgrammableGlyphFilter.h>
#include <vtkSurfaceUniformPointSampler.h>
#include <vtkTriangleFilter.h>


SurfaceUniformFluorophoreProperty::
SurfaceUniformFluorophoreProperty(const std::string& name,
                                  vtkPolyDataAlgorithm* geometry,
                                  bool editable, bool optimizable) 
  : UniformFluorophoreProperty(name, geometry, editable, optimizable) {

  vtkSmartPointer<vtkTriangleFilter> triangulizer = vtkSmartPointer<vtkTriangleFilter>::New();
  triangulizer->PassLinesOff();
  triangulizer->PassVertsOff();
  triangulizer->SetInputConnection(geometry->GetOutputPort());
  
  m_SurfaceSampler = vtkSmartPointer<vtkSurfaceUniformPointSampler>::New();
  m_SurfaceSampler->SetInputConnection(triangulizer->GetOutputPort());
  m_Sampler = m_SurfaceSampler;

  m_Glypher->SetInputConnection(m_Sampler->GetOutputPort());

  SetDensity(100.0);
  SetSamplingModeToFixedDensity();
  SetSamplePatternToSinglePoint();
}


SurfaceUniformFluorophoreProperty::
~SurfaceUniformFluorophoreProperty() {
}


double
SurfaceUniformFluorophoreProperty
::GetGeometryArea() {
  return m_SurfaceSampler->GetSurfaceArea() * 1.0e-6;
}


double
SurfaceUniformFluorophoreProperty
::GetDensityScale() {
  return 1.0e-6;
}
