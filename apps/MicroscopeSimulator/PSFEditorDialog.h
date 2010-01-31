#ifndef _PSF_EDITOR_DIALOG_H_
#define _PSF_EDITOR_DIALOG_H_

#include <ui_PSFEditorDialog.h>
#include <QDialog.h>

class ImagePlaneVisualizationPipeline;
class QWidget;
class QPSFListModel;
class QPointSpreadFunctionPropertyTableModel;
class PointSpreadFunctionList;

class vtkRenderer;


class PSFEditorDialog : public QDialog,
  private Ui_PSFEditorDialog {
Q_OBJECT

 public:
  PSFEditorDialog(QWidget* parent=0);
  virtual ~PSFEditorDialog();

  void SetPSFList(PointSpreadFunctionList* list);
  PointSpreadFunctionList* GetPSFList();

  void SetWindowSplitterSizes(QList<QVariant> sizes);
  QList<QVariant> GetWindowSplitterSizes();

  void SetControlPanelSplitterSizes(QList<QVariant> sizes);
  QList<QVariant> GetControlPanelSplitterSizes();

 public slots:
  virtual void on_gui_AddCalculatedGaussianPSFButton_clicked();
  virtual void on_gui_AddCalculatedWidefieldPSFButton_clicked();
  virtual void on_gui_ImportPSFButton_clicked();
  virtual void on_gui_DeletePSFButton_clicked();

  virtual void handle_PSFListModel_selectionChanged(const QItemSelection&, const QItemSelection&);
  virtual void handle_PSFListModel_dataChanged(const QModelIndex&, const QModelIndex&);

 protected:
  QPSFListModel* m_PSFListModel;

  QItemSelectionModel* m_PSFSelectionModel;

  QPointSpreadFunctionPropertyTableModel* m_PSFTableModel;

  vtkRenderer* m_Renderer;
  vtkRenderWindow* m_RenderWindow;

  ImagePlaneVisualizationPipeline* m_ImagePlaneVisualization;

  void UpdatePSFVisualization();

};


#endif // _PSF_EDITOR_DIALOG_H_
