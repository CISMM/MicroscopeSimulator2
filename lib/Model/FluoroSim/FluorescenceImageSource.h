#ifndef _FLUORESCENCE_IMAGE_SOURCE_H_
#define _FLUORESCENCE_IMAGE_SOURCE_H_

/* A fluorescence image source is responsible for generating and exporting
   fluorescence images from a set of parameters. */

// Forward declarations
class vtkImageData;
class vtkPolyDataCollection;


class FluorescenceImageSource {

 public:
  FluorescenceImageSource() {};
  virtual ~FluorescenceImageSource() {};

  // Exports a 2D fluorescence image
  virtual vtkImageData* GenerateFluorescenceImage() = 0;

  // Exports a stack of 2D fluorescence images (i.e. a 3D stack)
  virtual vtkImageData* GenerateFluorescenceStackImage() = 0;

  // Computes gradients for the point samples from all the
  // model objects.
  virtual void ComputePointsGradient() = 0;

  // Get point gradient for a model object.
  virtual float* GetPointsGradientForFluorophoreProperty(int objectIndex, 
                                                         int fluorophorePropertyIndex,
                                                         int& numPoints) = 0;
  virtual vtkPolyDataCollection* GetPointGradientsForModelObject(int objectIndex) = 0;

  virtual int GetNumberOfParameters() = 0;

  virtual void SetParameters(double* params) = 0;
  virtual void GetParameters(double* params) = 0;

  virtual int* GetDimensions() = 0;
  virtual double* GetSpacing() = 0;

 protected:
  int    m_Dimensions[3];
  double m_Spacing[3];

};

#endif // _FLUORESCENCE_IMAGE_SOURCE_H_
