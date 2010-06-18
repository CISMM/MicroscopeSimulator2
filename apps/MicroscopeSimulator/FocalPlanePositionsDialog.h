#ifndef _FOCAL_PLANE_POSITIONS_DIALOG_H_
#define _FOCAL_PLANE_POSITIONS_DIALOG_H_

#include <ui_FocalPlanePositionsDialog.h>
#include <QDialog>


class FluorescenceSimulation;


class FocalPlanePositionsDialog : public QDialog,
  private Ui_FocalPlanePositionsDialog {
Q_OBJECT

 public:
  FocalPlanePositionsDialog(QWidget* parent=0);
  virtual ~FocalPlanePositionsDialog();

 protected:
  FluorescenceSimulation* m_Simulation;

};

#endif // _FOCAL_PLANE_POSITIONS_DIALOG_H_
