#ifndef _GRADIENT_DESCENT_FLUORESCENCE_OPTIMIZER_H_
#define _GRADIENT_DESCENT_FLUORESCENCE_OPTIMIZER_H_

#include <itkGradientDescentOptimizer.h>

#include <ITKFluorescenceOptimizer.h>


class GradientDescentFluorescenceOptimizer : public ITKFluorescenceOptimizer {

 public:

  typedef itk::GradientDescentOptimizer GradientDescentOptimizerType;

  /** Constructor/destructor. */
  GradientDescentFluorescenceOptimizer();
  virtual ~GradientDescentFluorescenceOptimizer();

  virtual void Optimize();

};


#endif // _GRADIENT_DESCENT_FLUORESCENCE_OPTIMIZER_H_
