#ifndef _GEOMETRY_VERTICES_FLUOROPHORE_PROPERTY_H_
#define _GEOMETRY_VERTICES_FLUOROPHORE_PROPERTY_H_

#include <FluorophoreModelObjectProperty.h>

class GeometryVerticesFluorophoreProperty : public FluorophoreModelObjectProperty {

 public:
  GeometryVerticesFluorophoreProperty(const std::string& name,
                                    vtkPolyDataAlgorithm* geometry,
                                    bool editable = false,
                                    bool optimizable = true);
  virtual ~GeometryVerticesFluorophoreProperty();

 protected:
  GeometryVerticesFluorophoreProperty() {};

};


#endif // _GEOMETRY_VERTICES_FLUOROPHORE_PROPERTY_H_
