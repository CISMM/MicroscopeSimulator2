#ifndef _PSF_EDITOR_DIALOG_H_
#define _PSF_EDITOR_DIALOG_H_

#include <ui_PSFEditorDialog.h>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>

class ImagePlaneVisualizationPipeline;
class OutlineVisualizationPipeline;
class QWidget;
class QPSFListModel;
class QPointSpreadFunctionPropertyTableModel;
class PointSpreadFunctionList;

#include <vtkSmartPointer.h>
class vtkRenderer;


class PSFEditorDialog : public QDialog,
  private Ui_PSFEditorDialog {
Q_OBJECT

 public:
  PSFEditorDialog(QWidget* parent=0);
  virtual ~PSFEditorDialog();

  void SetPSFList(PointSpreadFunctionList* list);
  PointSpreadFunctionList* GetPSFList();

  void SaveGUISettings();
  void LoadGUISettings();

 public slots:
  virtual void on_gui_AddCalculatedGaussianPSFButton_clicked();
  virtual void on_gui_AddCalculatedWidefieldPSFButton_clicked();
  virtual void on_gui_ImportPSFButton_clicked();
  virtual void on_gui_DeletePSFButton_clicked();
  virtual void on_gui_ApplyButton_clicked();

  virtual void on_gui_ShowXPlaneCheckBox_toggled(bool value);
  virtual void on_gui_XPlaneEdit_textChanged(QString text);
  virtual void on_gui_XPlaneSlider_valueChanged(int value);

  virtual void on_gui_ShowYPlaneCheckBox_toggled(bool value);
  virtual void on_gui_YPlaneEdit_textChanged(QString text);
  virtual void on_gui_YPlaneSlider_valueChanged(int value);

  virtual void on_gui_ShowZPlaneCheckBox_toggled(bool value);
  virtual void on_gui_ZPlaneEdit_textChanged(QString text);
  virtual void on_gui_ZPlaneSlider_valueChanged(int value);

  virtual void on_gui_MinLevelEdit_textChanged(QString value);
  virtual void on_gui_MinLevelSlider_valueChanged(int value);
  virtual void on_gui_MaxLevelEdit_textChanged(QString value);
  virtual void on_gui_MaxLevelSlider_valueChanged(int value);
  virtual void on_gui_ResetButton_clicked();

  virtual void on_gui_XPlusButton_clicked();
  virtual void on_gui_XMinusButton_clicked();
  virtual void on_gui_YPlusButton_clicked();
  virtual void on_gui_YMinusButton_clicked();
  virtual void on_gui_ZPlusButton_clicked();
  virtual void on_gui_ZMinusButton_clicked();
  
  virtual void handle_PSFListModel_selectionChanged(const QItemSelection&, const QItemSelection&);
  virtual void handle_PSFListModel_dataChanged(const QModelIndex&, const QModelIndex&);

 protected:
  QPSFListModel* m_PSFListModel;

  QItemSelectionModel* m_PSFSelectionModel;

  QPointSpreadFunctionPropertyTableModel* m_PSFTableModel;

  vtkSmartPointer<vtkRenderer>     m_Renderer;
  vtkSmartPointer<vtkRenderWindow> m_RenderWindow;

  bool m_FirstRender;

  ImagePlaneVisualizationPipeline* m_XImagePlaneVisualization;
  ImagePlaneVisualizationPipeline* m_YImagePlaneVisualization;
  ImagePlaneVisualizationPipeline* m_ZImagePlaneVisualization;
  OutlineVisualizationPipeline*    m_OutlineVisualization;

  int IntensityToSliderValue(double intensity, const QSlider& slider);
  double SliderValueToIntensity(int value, const QSlider& slider);

  void UpdatePlane(int slice, ImagePlaneVisualizationPipeline* vis,
                   QSlider* slider);

  void UpdateImage();
  void UpdateSliders();
  void UpdatePSFVisualization();
  void SetWidgetsEnabled(bool enabled);
  
  void RescaleToFullDynamicRange();

  void ResetView();
  void SetViewToXPlus();
  void SetViewToXMinus();
  void SetViewToYPlus();
  void SetViewToYMinus();
  void SetViewToZPlus();
  void SetViewToZMinus();

};


#endif // _PSF_EDITOR_DIALOG_H_
