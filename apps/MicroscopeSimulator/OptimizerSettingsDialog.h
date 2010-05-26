#ifndef _OPTIMIZER_SETTINGS_DIALOG_H_
#define _OPTIMIZER_SETTINGS_DIALOG_H_

#include <ui_OptimizerSettingsDialog.h>
#include <QDialog>


class FluorescenceOptimizer;
class ModelObjectList;

class QFluorescenceOptimizerSettingsTableModel;


class OptimizerSettingsDialog : public QDialog, 
  private Ui_OptimizerSettingsDialog {
Q_OBJECT

 public:
  OptimizerSettingsDialog(QWidget* parent=0);
  virtual ~OptimizerSettingsDialog();

  void SetFluorescenceOptimizer(FluorescenceOptimizer* optimizer);

  void Update();

  void SaveGUISettings();
  void LoadGUISettings();

 public slots:
  
 protected:
  QFluorescenceOptimizerSettingsTableModel* m_FluorescenceOptimizerTableModel;

};

#endif // _OPTIMIZER_SETTINGS_H_
