#ifndef _VOLUME_UNIFORM_FLUOROPHORE_PROPERTY_H_
#define _VOLUME_UNIFORM_FLUOROPHORE_PROPERTY_H_

#include <FluorophoreModelObjectProperty.h>

class vtkVolumeUniformPointSampler;


class VolumeUniformFluorophoreProperty : public FluorophoreModelObjectProperty {

 public:
  VolumeUniformFluorophoreProperty(const std::string& name,
                                    vtkPolyDataAlgorithm* geometry,
                                    bool editable = false,
                                    bool optimizable = true);
  virtual ~VolumeUniformFluorophoreProperty();

  void UseFixedDensity();
  bool GetUseFixedDensity();

  void UseFixedNumberOfFluorophores();
  bool GetUseFixedNumberOfFluorophores();

  void   SetDensity(double density);
  double GetDensity();

  void SetNumberOfFluorophores(int number);
  int  GetNumberOfFluorophores();

  double GetGeometryVolume();

  virtual void GetXMLConfiguration(xmlNodePtr root);
  virtual void RestoreFromXML(xmlNodePtr root);

 protected:
  double m_Density;

  vtkSmartPointer<vtkVolumeUniformPointSampler> m_Sampler;

};


#endif // _VOLUME_UNIFORM_FLUOROPHORE_PROPERTY_H_
