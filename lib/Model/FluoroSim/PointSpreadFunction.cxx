#include <PointSpreadFunction.h>


PointSpreadFunction
::PointSpreadFunction() {
}


PointSpreadFunction
::~PointSpreadFunction() {

}


void
PointSpreadFunction
::SetName(const std::string& name) {
  m_Name = name;
}


std::string&
PointSpreadFunction
::GetName() {
  return m_Name;
}
