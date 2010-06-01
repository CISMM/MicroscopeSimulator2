#ifndef _SURFACE_UNIFORM_FLUOROPHORE_PROPERTY_H_
#define _SURFACE_UNIFORM_FLUOROPHORE_PROPERTY_H_

#include <FluorophoreModelObjectProperty.h>

class vtkSurfaceUniformPointSampler;


class SurfaceUniformFluorophoreProperty : public FluorophoreModelObjectProperty {

 public:
  SurfaceUniformFluorophoreProperty(const std::string& name,
                                    vtkPolyDataAlgorithm* geometry,
                                    bool editable = false,
                                    bool optimizable = true);
  virtual ~SurfaceUniformFluorophoreProperty();

  void UseFixedDensity();
  bool GetUseFixedDensity();

  void UseFixedNumberOfFluorophores();
  bool GetUseFixedNumberOfFluorophores();

  void   SetDensity(double density);
  double GetDensity();

  void SetNumberOfFluorophores(int number);
  int  GetNumberOfFluorophores();

  double GetGeometryArea();

  virtual void GetXMLConfiguration(xmlNodePtr root);
  virtual void RestoreFromXML(xmlNodePtr root);

 protected:
  SurfaceUniformFluorophoreProperty() {};

  double m_Density;

  vtkSmartPointer<vtkSurfaceUniformPointSampler> m_Sampler;

};


#endif // _SURFACE_UNIFORM_FLUOROPHORE_PROPERTY_H_
