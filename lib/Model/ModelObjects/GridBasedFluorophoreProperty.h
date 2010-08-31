#ifndef _GRID_BASED_FLUOROPHORE_PROPERTY_H_
#define _GRID_BASED_FLUOROPHORE_PROPERTY_H_

#include <FluorophoreModelObjectProperty.h>

class vtkAppendPoints;
class vtkUnstructuredGridAlgorithm;


class GridBasedFluorophoreProperty : public FluorophoreModelObjectProperty {

 public:
  GridBasedFluorophoreProperty(const std::string& name,
                               vtkUnstructuredGridAlgorithm* gridSource,
                               bool editable = false,
                               bool optimizable = true);
  virtual ~GridBasedFluorophoreProperty();

  virtual int GetNumberOfFluorophores();

 protected:
  GridBasedFluorophoreProperty() {};

  virtual double GetDensityScale();

  vtkSmartPointer<vtkUnstructuredGridAlgorithm> m_GridSource;
  vtkSmartPointer<vtkAppendPoints>              m_PointSource;
};


#endif // _GRID_BASED_FLUOROPHORE_PROPERTY_H_
