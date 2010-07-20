#include "PreferencesDialog.h"

#include <QFileDialog>
#include <QSettings>

#include <MicroscopeSimulator.h>
#include <Preferences.h>


PreferencesDialog
::PreferencesDialog(QWidget* parent, MicroscopeSimulator* sim, 
                    Preferences* preferences) 
  : QDialog(parent), m_Simulator(sim), m_Preferences(preferences) {
  setupUi(this);

}


PreferencesDialog
::~PreferencesDialog() {

}


int
PreferencesDialog
::exec() {
  gui_DirectoryPathEdit->setText(QString(m_Preferences->GetDataDirectoryPath().c_str()));
  //gui_CopyImportedFilesToDataDirectoryCheckBox->
  //  setCheckState(m_Preferences->GetCopyImportedFiles() ? Qt::Checked : Qt::Unchecked);

  
  QSettings prefs;
  prefs.beginGroup("OpenGLCapabilities");
  if (prefs.value("RequiredExtensions", true).toBool()) {
    gui_RequiredExtensionsValueLabel->setText("Supported");
  } else {
    gui_RequiredExtensionsValueLabel->setText("Not supported");
  }

  if (prefs.value("16BitFloatingPointBlend", true).toBool()) {
    gui_16BitFloatingPointBlendingValueLabel->setText("Supported");
  } else {
    gui_16BitFloatingPointBlendingValueLabel->setText("Not supported");
  }

  if (prefs.value("32BitFloatingPointBlend", false).toBool()) {
    gui_32BitFloatingPointBlendingValueLabel->setText("Supported");
  } else {
    gui_32BitFloatingPointBlendingValueLabel->setText("Not supported");
  }

  if (prefs.value("FloatingPointTextureTrilinearInterpolation", false).toBool()) {
    gui_3DTrilinearInterpolationValueLabel->setText("Supported");
  } else {
    gui_3DTrilinearInterpolationValueLabel->setText("Not supported");
  }

  if (prefs.value("GLSLUnsignedInts", false).toBool()) {
    gui_GLSLUnsignedIntegersValueLabel->setText("Supported");
  } else {
    gui_GLSLUnsignedIntegersValueLabel->setText("Not supported");
  }
  prefs.endGroup();

  int result = QDialog::exec();
  if (result == QDialog::Accepted) {
    // Save preferences
    m_Preferences->SetDataDirectoryPath(gui_DirectoryPathEdit->text().toStdString());
    //m_Preferences->SetCopyImportedFiles(gui_CopyImportedFilesToDataDirectoryCheckBox->checkState() == Qt::Checked);
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


void
PreferencesDialog
::on_gui_CheckOpenGLSettingsAgainButton_clicked() {
  QSettings prefs;
  prefs.beginGroup("OpenGLCapabilities");
  prefs.setValue("Checked", false);

  m_Simulator->CheckOpenGLCapabilities();

  prefs.endGroup();
}
