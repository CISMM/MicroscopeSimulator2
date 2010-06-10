#ifndef _UNIFORM_FLUOROPHORE_PROPERTY_H_
#define _UNIFORM_FLUOROPHORE_PROPERTY_H_

#include <FluorophoreModelObjectProperty.h>

class vtkGlyph3D;
class vtkPointRingSource;
class vtkUniformPointSampler;


class UniformFluorophoreProperty : public FluorophoreModelObjectProperty {

 public:

  typedef enum {
    FIXED_DENSITY,
    FIXED_NUMBER
  } SamplingMode_t;

  typedef enum {
    SINGLE_POINT,
    POINT_RING
  } SamplePattern_t;

  UniformFluorophoreProperty(const std::string& name,
                             vtkPolyDataAlgorithm* geometry,
                             bool editable = false,
                             bool optimizable = true);
  virtual ~UniformFluorophoreProperty();

  void SetSamplingMode(SamplingMode_t mode);
  void SetSamplingModeToFixedDensity();
  void SetSamplingModeToFixedNumber();
  SamplingMode_t GetSamplingMode();

  void   SetDensity(double density);
  double GetDensity();

  void SetNumberOfFluorophores(int number);
  int  GetNumberOfFluorophores();

  void SetSamplePattern(SamplePattern_t pattern);
  void SetSamplePatternToSinglePoint();
  void SetSamplePatternToPointRing();
  SamplePattern_t GetSamplePattern();

  void SetNumberOfRingFluorophores(int numFluorophores);
  int  GetNumberOfRingFluorophores();

  void   SetRingRadius(double radius);
  double GetRingRadius();

  virtual void GetXMLConfiguration(xmlNodePtr root);
  virtual void RestoreFromXML(xmlNodePtr root);

 protected:
  UniformFluorophoreProperty() {};

  SamplingMode_t m_SamplingMode;
  double         m_Density;

  vtkSmartPointer<vtkUniformPointSampler> m_Sampler;

  SamplePattern_t m_SamplePattern;

  double m_PointRingRadius;
  int    m_NumberOfFluorophores;

  vtkSmartPointer<vtkPointRingSource> m_PointRingSource;

  vtkSmartPointer<vtkGlyph3D> m_PointRingGlypher;

  virtual double GetDensityScale() = 0;

};

#endif // _UNIFORM_FLUOROPHORE_PROPERTY_H_
