#ifndef _NELDER_MEAD_FLUORESCENCE_OPTIMIZER_H_
#define _NELDER_MEAD_FLUORESCENCE_OPTIMIZER_H_

#include <itkAmoebaOptimizer.h>

#include <ITKFluorescenceOptimizer.h>


class NelderMeadFluorescenceOptimizer : public ITKFluorescenceOptimizer {

 public:
  
  static const char* OPTIMIZER_ELEM;

  static const char* MAXIMUM_ITERATIONS_PARAM;

  static const char* PARAMETERS_CONVERGENCE_TOLERANCE_PARAM;

  typedef itk::AmoebaOptimizer NelderMeadOptimizerType;


  /** Constructor/destructor. */
  NelderMeadFluorescenceOptimizer(DirtyListener* listener);
  virtual ~NelderMeadFluorescenceOptimizer();

  virtual void Optimize();

 protected:
  NelderMeadFluorescenceOptimizer() {};

};

#endif // _NELDER_MEAD_FLUORESCENCE_OPTIMIZER_H_
