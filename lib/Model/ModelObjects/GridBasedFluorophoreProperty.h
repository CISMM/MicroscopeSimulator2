#ifndef _GRID_BASED_FLUOROPHORE_PROPERTY_H_
#define _GRID_BASED_FLUOROPHORE_PROPERTY_H_

#include <FluorophoreModelObjectProperty.h>

class vtkAppendPoints;
class vtkImplicitModeller;
class vtkProgrammableFilter;
class vtkUnstructuredGridAlgorithm;


class GridBasedFluorophoreProperty : public FluorophoreModelObjectProperty {

 public:
  GridBasedFluorophoreProperty(const std::string& name,
                               vtkUnstructuredGridAlgorithm* gridSource,
                               bool editable = false,
                               bool optimizable = true);
  virtual ~GridBasedFluorophoreProperty();

  virtual void   SetSampleSpacing(double spacing);
  virtual double GetSampleSpacing();

  virtual int GetNumberOfFluorophores();

  virtual void Update();

  typedef struct _vtkProgrammableFilterUserData {
    vtkProgrammableFilter* filter;
    double spacing;
  } vtkProgrammableFilterUserData;

 protected:
  GridBasedFluorophoreProperty() {};

  virtual double GetDensityScale();

  double m_SampleSpacing;

  vtkSmartPointer<vtkUnstructuredGridAlgorithm> m_GridSource;
  vtkSmartPointer<vtkImplicitModeller>          m_Voxelizer1;
  vtkSmartPointer<vtkImplicitModeller>          m_Voxelizer2;
  vtkSmartPointer<vtkAppendPoints>              m_PointSource;
  vtkProgrammableFilterUserData                 m_UserData;
};


#endif // _GRID_BASED_FLUOROPHORE_PROPERTY_H_
