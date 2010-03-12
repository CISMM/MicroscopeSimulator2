#ifndef _FLUORESCENCE_OPTIMIZER_H_
#define _FLUORESCENCE_OPTIMIZER_H_

#include <itkFluorescenceImageSource.h>
#include <itkImage.h>

#include <itkAmoebaOptimizer.h>
#include <itkNormalizedCorrelationImageToImageMetric.h>
#include <itkImageToParameterizedImageSourceMetric.h>


class FluorescenceSimulation;
class ModelObjectList;


class FluorescenceOptimizer {

 public:

  typedef float PixelType;
  typedef itk::Image<PixelType, 3> FluorescenceImageType;
  typedef itk::FluorescenceImageSource<FluorescenceImageType> SyntheticImageSourceType;

  // Types for optimization.
  typedef itk::ImageToParameterizedImageSourceMetric<FluorescenceImageType, SyntheticImageSourceType>
    ParameterizedCostFunctionType;
  typedef itk::NormalizedCorrelationImageToImageMetric<FluorescenceImageType, FluorescenceImageType>
    ImageToImageCostFunctionType;
  typedef itk::AmoebaOptimizer OptimizerType;


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

  // The cost function used by the optimizer
  ParameterizedCostFunctionType::Pointer m_CostFunction;

  // The delegate cost function used by m_CostFunction
  ImageToImageCostFunctionType::Pointer  m_ImageToImageCostFunction;
  
  // The optimizer
  OptimizerType::Pointer m_Optimizer;

};

#endif
