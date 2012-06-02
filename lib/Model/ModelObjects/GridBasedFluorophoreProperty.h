#ifndef _GRID_BASED_FLUOROPHORE_PROPERTY_H_
#define _GRID_BASED_FLUOROPHORE_PROPERTY_H_

#include <FluorophoreModelObjectProperty.h>

class vtkPartialVolumeModeller;
class vtkThresholdPoints;
class vtkUnstructuredGridAlgorithm;


class GridBasedFluorophoreProperty : public FluorophoreModelObjectProperty {

 public:

  static const char* SAMPLE_SPACING_ATT;

  GridBasedFluorophoreProperty(const std::string& name,
                               vtkUnstructuredGridAlgorithm* gridSource,
                               bool editable = false,
                               bool optimizable = true);
  virtual ~GridBasedFluorophoreProperty();

  virtual void   SetSampleSpacing(double spacing);
  virtual double GetSampleSpacing();

  virtual int GetNumberOfFluorophores();

  virtual void Update();

  virtual void GetXMLConfiguration(xmlNodePtr root);
  virtual void RestoreFromXML(xmlNodePtr root);

 protected:
  GridBasedFluorophoreProperty() {};

  virtual double GetDensityScale();

  double m_SampleSpacing;

  vtkSmartPointer<vtkUnstructuredGridAlgorithm> m_GridSource;
  vtkSmartPointer<vtkThresholdPoints>           m_Threshold;
  vtkSmartPointer<vtkPartialVolumeModeller>     m_PartialVolumeVoxelizer;
};


#endif // _GRID_BASED_FLUOROPHORE_PROPERTY_H_
