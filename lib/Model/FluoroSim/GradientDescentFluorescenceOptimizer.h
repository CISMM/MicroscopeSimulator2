#ifndef _GRADIENT_DESCENT_FLUORESCENCE_OPTIMIZER_H_
#define _GRADIENT_DESCENT_FLUORESCENCE_OPTIMIZER_H_

#include <itkGradientDescentOptimizer.h>

#include <ITKFluorescenceOptimizer.h>


class GradientDescentFluorescenceOptimizer : public ITKFluorescenceOptimizer {

 public:

  static const char* OPTIMIZER_ELEM;

  typedef itk::GradientDescentOptimizer GradientDescentOptimizerType;

  /** Constructor/destructor. */
  GradientDescentFluorescenceOptimizer(DirtyListener* listener);
  virtual ~GradientDescentFluorescenceOptimizer();

  virtual void Optimize();

 protected:
  GradientDescentFluorescenceOptimizer() {}

};


#endif // _GRADIENT_DESCENT_FLUORESCENCE_OPTIMIZER_H_
