#ifndef _PREFERENCES_DIALOG_H_
#define _PREFERENCES_DIALOG_H_

#include <ui_PreferencesDialog.h>
#include <QDialog>

class Preferences;


class PreferencesDialog : public QDialog,
  private Ui_PreferencesDialog {
Q_OBJECT

 public:
  PreferencesDialog(QWidget* parent=0, Preferences* preferences=0);
  virtual ~PreferencesDialog();

 public slots:
  int exec();

  void on_gui_DirectoryPathButton_clicked();

 protected:
  Preferences* m_Preferences;

};

#endif // _PREFERENCES_DIALOG_H_
