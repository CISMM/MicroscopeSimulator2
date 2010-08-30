#ifndef _GEOMETRY_VERTICES_FLUOROPHORE_PROPERTY_H_
#define _GEOMETRY_VERTICES_FLUOROPHORE_PROPERTY_H_

#include <FluorophoreModelObjectProperty.h>

class vtkPolyDataAlgorithm;


class GeometryVerticesFluorophoreProperty : public FluorophoreModelObjectProperty {

 public:
  GeometryVerticesFluorophoreProperty(const std::string& name,
                                    vtkPolyDataAlgorithm* pointSet,
                                    bool editable = false,
                                    bool optimizable = true);
  virtual ~GeometryVerticesFluorophoreProperty();

  int GetNumberOfFluorophores();

 protected:
  GeometryVerticesFluorophoreProperty() {};
 
  vtkSmartPointer<vtkPolyDataAlgorithm> m_GeometrySource;

};


#endif // _GEOMETRY_VERTICES_FLUOROPHORE_PROPERTY_H_
