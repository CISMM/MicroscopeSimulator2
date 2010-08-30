#include <GeometryVerticesFluorophoreProperty.h>

#include <vtkPolyData.h>
#include <vtkPolyDataAlgorithm.h>


GeometryVerticesFluorophoreProperty
::GeometryVerticesFluorophoreProperty(const std::string& name,
                                      vtkPolyDataAlgorithm* geometrySource,
                                      bool editable, bool optimizable) 
  : FluorophoreModelObjectProperty(name, editable, optimizable) {
  
  m_GeometrySource    = geometrySource;
  m_FluorophoreOutput = geometrySource;
}


GeometryVerticesFluorophoreProperty
::~GeometryVerticesFluorophoreProperty() {

}


int
GeometryVerticesFluorophoreProperty
::GetNumberOfFluorophores() {
  m_GeometrySource->GetOutput()->Update();
  return m_GeometrySource->GetOutput()->GetNumberOfPoints();
}
