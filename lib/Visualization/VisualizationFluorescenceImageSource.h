#ifndef _VISUALIZATION_FLUORESCENCE_IMAGE_SOURCE_H_
#define _VISUALIZATION_FLUORESCENCE_IMAGE_SOURCE_H_

#include <FluorescenceImageSource.h>

class Simulation;
class Visualization;


class VisualizationFluorescenceImageSource : public FluorescenceImageSource {

 public:
  VisualizationFluorescenceImageSource();
  virtual ~VisualizationFluorescenceImageSource();

  void SetVisualization(Visualization* vis);
  
  virtual vtkImageData* GenerateFluorescenceImage();
  virtual vtkImageData* GenerateFluorescenceStackImage();

  virtual int GetNumberOfParameters();

  virtual void SetParameters(double* params);
  virtual void GetParameters(double* params);

  virtual int* GetDimensions();
  virtual double* GetSpacing();

 protected:
  Visualization* m_Visualization;
};


#endif // _VISUALIZATION_FLUORESCENCE_IMAGE_SOURCE_H_
