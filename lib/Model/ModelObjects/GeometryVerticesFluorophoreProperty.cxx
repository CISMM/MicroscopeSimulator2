#include <GeometryVerticesFluorophoreProperty.h>

#include <vtkPolyDataAlgorithm.h>


GeometryVerticesFluorophoreProperty
::GeometryVerticesFluorophoreProperty(const std::string& name,
                                      vtkPolyDataAlgorithm* geometry,
                                      bool editable, bool optimizable) 
  : FluorophoreModelObjectProperty(name, geometry, editable, optimizable) {

  m_FluorophoreOutput = m_GeometrySource;
}


GeometryVerticesFluorophoreProperty
::~GeometryVerticesFluorophoreProperty() {

}
