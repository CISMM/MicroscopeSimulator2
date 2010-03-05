#include "PreferencesDialog.h"

#include <QFileDialog>
#include <QSettings>

#include <Preferences.h>


PreferencesDialog
::PreferencesDialog(QWidget* parent, Preferences* preferences) 
  : QDialog(parent), m_Preferences(preferences) {
  setupUi(this);

}


PreferencesDialog
::~PreferencesDialog() {

}


int
PreferencesDialog
::exec() {
  gui_DirectoryPathEdit->setText(QString(m_Preferences->GetDataDirectoryPath().c_str()));
  gui_CopyImportedFilesToDataDirectoryCheckBox->
    setCheckState(m_Preferences->GetCopyImportedFiles() ? Qt::Checked : Qt::Unchecked);

  int result = QDialog::exec();
  if (result == QDialog::Accepted) {
    // Save preferences
    m_Preferences->SetDataDirectoryPath(gui_DirectoryPathEdit->text().toStdString());
    m_Preferences->SetCopyImportedFiles(gui_CopyImportedFilesToDataDirectoryCheckBox->checkState() == Qt::Checked);
  }

  return result;
}


void
PreferencesDialog
::on_gui_DirectoryPathButton_clicked() {
  // Bring up dialog box to choose directory.
  QString dir = QFileDialog::getExistingDirectory(this, tr("Select data directory"), tr(""),
                                                  QFileDialog::ShowDirsOnly
                                                  | QFileDialog::DontResolveSymlinks);
  if (dir.length() > 0) {
    gui_DirectoryPathEdit->setText(dir);
  }
}
