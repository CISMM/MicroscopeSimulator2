#include <VolumeUniformFluorophoreProperty.h>

#include <vtkGlyph3D.h>
#include <vtkPolyDataToTetrahedralGrid.h>
#include <vtkProgrammableGlyphFilter.h>
#include <vtkUnstructuredGridAlgorithm.h>
#include <vtkVolumeUniformPointSampler.h>


VolumeUniformFluorophoreProperty::
VolumeUniformFluorophoreProperty(const std::string& name,
                                 vtkUnstructuredGridAlgorithm* gridSource,
                                 bool editable, bool optimizable)
  : UniformFluorophoreProperty(name, editable, optimizable) {
  
  m_GridSource = gridSource;

  m_VolumeSampler = vtkSmartPointer<vtkVolumeUniformPointSampler>::New();
  m_VolumeSampler->SetInputConnection(m_GridSource->GetOutputPort());
  m_Sampler = m_VolumeSampler;

  m_Glypher->SetInputConnection(m_Sampler->GetOutputPort());

  SetDensity(100.0);
  SetSamplingModeToFixedDensity();
  SetSamplePatternToSinglePoint();
}


VolumeUniformFluorophoreProperty::
~VolumeUniformFluorophoreProperty() {
}


double
VolumeUniformFluorophoreProperty
::GetGeometryVolume() {
  return m_VolumeSampler->GetVolume() * 1.0e-9;
}


double
VolumeUniformFluorophoreProperty
::GetDensityScale() {
  return 1.0e-9;
}
