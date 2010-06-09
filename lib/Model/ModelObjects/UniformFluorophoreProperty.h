#ifndef _UNIFORM_FLUOROPHORE_PROPERTY_H_
#define _UNIFORM_FLUOROPHORE_PROPERTY_H_

#include <FluorophoreModelObjectProperty.h>

class vtkGlyph3D;
class vtkPointRingSource;
class vtkUniformPointSampler;


class UniformFluorophoreProperty : public FluorophoreModelObjectProperty {

 public:
  UniformFluorophoreProperty(const std::string& name,
                             vtkPolyDataAlgorithm* geometry,
                             bool editable = false,
                             bool optimizable = true);
  virtual ~UniformFluorophoreProperty();

  void UseFixedDensity();
  bool GetUseFixedDensity();

  void UseFixedNumberOfFluorophores();
  bool GetUseFixedNumberOfFluorophores();

  void   SetDensity(double density);
  double GetDensity();

  void SetNumberOfFluorophores(int number);
  int  GetNumberOfFluorophores();

  void UseOneFluorophorePerSample();
  bool GetUseOneFluorophorePerSample();

  void UseRingClusterPerSample();
  bool GetUseRingClusterPerSample();

  void SetFluorophoresAroundRing(int numFluorophores);
  int  GetFluorophoresAroundRing();

  void   SetRingRadius(double radius);
  double GetRingRadius();

  virtual void GetXMLConfiguration(xmlNodePtr root);
  virtual void RestoreFromXML(xmlNodePtr root);

 protected:
  UniformFluorophoreProperty() {};

  double m_Density;

  vtkSmartPointer<vtkUniformPointSampler> m_Sampler;

  bool   m_UsePointRingCluster;
  double m_PointRingRadius;
  int    m_NumberOfFluorophores;

  vtkSmartPointer<vtkPointRingSource> m_PointRingSource;

  vtkSmartPointer<vtkGlyph3D> m_PointRingGlypher;

  virtual double GetDensityScale() = 0;

};

#endif // _UNIFORM_FLUOROPHORE_PROPERTY_H_
