#include <WidefieldPointSpreadFunction.h>


WidefieldPointSpreadFunction
::WidefieldPointSpreadFunction() {

}


WidefieldPointSpreadFunction
::~WidefieldPointSpreadFunction() {

}


vtkImageData*
WidefieldPointSpreadFunction
::GetOutput() {
  return NULL;
}


vtkAlgorithmOutput*
WidefieldPointSpreadFunction
::GetOutputPort() {
  return NULL;
}


int
WidefieldPointSpreadFunction
::GetNumberOfProperties() {
  return 0;
}


std::string
WidefieldPointSpreadFunction
::GetParameterName(int index) {
  return std::string("Param");
}


double
WidefieldPointSpreadFunction
::GetParameterValue(int index) {
  return 0.0;
}


void
WidefieldPointSpreadFunction
::SetParameterValue(int index, double value) {
  
}


void
WidefieldPointSpreadFunction
::GetXMLConfiguration(xmlNodePtr node) {

}


void
WidefieldPointSpreadFunction
::RestoreFromXML(xmlNodePtr node) {


}
