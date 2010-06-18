#include <FocalPlanePositionsDialog.h>

#include <QFocalPlanePositionsTableModel.h>

#include <FluorescenceSimulation.h>


FocalPlanePositionsDialog
::FocalPlanePositionsDialog(QWidget* parent) : QDialog(parent) {
  setupUi(this);

  m_Simulation = NULL;

  m_FocalPlanePositionsModel = new QFocalPlanePositionsTableModel();
  focalPlanePositionsTableView->setModel(m_FocalPlanePositionsModel);
  m_FocalPlanePositionsModel->Refresh();
}


FocalPlanePositionsDialog
::~FocalPlanePositionsDialog() {
  delete m_FocalPlanePositionsModel;
}


void
FocalPlanePositionsDialog
::SetFluorescenceSimulation(FluorescenceSimulation* simulation) {
  m_Simulation = simulation;
  m_FocalPlanePositionsModel->SetFluorescenceSimulation(simulation);
}


FluorescenceSimulation*
FocalPlanePositionsDialog
::GetFluorescenceSimulation() {
  return m_Simulation;
}


void
FocalPlanePositionsDialog
::Refresh() {
  m_FocalPlanePositionsModel->Refresh();
}


void
FocalPlanePositionsDialog
::on_focalPlanePositionsResetButton_clicked() {
  for (unsigned int i = 0; i < m_Simulation->GetNumberOfFocalPlanes(); i++) {
    double position = static_cast<double>(i) * m_Simulation->GetFocalPlaneSpacing();
    QModelIndex index = m_FocalPlanePositionsModel->index(i,1);
    m_FocalPlanePositionsModel->setData(index, QVariant(position), Qt::EditRole);
  }
  m_FocalPlanePositionsModel->Refresh();
}
