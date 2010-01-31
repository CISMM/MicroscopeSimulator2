#include <ImportedPointSpreadFunction.h>

#include <vtkAlgorithmOutput.h>


ImportedPointSpreadFunction
::ImportedPointSpreadFunction() : PointSpreadFunction() {
}


ImportedPointSpreadFunction
::~ImportedPointSpreadFunction() {

}


void
ImportedPointSpreadFunction
::SetFileName(const std::string& fileName) {
  m_FileName = fileName;
}


std::string
ImportedPointSpreadFunction
::GetFileName() {
  return m_FileName;
}


void
ImportedPointSpreadFunction
::SetSpacing(double x, double y, double z) {

}


void
ImportedPointSpreadFunction
::GetSpacing(double& x, double& y, double& z) {

}


void
ImportedPointSpreadFunction
::SetOrigin(double x, double y, double z) {

}


void
ImportedPointSpreadFunction
::GetOrigin(double& x, double& y, double& z) {

}


void
ImportedPointSpreadFunction
::GetSize(int& ix, int& iy, int& iz) {

}


vtkImageData*
ImportedPointSpreadFunction
::GetOutput() {
  return NULL;
}


vtkAlgorithmOutput*
ImportedPointSpreadFunction
::GetOutputPort() {
  return NULL;
}


int
ImportedPointSpreadFunction
::GetNumberOfProperties() {
  return 0;
}


std::string
ImportedPointSpreadFunction
::GetParameterName(int index) {
  return std::string("Param");
}


double
ImportedPointSpreadFunction
::GetParameterValue(int index) {
  return 0.0;
}


void
ImportedPointSpreadFunction
::SetParameterValue(int index, double value) {

}


void
ImportedPointSpreadFunction
::GetXMLConfiguration(xmlNodePtr node) {

}


void
ImportedPointSpreadFunction
::RestoreFromXML(xmlNodePtr node) {


}
