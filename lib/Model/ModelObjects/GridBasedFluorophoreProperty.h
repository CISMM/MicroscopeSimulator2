#ifndef _GRID_BASED_FLUOROPHORE_PROPERTY_H_
#define _GRID_BASED_FLUOROPHORE_PROPERTY_H_

#include <FluorophoreModelObjectProperty.h>

class vtkAppendPoints;
class vtkProgrammableFilter;
class vtkUnstructuredGridAlgorithm;


class GridBasedFluorophoreProperty : public FluorophoreModelObjectProperty {

 public:
  GridBasedFluorophoreProperty(const std::string& name,
                               vtkUnstructuredGridAlgorithm* gridSource,
                               bool editable = false,
                               bool optimizable = true);
  virtual ~GridBasedFluorophoreProperty();

  virtual int GetNumberOfFluorophores();

  typedef struct _vtkProgrammableFilterUserData {
    vtkProgrammableFilter* filter;
    double spacing;
  } vtkProgrammableFilterUserData;

 protected:
  GridBasedFluorophoreProperty() {};

  virtual double GetDensityScale();

  vtkSmartPointer<vtkUnstructuredGridAlgorithm> m_GridSource;
  vtkSmartPointer<vtkAppendPoints>              m_PointSource;
  vtkProgrammableFilterUserData                 m_UserData;
};


#endif // _GRID_BASED_FLUOROPHORE_PROPERTY_H_
