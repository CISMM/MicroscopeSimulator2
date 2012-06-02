#ifndef _VOLUME_UNIFORM_FLUOROPHORE_PROPERTY_H_
#define _VOLUME_UNIFORM_FLUOROPHORE_PROPERTY_H_

#include <UniformFluorophoreProperty.h>

class vtkUnstructuredGridAlgorithm;
class vtkVolumeUniformPointSampler;


class VolumeUniformFluorophoreProperty : public UniformFluorophoreProperty {

 public:
  VolumeUniformFluorophoreProperty(const std::string& name,
                                    vtkUnstructuredGridAlgorithm* gridSource,
                                    bool editable = false,
                                    bool optimizable = true);
  virtual ~VolumeUniformFluorophoreProperty();

  double GetGeometryVolume();

 protected:
  VolumeUniformFluorophoreProperty() {};

  virtual double GetDensityScale();

  vtkSmartPointer<vtkUnstructuredGridAlgorithm> m_GridSource;
  vtkSmartPointer<vtkVolumeUniformPointSampler> m_VolumeSampler;
};


#endif // _VOLUME_UNIFORM_FLUOROPHORE_PROPERTY_H_
