#include <PointsGradientFluorescenceOptimizer.h>

#include <FluorescenceSimulation.h>
#include <ModelObjectList.h>
#include <VisualizationFluorescenceImageSource.h>


PointsGradientFluorescenceOptimizer
::PointsGradientFluorescenceOptimizer() {
  AddObjectiveFunctionName(std::string("Gaussian Noise Maximum Likelihood"));
  AddObjectiveFunctionName(std::string("Poisson Noise Maximum Likelihood"));
}


PointsGradientFluorescenceOptimizer
::~PointsGradientFluorescenceOptimizer() {

}


void
PointsGradientFluorescenceOptimizer
::Optimize() {

  // TODO - set the cost function in the fluorescence image source. Currently,
  // only the Gaussian and Poisson noise cost functions are supported.
    

  // TODO - figure out convergence criteria

  for (int i = 0; i < 50; i++) {
    
    // Compute the gradient. We don't need to read or process image data here.
    // It is all done on the GPU.
    VisualizationFluorescenceImageSource* imageSource =
      dynamic_cast<VisualizationFluorescenceImageSource*>
      (m_FluoroSim->GetFluorescenceImageSource());
    imageSource->ComputePointsGradient();
    
    // Loop over the model objects. For each model object, get the
    // gradient of its points and tell the model object about the gradient.
    // We assume the model object will know how to adjust its parameters
    // given the point gradient.
    int numPoints;
    float* gradientSrc = imageSource->
      GetPointsGradientForModelObjectAtIndex(0, numPoints);
    float* gradient = new float[3*numPoints];
    memcpy(gradient, gradientSrc, sizeof(float)*3*numPoints);
    
    // Scale the gradient
    float t = 20.0;
    for (int i = 0; i < numPoints; i++) {
      gradient[3*i + 0] *= t;
      gradient[3*i + 1] *= t;
      gradient[3*i + 2] *= t;
    }
    
    m_ModelObjectList->GetModelObjectAtIndex(0)->
      ApplySurfaceSampleForces(gradient);
    
    delete[] gradient;
  }
}
