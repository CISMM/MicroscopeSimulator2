#include <Preferences.h>

#include <QSettings>
#include <QString>

const std::string Preferences::DATA_DIRECTORY_GROUP = "DataDirectory";
const std::string Preferences::PATH_ENTRY = "path";
const std::string Preferences::COPY_IMPORTED_FILES_ENTRY = "copyImportedFiles";


Preferences
::Preferences() {
  m_Settings = new QSettings();
}


Preferences
::~Preferences() {
  delete m_Settings;
}


void
Preferences
::SetDataDirectoryPath(const std::string& path) {
  m_Settings->beginGroup(DATA_DIRECTORY_GROUP.c_str());
  m_Settings->setValue(PATH_ENTRY.c_str(), QString(path.c_str()));
  m_Settings->endGroup();
  m_Settings->sync();
}


std::string
Preferences
::GetDataDirectoryPath() {
  m_Settings->beginGroup(DATA_DIRECTORY_GROUP.c_str());
  std::string path = m_Settings->value(PATH_ENTRY.c_str(), QString()).toString().toStdString();
  m_Settings->endGroup();
  m_Settings->sync();

  return path;
}


void
Preferences
::SetCopyImportedFiles(bool copy) {
  m_Settings->beginGroup(DATA_DIRECTORY_GROUP.c_str());
  m_Settings->setValue(COPY_IMPORTED_FILES_ENTRY.c_str(), copy);
  m_Settings->endGroup();
  m_Settings->sync();
}


bool
Preferences
::GetCopyImportedFiles() {
  m_Settings->beginGroup(DATA_DIRECTORY_GROUP.c_str());
  bool copy = m_Settings->value(COPY_IMPORTED_FILES_ENTRY.c_str(), false).toBool();
  m_Settings->endGroup();

  return copy;
}
