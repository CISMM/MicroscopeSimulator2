#include <PointSpreadFunction.h>

const std::string PointSpreadFunction::NAME_ATTRIBUTE  = "Name";
const std::string PointSpreadFunction::SIZE_ELEMENT    = "Size";
const std::string PointSpreadFunction::SPACING_ELEMENT = "Spacing";
const std::string PointSpreadFunction::X_ATTRIBUTE     = "X";
const std::string PointSpreadFunction::Y_ATTRIBUTE     = "Y";
const std::string PointSpreadFunction::Z_ATTRIBUTE     = "Z";


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
