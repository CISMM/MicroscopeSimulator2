#ifndef _UNIFORM_FLUOROPHORE_PROPERTY_H_
#define _UNIFORM_FLUOROPHORE_PROPERTY_H_

#include <FluorophoreModelObjectProperty.h>

class vtkMinimalStandardRandomSequence;
class vtkPassThrough;
class vtkPointRingSource;
class vtkProgrammableGlyphFilter;
class vtkTransform;
class vtkTransformPolyDataFilter;
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

  void RegenerateFluorophores();
 
  virtual void GetXMLConfiguration(xmlNodePtr root);
  virtual void RestoreFromXML(xmlNodePtr root);

 protected:
  UniformFluorophoreProperty() {};

  SamplingMode_t m_SamplingMode;

  vtkSmartPointer<vtkUniformPointSampler> m_Sampler;

  vtkSmartPointer<vtkPassThrough> m_PassThroughFilter;

  SamplePattern_t m_SamplePattern;

  vtkSmartPointer<vtkTransform>               m_Transform;
  vtkSmartPointer<vtkTransformPolyDataFilter> m_TransformFilter;

  double                              m_PointRingRadius;
  vtkSmartPointer<vtkPointRingSource> m_PointRingSource;

  vtkSmartPointer<vtkProgrammableGlyphFilter> m_Glypher;

  bool m_RandomizePatternOrientations;

  vtkSmartPointer<vtkMinimalStandardRandomSequence> m_RandomSequence;

  virtual double GetDensityScale() = 0;

  static void GlyphFunction(void* arg);

  vtkTransform*                     GetGlyphTransform();
  vtkProgrammableGlyphFilter*       GetGlypher();
  vtkMinimalStandardRandomSequence* GetRandomSequence();

};

#endif // _UNIFORM_FLUOROPHORE_PROPERTY_H_
