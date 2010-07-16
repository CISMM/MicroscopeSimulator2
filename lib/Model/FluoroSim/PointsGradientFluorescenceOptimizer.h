#ifndef _POINTS_GRADIENT_FLUORESCENCE_OPTIMIZER_H_
#define _POINTS_GRADIENT_FLUORESCENCE_OPTIMIZER_H_

#include <FluorescenceOptimizer.h>


class PointsGradientFluorescenceOptimizer : public FluorescenceOptimizer {

 public:

  static const char* OPTIMIZER_ELEM;

  static const char* STEP_SIZE_PARAM;

  static const char* ITERATIONS_PARAM;

  static const char* GAUSSIAN_NOISE_OBJECTIVE_FUNCTION;
  
  static const char* POISSON_NOISE_OBJECTIVE_FUNCTION;


  PointsGradientFluorescenceOptimizer(DirtyListener* listener);
  virtual ~PointsGradientFluorescenceOptimizer();

  virtual void Optimize();

 protected:
  PointsGradientFluorescenceOptimizer() {};

};


#endif // _POINTS_GRADIENT_FLUORESCENCE_OPTIMIZER_H_
