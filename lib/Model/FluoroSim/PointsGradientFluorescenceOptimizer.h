#ifndef _POINTS_GRADIENT_FLUORESCENCE_OPTIMIZER_H_
#define _POINTS_GRADIENT_FLUORESCENCE_OPTIMIZER_H_

#include <FluorescenceOptimizer.h>


class PointsGradientFluorescenceOptimizer : public FluorescenceOptimizer {

 public:
  PointsGradientFluorescenceOptimizer();
  virtual ~PointsGradientFluorescenceOptimizer();

  virtual void Optimize();

};


#endif // _POINTS_GRADIENT_FLUORESCENCE_OPTIMIZER_H_
