#ifndef _FLUORESCENCE_OPTIMIZER_H_
#define _FLUORESCENCE_OPTIMIZER_H_

#include <itkFluorescenceImageSource.h>
#include <itkImage.h>

class FluorescenceSimulation;
class ModelObjectList;


class FluorescenceOptimizer {

 public:

  typedef float PixelType;
  typedef itk::Image<PixelType, 3> FluorescenceImageType;
  typedef itk::FluorescenceImageSource<FluorescenceImageType> SyntheticImageSourceType;

  /** Constructor/destructor. */
  FluorescenceOptimizer();
  virtual ~FluorescenceOptimizer();

  void SetFluorescenceSimulation(FluorescenceSimulation* simulation);

  void SetModelObjectList(ModelObjectList* list);

  void Optimize();

  void SetParameters(double* params);

 protected:
  FluorescenceSimulation* m_FluoroSim;

  ModelObjectList* m_ModelObjectList;

  SyntheticImageSourceType::Pointer m_FluorescenceImageSource;
};

#endif
