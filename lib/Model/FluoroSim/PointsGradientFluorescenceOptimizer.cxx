#include <PointsGradientFluorescenceOptimizer.h>

#include <FluorescenceSimulation.h>
#include <FluorophoreModelObjectProperty.h>
#include <ImageModelObject.h>
#include <ModelObjectList.h>
#include <VisualizationFluorescenceImageSource.h>


const char* PointsGradientFluorescenceOptimizer::OPTIMIZER_ELEM = "PointsGradientFluorescenceOptimizer";

const char* PointsGradientFluorescenceOptimizer::STEP_SIZE_PARAM = "Step Size";

const char* PointsGradientFluorescenceOptimizer::ITERATIONS_PARAM = "Iterations";

const char* PointsGradientFluorescenceOptimizer::GAUSSIAN_NOISE_OBJECTIVE_FUNCTION =
  "Gaussian Noise Maximum Likelihood";

const char* PointsGradientFluorescenceOptimizer::POISSON_NOISE_OBJECTIVE_FUNCTION =
  "Poisson Noise Maximum Likelihood";


PointsGradientFluorescenceOptimizer
::PointsGradientFluorescenceOptimizer(DirtyListener* listener)
  : FluorescenceOptimizer(listener) {
  AddObjectiveFunctionName(std::string(GAUSSIAN_NOISE_OBJECTIVE_FUNCTION));
  //AddObjectiveFunctionName(std::string(POISSON_NOISE_OBJECTIVE_FUNCTION));

  Variant stepSize;
  stepSize.dValue = 1.0;
  AddOptimizerParameter(STEP_SIZE_PARAM, DOUBLE_TYPE, stepSize);

  Variant maxIterations;
  maxIterations.iValue = 100;
  AddOptimizerParameter(ITERATIONS_PARAM, INT_TYPE, maxIterations);
}


PointsGradientFluorescenceOptimizer
::~PointsGradientFluorescenceOptimizer() {

}


void
PointsGradientFluorescenceOptimizer
::Optimize() {

  // TODO - set the cost function in the fluorescence image source. Currently,
  // only the Gaussian and Poisson noise cost functions are supported.

  double stepSize = GetOptimizerParameterValue(STEP_SIZE_PARAM).dValue;
  int iterations = GetOptimizerParameterValue(ITERATIONS_PARAM).iValue;
  for (int i = 0; i < iterations; i++) {
    
    // Compute the gradient. We don't need to read or process image data 
    // here because it is all done on the GPU.
    VisualizationFluorescenceImageSource* imageSource =
      dynamic_cast<VisualizationFluorescenceImageSource*>
      (m_FluoroSim->GetFluorescenceImageSource());
    imageSource->ComputePointsGradient();
    
    // Loop over the model objects. For each model object, get the
    // gradient of its points and tell the model object about the gradient.
    // We assume the model object will know how to adjust its parameters
    // given the point gradient.
    for (size_t objectIndex = 0; objectIndex < m_ModelObjectList->GetSize(); objectIndex++) {
      ModelObjectPtr modelObject = m_ModelObjectList->GetModelObjectAtIndex(objectIndex);

      for (int fluorophorePropertyIndex = 0;
           fluorophorePropertyIndex < modelObject->GetNumberOfFluorophoreProperties();
           fluorophorePropertyIndex++) {
        if (!modelObject->GetFluorophoreProperty(fluorophorePropertyIndex)->GetEnabled()) {
          continue;
        }

        int numPoints;
        float* gradientSrc = imageSource->
          GetPointsGradientForFluorophoreProperty
          (objectIndex, fluorophorePropertyIndex, numPoints);
        float* gradient = new float[3*numPoints];
        memcpy(gradient, gradientSrc, sizeof(float)*3*numPoints);
        
        // Scale the gradient
        for (int i = 0; i < numPoints; i++) {
          gradient[3*i + 0] *= stepSize;
          gradient[3*i + 1] *= stepSize;
          gradient[3*i + 2] *= stepSize;
        }
        
        m_ModelObjectList->GetModelObjectAtIndex(objectIndex)->
          ApplySampleForces(fluorophorePropertyIndex, gradient);

        delete[] gradient;
      }
    }
  }
}
