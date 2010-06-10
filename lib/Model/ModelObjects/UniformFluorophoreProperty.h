#ifndef _UNIFORM_FLUOROPHORE_PROPERTY_H_
#define _UNIFORM_FLUOROPHORE_PROPERTY_H_

#include <FluorophoreModelObjectProperty.h>

class vtkGlyph3D;
class vtkPointRingSource;
class vtkUniformPointSampler;


class UniformFluorophoreProperty : public FluorophoreModelObjectProperty {

 public:

  static const char* DENSITY_ATT;
  static const char* NUMBER_OF_FLUOROPHORES_ATT;
  static const char* SAMPLING_MODE_ATT;
  static const char* FIXED_DENSITY_VALUE;
  static const char* FIXED_NUMBER_VALUE;
  static const char* SAMPLE_PATTERN_ATT;
  static const char* SINGLE_POINT_VALUE;
  static const char* POINT_RING_VALUE;
  static const char* NUMBER_OF_RING_FLUOROPHORES_ATT;
  static const char* RING_RADIUS_ATT;
  static const char* RANDOMIZE_PATTERN_ORIENTATIONS_ATT;

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

  void SetRandomizePatternOrientations(bool enabled);
  void RandomizePatternOrientationsOn();
  void RandomizePatternOrientationsOff();
  bool GetRandomizePatternOrientations();
  
  virtual void GetXMLConfiguration(xmlNodePtr root);
  virtual void RestoreFromXML(xmlNodePtr root);

 protected:
  UniformFluorophoreProperty() {};

  SamplingMode_t m_SamplingMode;
  double         m_Density;
  int            m_NumberOfFluorophores;

  vtkSmartPointer<vtkUniformPointSampler> m_Sampler;

  SamplePattern_t m_SamplePattern;

  double                              m_PointRingRadius;
  vtkSmartPointer<vtkPointRingSource> m_PointRingSource;
  vtkSmartPointer<vtkGlyph3D>         m_PointRingGlypher;

  bool m_RandomizePatternOrientations;

  virtual double GetDensityScale() = 0;

};

#endif // _UNIFORM_FLUOROPHORE_PROPERTY_H_
