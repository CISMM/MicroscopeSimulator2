#include <VolumeUniformFluorophoreProperty.h>

#include <vtkPolyDataAlgorithm.h>
#include <vtkPolyDataToTetrahedralGrid.h>
#include <vtkVolumeUniformPointSampler.h>

#include <vtkGlyph3D.h>

VolumeUniformFluorophoreProperty::
VolumeUniformFluorophoreProperty(const std::string& name,
                                 vtkPolyDataAlgorithm* geometry,
                                 bool editable, bool optimizable)
  : UniformFluorophoreProperty(name, geometry, editable, optimizable) {

  vtkSmartPointer<vtkPolyDataToTetrahedralGrid> tetrahedralizer =
    vtkSmartPointer<vtkPolyDataToTetrahedralGrid>::New();
  tetrahedralizer->SetInputConnection(geometry->GetOutputPort());
  
  m_VolumeSampler = vtkSmartPointer<vtkVolumeUniformPointSampler>::New();
  m_VolumeSampler->SetInputConnection(tetrahedralizer->GetOutputPort());
  m_Sampler = m_VolumeSampler;

  m_PointRingGlypher->SetInputConnection(m_Sampler->GetOutputPort());

  SetDensity(100.0);
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
