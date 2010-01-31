#include <AppendGeometrySource.h>

#include <vtkAppendPolyData.h>


AppendGeometrySource
::AppendGeometrySource(ModelObjectPtr modelObject) :
  GeometrySource(modelObject) {
  m_Appender = vtkSmartPointer<vtkAppendPolyData>::New();
}


AppendGeometrySource
::~AppendGeometrySource() {

}


void
AppendGeometrySource
::AddGeometrySource(GeometrySource* source) {
  m_Sources.push_back(source);

  m_Appender->AddInputConnection(source->GetPolyDataOutputPort());
}


vtkAlgorithmOutput*
AppendGeometrySource
::GetPolyDataOutputPort() {
  return m_Appender->GetOutputPort();
}


void
AppendGeometrySource
::Update() {
  std::list<GeometrySource*>::iterator iter;
  for (iter = m_Sources.begin(); iter != m_Sources.end(); iter++) {
    (*iter)->Update();
  }
}
