#include <PSFEditorDialog.h>

#include <QFileDialog.h>
#include <QMessageBox.h>
#include <QPSFListModel.h>
#include <QPointSpreadFunctionPropertyTableModel.h>
#include <vtkAlgorithmOutput.h>
#include <vtkImageData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include <ImagePlaneVisualizationPipeline.h>
#include <PointSpreadFunction.h>
#include <PointSpreadFunctionList.h>


PSFEditorDialog
::PSFEditorDialog(QWidget* parent) : QDialog(parent) {
  setupUi(this);

  m_PSFListModel = new QPSFListModel();
  gui_PSFListView->setModel(m_PSFListModel);
  m_PSFListModel->Refresh();

  m_PSFTableModel = new QPointSpreadFunctionPropertyTableModel();
  gui_PSFParametersView->setModel(m_PSFTableModel);
  connect(m_PSFTableModel,
          SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
          this,
          SLOT(handle_PSFListModel_dataChanged(const QModelIndex&, const QModelIndex&)));

  m_PSFSelectionModel = gui_PSFListView->selectionModel();
  connect(m_PSFSelectionModel,
          SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
          this,
          SLOT(handle_PSFListModel_selectionChanged(const QItemSelection&, const QItemSelection&)));
  
  m_ImagePlaneVisualization = new ImagePlaneVisualizationPipeline();
  m_ImagePlaneVisualization->SetAutoScalingOff();

  m_Renderer = vtkRenderer::New();
  m_RenderWindow = vtkRenderWindow::New();
  m_RenderWindow->AddRenderer(m_Renderer);

  gui_PSFDisplayQvtkWidget->SetRenderWindow(m_RenderWindow);
}


PSFEditorDialog
::~PSFEditorDialog() {

}


void
PSFEditorDialog
::SetPSFList(PointSpreadFunctionList* list) {
  m_PSFListModel->SetPSFList(list);
  m_PSFListModel->Refresh();
}


PointSpreadFunctionList*
PSFEditorDialog
::GetPSFList() {
  return m_PSFListModel->GetPSFList();
}


void
PSFEditorDialog
::SetWindowSplitterSizes(QList<QVariant> sizes) {
  QList<int> intSizes;
  for (QList<QVariant>::iterator iter = sizes.begin(); iter != sizes.end(); iter++) {
    intSizes.push_back((*iter).toInt());
  }
  gui_WindowSplitter->setSizes(intSizes);
}


QList<QVariant>
PSFEditorDialog
::GetWindowSplitterSizes() {
  QList<int> sizes = gui_WindowSplitter->sizes();
  QList<QVariant> variantSizes;
  for (QList<int>::iterator iter = sizes.begin(); iter != sizes.end(); iter++) {
    variantSizes.push_back(QVariant(*iter));
  }

  return variantSizes;
}


void
PSFEditorDialog
::SetControlPanelSplitterSizes(QList<QVariant> sizes) {
  QList<int> intSizes;
  for (QList<QVariant>::iterator iter = sizes.begin(); iter != sizes.end(); iter++) {
    intSizes.push_back((*iter).toInt());
  }
  gui_ControlPanelSplitter->setSizes(intSizes);
}


QList<QVariant>
PSFEditorDialog
::GetControlPanelSplitterSizes() {
  QList<int> sizes = gui_ControlPanelSplitter->sizes();
  QList<QVariant> variantSizes;
  for (QList<int>::iterator iter = sizes.begin(); iter != sizes.end(); iter++) {
    variantSizes.push_back(QVariant(*iter));
  }

  return variantSizes;
}


void
PSFEditorDialog
::on_gui_AddCalculatedGaussianPSFButton_clicked() {
  m_PSFListModel->GetPSFList()->AddGaussianPointSpreadFunction("Gaussian");
  m_PSFListModel->Refresh();
}


void
PSFEditorDialog
::on_gui_AddCalculatedWidefieldPSFButton_clicked() {
  m_PSFListModel->GetPSFList()->AddWidefieldPointSpreadFunction("Widefield");
  m_PSFListModel->Refresh();
}


void
PSFEditorDialog
::on_gui_ImportPSFButton_clicked() {
  QString fileName = QFileDialog::
    getOpenFileName(this, tr("Open PSF Image"), QString(), 
                    tr("TIF Files (*.tif *.tiff);;LSM Files (*.lsm)"));

  m_PSFListModel->GetPSFList()->ImportPointSpreadFunction(fileName.toStdString());
  m_PSFListModel->Refresh();
}


void
PSFEditorDialog
::on_gui_DeletePSFButton_clicked() {
  QItemSelectionModel* selectionList = gui_PSFListView->selectionModel();
  if (!selectionList->hasSelection())
    return;

  int selected = selectionList->currentIndex().row();

  std::string psfName = 
    m_PSFListModel->GetPSFList()->GetPointSpreadFunctionAt(selected)->GetName();

  QMessageBox::StandardButton reply;
  reply = QMessageBox::question(this, tr("Really delete point-spread function?"),
                                tr("Are you sure you want to delete the point-spread function '").
                                append(psfName.c_str()).append("'?"),
                                QMessageBox::Ok | QMessageBox::Cancel);
  if (reply == QMessageBox::Ok) {
    m_PSFTableModel->SetPointSpreadFunction(NULL);
    m_PSFListModel->GetPSFList()->DeletePointSpreadFunction(selected);
    m_PSFListModel->Refresh();
  }
}


void
PSFEditorDialog
::on_gui_MinLevelEdit_textChanged(QString value) {
  double level = gui_MinLevelEdit->text().toDouble();
  m_ImagePlaneVisualization->SetMapsToBlack(level);

  m_RenderWindow->Render();
}


void
PSFEditorDialog
::on_gui_MinLevelSlider_valueChanged(int value) {
  double dValue = SliderValueToIntensity(value, *gui_MinLevelSlider);
  gui_MinLevelEdit->setText(QString().sprintf("%0.6f", dValue));

  m_RenderWindow->Render();
}


void
PSFEditorDialog
::on_gui_MaxLevelEdit_textChanged(QString value) {
  double level = gui_MaxLevelEdit->text().toDouble();
  m_ImagePlaneVisualization->SetMapsToWhite(level);

  m_RenderWindow->Render();
}


void
PSFEditorDialog
::on_gui_MaxLevelSlider_valueChanged(int value) {
  double dValue = SliderValueToIntensity(value, *gui_MaxLevelSlider);
  gui_MaxLevelEdit->setText(QString().sprintf("%0.6f", dValue));

  m_RenderWindow->Render();
}


void
PSFEditorDialog
::on_gui_RescaleButton_clicked() {
  RescaleToFullDynamicRange();
  
  m_RenderWindow->Render();
}


void
PSFEditorDialog
::handle_PSFListModel_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
  QModelIndexList indexes = selected.indexes();
  if (!indexes.isEmpty() && indexes[0].row() >= 0) {
    int selected = indexes[0].row();
    PointSpreadFunction* activePSF = m_PSFListModel->GetPSFList()->
      GetPointSpreadFunctionAt(selected);
    m_PSFTableModel->SetPointSpreadFunction(activePSF);

    m_ImagePlaneVisualization->SetInputConnection(activePSF->GetOutputPort());

    UpdatePSFVisualization();
  }
}


void
PSFEditorDialog
::handle_PSFListModel_dataChanged(const QModelIndex&, const QModelIndex&) {
  m_PSFTableModel->GetPointSpreadFunction()->GetOutputPort()->GetProducer()->Update();

  UpdatePSFVisualization();
}


int
PSFEditorDialog
::IntensityToSliderValue(double intensity, const QSlider& slider) {
  int sliderMin = slider.minimum();
  int sliderMax = slider.maximum();

  PointSpreadFunction* psf = m_PSFTableModel->GetPointSpreadFunction();
  double* range = psf->GetOutput()->GetScalarRange();
  double minImageValue = range[0];
  double maxImageValue = range[1];

  double normed = (intensity - minImageValue) / (maxImageValue - minImageValue);
  double dSliderValue = static_cast<double>(sliderMax - sliderMin) * normed;
  int sliderValue = static_cast<int>(dSliderValue) + sliderMin;

  return sliderValue;
}


double
PSFEditorDialog
::SliderValueToIntensity(int value, const QSlider& slider) {
  int sliderMin = slider.minimum();
  int sliderMax = slider.maximum();
  double normed = static_cast<double>(value - sliderMin) /
    static_cast<double>(sliderMax - sliderMin);

  PointSpreadFunction* psf = m_PSFTableModel->GetPointSpreadFunction();
  double* range = psf->GetOutput()->GetScalarRange();
  double minImageValue = range[0];
  double maxImageValue = range[1];
  
  return (normed * (maxImageValue - minImageValue)) + minImageValue;
}


void
PSFEditorDialog
::UpdatePSFVisualization() {
  m_Renderer->RemoveAllViewProps();
  m_ImagePlaneVisualization->AddToRenderer(m_Renderer);
  m_Renderer->ResetCamera();  
  m_RenderWindow->Render();
}


void
PSFEditorDialog
::RescaleToFullDynamicRange() {
  QModelIndex index = m_PSFSelectionModel->currentIndex();

  PointSpreadFunction* psf = m_PSFTableModel->GetPointSpreadFunction();
  double* range = psf->GetOutput()->GetScalarRange();

  gui_MinLevelEdit->setText(QString().sprintf("%f", range[0]));
  gui_MaxLevelEdit->setText(QString().sprintf("%f", range[1]));

  double minTextValue = SliderValueToIntensity(gui_MinLevelSlider->value(),
                                               *gui_MinLevelSlider);
  gui_MinLevelEdit->setText(QString().sprintf("%f", minTextValue));

  double maxTextValue = SliderValueToIntensity(gui_MaxLevelSlider->value(),
                                               *gui_MaxLevelSlider);
  gui_MaxLevelEdit->setText(QString().sprintf("%f", maxTextValue));

  gui_MinLevelSlider->setValue(gui_MinLevelSlider->minimum());
  gui_MaxLevelSlider->setValue(gui_MaxLevelSlider->maximum());
}
