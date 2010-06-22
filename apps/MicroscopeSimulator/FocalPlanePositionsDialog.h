#ifndef _FOCAL_PLANE_POSITIONS_DIALOG_H_
#define _FOCAL_PLANE_POSITIONS_DIALOG_H_

#include <ui_FocalPlanePositionsDialog.h>
#include <QDialog>


class FluorescenceSimulation;
class QFocalPlanePositionsTableModel;


class FocalPlanePositionsDialog : public QDialog,
  private Ui_FocalPlanePositionsDialog {
Q_OBJECT

 public:
  FocalPlanePositionsDialog(QWidget* parent=0);
  virtual ~FocalPlanePositionsDialog();

  void SetFluorescenceSimulation(FluorescenceSimulation* simulation);
  FluorescenceSimulation* GetFluorescenceSimulation();

  void Refresh();                                                    

 public slots:
  virtual void on_focalPlanePositionsResetButton_clicked();

 protected:
  FluorescenceSimulation* m_Simulation;

  QFocalPlanePositionsTableModel* m_FocalPlanePositionsModel;
};

#endif // _FOCAL_PLANE_POSITIONS_DIALOG_H_
