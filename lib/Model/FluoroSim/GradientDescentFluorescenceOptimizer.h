#ifndef _GRADIENT_DESCENT_FLUORESCENCE_OPTIMIZER_H_
#define _GRADIENT_DESCENT_FLUORESCENCE_OPTIMIZER_H_

#define ITK_MANUAL_INSTANTIATION
#include <itkGradientDescentOptimizer.h>
#undef ITK_MANUAL_INSTANTIATION

#include <ITKFluorescenceOptimizer.h>


class GradientDescentFluorescenceOptimizer : public ITKFluorescenceOptimizer {

 public:

  static const char* OPTIMIZER_ELEM;

  static const char* ITERATIONS_PARAM;

  static const char* DERIVATIVE_ESTIMATE_STEP_SIZE;

  typedef itk::GradientDescentOptimizer GradientDescentOptimizerType;

  /** Constructor/destructor. */
  GradientDescentFluorescenceOptimizer(DirtyListener* listener);
  virtual ~GradientDescentFluorescenceOptimizer();

  virtual void Optimize();

 protected:
  GradientDescentFluorescenceOptimizer() {}

};


#endif // _GRADIENT_DESCENT_FLUORESCENCE_OPTIMIZER_H_
