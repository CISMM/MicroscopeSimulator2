#ifndef _OPTIMIZER_SETTINGS_DIALOG_H_
#define _OPTIMIZER_SETTINGS_DIALOG_H_

#include <ui_OptimizerSettingsDialog.h>
#include <QDialog>

class ModelObjectList;


class OptimizerSettingsDialog : public QDialog, 
  private Ui_OptimizerSettingsDialog {
Q_OBJECT

 public:
  OptimizerSettingsDialog(QWidget* parent=0);
  virtual ~OptimizerSettingsDialog();

  void Update();

 public slots:
  
 protected:

};

#endif // _OPTIMIZER_SETTINGS_H_
