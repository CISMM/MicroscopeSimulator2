#include <OptimizerSettingsDialog.h>

#include <ModelObjectList.h>

#include <QFluorescenceOptimizerSettingsTableModel.h>
#include <QSettings>
#include <QTreeWidgetItem>


OptimizerSettingsDialog
::OptimizerSettingsDialog(QWidget* parent) : QDialog(parent) {
  setupUi(this);

  m_FluorescenceOptimizerTableModel = 
    new QFluorescenceOptimizerSettingsTableModel(this);
  settingsTableView->setModel(m_FluorescenceOptimizerTableModel);
}


OptimizerSettingsDialog
::~OptimizerSettingsDialog() {
  delete m_FluorescenceOptimizerTableModel;
}


void
OptimizerSettingsDialog
::SetFluorescenceOptimizer(FluorescenceOptimizer* optimizer) {
  m_FluorescenceOptimizerTableModel->SetFluorescenceOptimizer(optimizer);
  m_FluorescenceOptimizerTableModel->Refresh();
}


void
OptimizerSettingsDialog
::Update() {
}


void
OptimizerSettingsDialog
::SaveGUISettings() {
  QSettings settings;

  settings.beginGroup("OptimizerSettingsDialog");
  settings.setValue("Geometry", saveGeometry());
  settings.endGroup();

  settings.beginGroup("OptimizerSettingsDialog");
  for (int i = 0; i < m_FluorescenceOptimizerTableModel->columnCount(); i++) {
    QString colWidthName;
    colWidthName.sprintf("ColumnWidth%d", i);
    settings.setValue(colWidthName, settingsTableView->columnWidth(i));
  }
  settings.endGroup();
}


void
OptimizerSettingsDialog
::LoadGUISettings() {
  QSettings settings;

  settings.beginGroup("OptimizerSettingsDialog");
  QVariant geometryVariant = settings.value("Geometry");
  if (geometryVariant.isValid()) {
    restoreGeometry(geometryVariant.toByteArray());
  }
  settings.endGroup();

  settings.beginGroup("OptimizerSettingsDialog");
  for (int i = 0; i < m_FluorescenceOptimizerTableModel->columnCount(); i++) {
    QString colWidthName;
    colWidthName.sprintf("ColumnWidth%d", i);
    int width = settings.value(colWidthName).toInt();
    if (width > 0)
      settingsTableView->setColumnWidth(i, width);
  }
  settings.endGroup();
}
