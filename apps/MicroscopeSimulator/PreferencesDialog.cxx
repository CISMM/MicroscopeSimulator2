#include "PreferencesDialog.h"

#include <QFileDialog>
#include <QSettings>


PreferencesDialog
::PreferencesDialog(QWidget* parent) : QDialog(parent) {
  setupUi(this);

}


PreferencesDialog
::~PreferencesDialog() {

}


int
PreferencesDialog
::exec() {
  QSettings settings;
  settings.beginGroup("DataDirectory");
  if (settings.contains(tr("path"))) {
    gui_DirectoryPathEdit->setText(settings.value("path", tr("")).toString());
    gui_CopyImportedFilesToDataDirectoryCheckBox->
      setCheckState(settings.value("copyImportedFiles", true).toBool() ?
                    Qt::Checked : Qt::Unchecked);
  }
  settings.endGroup();

  int result = QDialog::exec();
  if (result == QDialog::Accepted) {
    // Save preferences
    QSettings settings;

    settings.beginGroup("DataDirectory");
    settings.setValue("path", gui_DirectoryPathEdit->text());
    settings.setValue("copyImportedFiles", gui_CopyImportedFilesToDataDirectoryCheckBox->isChecked());
    settings.endGroup();
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
  gui_DirectoryPathEdit->setText(dir);
}
