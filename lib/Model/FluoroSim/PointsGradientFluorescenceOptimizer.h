#ifndef _POINTS_GRADIENT_FLUORESCENCE_OPTIMIZER_H_
#define _POINTS_GRADIENT_FLUORESCENCE_OPTIMIZER_H_

#include <FluorescenceOptimizer.h>


class PointsGradientFluorescenceOptimizer : public FluorescenceOptimizer {

 public:

  static const char* OPTIMIZER_ELEM;

  PointsGradientFluorescenceOptimizer(DirtyListener* listener);
  virtual ~PointsGradientFluorescenceOptimizer();

  virtual void Optimize();

 protected:
  PointsGradientFluorescenceOptimizer() {};

};


#endif // _POINTS_GRADIENT_FLUORESCENCE_OPTIMIZER_H_
