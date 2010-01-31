#include "GeometrySource.h"

GeometrySource
::GeometrySource(ModelObjectPtr modelObject) {
  m_ModelObject = modelObject;
}


GeometrySource
::~GeometrySource() {

}


ModelObjectPtr
GeometrySource
::GetModelObject() {
  return m_ModelObject;
}
