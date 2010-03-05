#ifndef _PREFERENCES_H_
#define _PREFERENCES_H_

#include <string>

class QSettings;

class Preferences {

 public:
  const static std::string DATA_DIRECTORY_GROUP;
  const static std::string PATH_ENTRY;
  const static std::string COPY_IMPORTED_FILES_ENTRY;

  Preferences();
  virtual ~Preferences();

  void SetDataDirectoryPath(const std::string& path);
  std::string GetDataDirectoryPath();

  void SetCopyImportedFiles(bool copy);
  bool GetCopyImportedFiles();

 protected:
  QSettings* m_Settings;

};

#endif // _PREFERENCES_H_
