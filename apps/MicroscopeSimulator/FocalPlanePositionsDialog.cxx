#include <FocalPlanePositionsDialog.h>


FocalPlanePositionsDialog
::FocalPlanePositionsDialog(QWidget* parent)
  : QDialog(parent) {

  setupUi(this);

  m_Simulation = NULL;
}


FocalPlanePositionsDialog
::~FocalPlanePositionsDialog() {

}

