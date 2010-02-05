#include <PSFEditorDialog.h>

#include <QFileDialog.h>
#include <QMessageBox.h>
#include <QPSFListModel.h>
#include <QPointSpreadFunctionPropertyTableModel.h>
#include <vtkAlgorithmOutput.h>
#include <vtkCamera.h>
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
  
  m_XImagePlaneVisualization = new ImagePlaneVisualizationPipeline();
  m_XImagePlaneVisualization->SetAutoScalingOff();
  m_XImagePlaneVisualization->SetMapsToBlack(0.0);
  m_XImagePlaneVisualization->SetMapsToWhite(1.0);

  m_YImagePlaneVisualization = new ImagePlaneVisualizationPipeline();
  m_YImagePlaneVisualization->SetAutoScalingOff();
  m_YImagePlaneVisualization->SetMapsToBlack(0.0);
  m_YImagePlaneVisualization->SetMapsToWhite(1.0);

  m_ZImagePlaneVisualization = new ImagePlaneVisualizationPipeline();
  m_ZImagePlaneVisualization->SetAutoScalingOff();
  m_ZImagePlaneVisualization->SetMapsToBlack(0.0);
  m_ZImagePlaneVisualization->SetMapsToWhite(1.0);

  m_Renderer = vtkRenderer::New();
  m_RenderWindow = vtkRenderWindow::New();
  m_RenderWindow->AddRenderer(m_Renderer);

  gui_PSFDisplayQvtkWidget->SetRenderWindow(m_RenderWindow);
}


PSFEditorDialog
::~PSFEditorDialog() {
  delete m_XImagePlaneVisualization;
  delete m_YImagePlaneVisualization;
  delete m_ZImagePlaneVisualization;
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
::on_gui_ShowXPlaneCheckBox_toggled(bool value) {
  m_XImagePlaneVisualization->SetVisible(value);
  m_RenderWindow->Render();
}


void
PSFEditorDialog
::on_gui_XPlaneEdit_textChanged(QString text) {
  int slice = text.toInt() - 1;
  UpdatePlane(slice, m_XImagePlaneVisualization, gui_XPlaneSlider);
}


void
PSFEditorDialog
::on_gui_XPlaneSlider_valueChanged(int value) {
  gui_XPlaneEdit->setText(QString().sprintf("%d", value));
}


void
PSFEditorDialog
::on_gui_ShowYPlaneCheckBox_toggled(bool value) {
  m_YImagePlaneVisualization->SetVisible(value);
  m_RenderWindow->Render();
}

void
PSFEditorDialog
::on_gui_YPlaneEdit_textChanged(QString text) {
  int slice = text.toInt() - 1;
  UpdatePlane(slice, m_YImagePlaneVisualization, gui_YPlaneSlider);
}


void
PSFEditorDialog
::on_gui_YPlaneSlider_valueChanged(int value) {
  gui_YPlaneEdit->setText(QString().sprintf("%d", value));
}


void
PSFEditorDialog
::on_gui_ShowZPlaneCheckBox_toggled(bool value) {
  m_ZImagePlaneVisualization->SetVisible(value);
  m_RenderWindow->Render();
}


void
PSFEditorDialog
::on_gui_ZPlaneEdit_textChanged(QString text) {
  int slice = text.toInt() - 1;
  UpdatePlane(slice, m_ZImagePlaneVisualization, gui_ZPlaneSlider);
}


void
PSFEditorDialog
::on_gui_ZPlaneSlider_valueChanged(int value) {
  m_ZImagePlaneVisualization->SetSliceNumber(value-1);
  gui_ZPlaneEdit->setText(QString().sprintf("%d", value));
  m_RenderWindow->Render();
}


void
PSFEditorDialog
::on_gui_MinLevelEdit_textChanged(QString value) {
  double level = gui_MinLevelEdit->text().toDouble();
  m_XImagePlaneVisualization->SetMapsToBlack(level);
  m_YImagePlaneVisualization->SetMapsToBlack(level);
  m_ZImagePlaneVisualization->SetMapsToBlack(level);

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
  m_XImagePlaneVisualization->SetMapsToWhite(level);
  m_YImagePlaneVisualization->SetMapsToWhite(level);
  m_ZImagePlaneVisualization->SetMapsToWhite(level);

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
::on_gui_ResetButton_clicked() {
  RescaleToFullDynamicRange();
  
  m_RenderWindow->Render();
}


void
PSFEditorDialog
::on_gui_XPlusButton_clicked() {
  SetViewToXPlus();
  m_RenderWindow->Render();
}


void
PSFEditorDialog
::on_gui_XMinusButton_clicked() {
  SetViewToXMinus();
  m_RenderWindow->Render();
}


void
PSFEditorDialog
::on_gui_YPlusButton_clicked() {
  SetViewToYPlus();
  m_RenderWindow->Render();
}


void
PSFEditorDialog
::on_gui_YMinusButton_clicked() {
  SetViewToYMinus();
  m_RenderWindow->Render();
}


void
PSFEditorDialog
::on_gui_ZPlusButton_clicked() {
  SetViewToZPlus();
  m_RenderWindow->Render();
}


void
PSFEditorDialog
::on_gui_ZMinusButton_clicked() {
  SetViewToZMinus();
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

    UpdateImage();
    UpdateSliders();
    UpdatePSFVisualization();
  }
}


void
PSFEditorDialog
::handle_PSFListModel_dataChanged(const QModelIndex&, const QModelIndex&) {
  UpdateImage();
  UpdateSliders();
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
::UpdatePlane(int slice, ImagePlaneVisualizationPipeline* vis, QSlider* slider) {
  if (slice < 0)
    slice = 0;
  if (slice >= slider->maximum())
    slice = slider->maximum() - 1;
  vis->SetSliceNumber(slice);

  if (slider->value() != slice + 1) {
    slider->setValue(slice+1);
  }

  m_RenderWindow->Render();
}


void
PSFEditorDialog
::UpdateImage() {
  PointSpreadFunction* psf = m_PSFTableModel->GetPointSpreadFunction();
  psf->GetOutputPort()->GetProducer()->Update();
  psf->GetOutputPort()->GetProducer()->UpdateWholeExtent();
  psf->GetOutput()->Update();

  m_XImagePlaneVisualization->Update();
  m_YImagePlaneVisualization->Update();
  m_ZImagePlaneVisualization->Update();
}


void
PSFEditorDialog
::UpdateSliders() {
  // Get bounds of image
  PointSpreadFunction* activePSF = m_PSFTableModel->GetPointSpreadFunction();
  activePSF->GetOutput()->Update();
  int *bounds = activePSF->GetOutput()->GetDimensions();

  m_XImagePlaneVisualization->SetInputConnection(activePSF->GetOutputPort());
  m_XImagePlaneVisualization->SetToXPlane();
  m_XImagePlaneVisualization->SetSliceNumber(gui_XPlaneSlider->value());
  gui_XPlaneSlider->setMinimum(1);
  gui_XPlaneSlider->setMaximum(bounds[0]);

  m_YImagePlaneVisualization->SetInputConnection(activePSF->GetOutputPort());
  m_YImagePlaneVisualization->SetToYPlane();
  m_YImagePlaneVisualization->SetSliceNumber(gui_YPlaneSlider->value());
  gui_YPlaneSlider->setMinimum(1);
  gui_YPlaneSlider->setMaximum(bounds[1]);

  m_ZImagePlaneVisualization->SetInputConnection(activePSF->GetOutputPort());
  m_ZImagePlaneVisualization->SetToZPlane();
  m_ZImagePlaneVisualization->SetSliceNumber(gui_ZPlaneSlider->value());
  gui_ZPlaneSlider->setMinimum(1);
  gui_ZPlaneSlider->setMaximum(bounds[2]);
}


void
PSFEditorDialog
::UpdatePSFVisualization() {
  m_Renderer->RemoveAllViewProps();
  m_XImagePlaneVisualization->AddToRenderer(m_Renderer);
  m_YImagePlaneVisualization->AddToRenderer(m_Renderer);
  m_ZImagePlaneVisualization->AddToRenderer(m_Renderer);
  m_Renderer->ResetCamera();  
  m_RenderWindow->Render();
}


void
PSFEditorDialog
::RescaleToFullDynamicRange() {
  QModelIndex index = m_PSFSelectionModel->currentIndex();

  PointSpreadFunction* psf = m_PSFTableModel->GetPointSpreadFunction();
  if (!psf)
    return;

  psf->GetOutput()->Update();
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


void
PSFEditorDialog
::ResetView() {
  vtkCamera* camera = m_Renderer->GetActiveCamera();
  camera->SetFocalPoint(0, 0, 0);
  camera->SetPosition(0, 0, 1);
  camera->SetViewUp(0, 1, 0);
  m_Renderer->ResetCamera();
}


void 
PSFEditorDialog
::SetViewToXPlus() {
  ResetView();
  vtkCamera* camera = m_Renderer->GetActiveCamera();
  camera->Azimuth(-90.0);
}


void
PSFEditorDialog
::SetViewToXMinus() {
  ResetView();
  vtkCamera* camera = m_Renderer->GetActiveCamera();
  camera->Azimuth(90.0);
}


void
PSFEditorDialog
::SetViewToYPlus() {
  ResetView();
  vtkCamera* camera = m_Renderer->GetActiveCamera();
  camera->Elevation(-90.0);
}


void
PSFEditorDialog
::SetViewToYMinus() {
  ResetView();
  vtkCamera* camera = m_Renderer->GetActiveCamera();
  camera->Elevation(90.0);
}


void
PSFEditorDialog
::SetViewToZPlus() {
  ResetView();
  vtkCamera* camera = m_Renderer->GetActiveCamera();
  camera->Azimuth(180.0);
}


void
PSFEditorDialog
::SetViewToZMinus() {
  ResetView();
  // No rotation needed.
}

