#include <PSFEditorDialog.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

#include <QPSFListModel.h>
#include <QPointSpreadFunctionPropertyTableModel.h>

#include <vtkAlgorithmOutput.h>
#include <vtkCamera.h>
#include <vtkImageData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include <ImagePlaneVisualizationPipeline.h>
#include <OutlineVisualizationPipeline.h>
#include <PointSpreadFunction.h>
#include <PointSpreadFunctionList.h>


PSFEditorDialog
::PSFEditorDialog(QWidget* parent) : QDialog(parent) {
  setupUi(this);

  SetWidgetsEnabled(false);

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

  m_OutlineVisualization = new OutlineVisualizationPipeline();

  m_Renderer = vtkSmartPointer<vtkRenderer>::New();
  m_Renderer->SetBackground(0.3, 0.3, 0.3);

  m_RenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  m_RenderWindow->AddRenderer(m_Renderer);

  m_RenderWindow->SetInteractor(gui_PSFDisplayQvtkWidget->GetInteractor());
  gui_PSFDisplayQvtkWidget->SetRenderWindow(m_RenderWindow);

  // TODO - remove temporarily disabled PSF fitting widgets
  gui_FittingGroupBox->setVisible(false);

  m_FirstRender = true;
}


PSFEditorDialog
::~PSFEditorDialog() {
  delete m_XImagePlaneVisualization;
  delete m_YImagePlaneVisualization;
  delete m_ZImagePlaneVisualization;
  delete m_OutlineVisualization;
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
::SaveGUISettings() {
  QSettings settings;

  settings.beginGroup("PSFEditorDialog");
  settings.setValue("Geometry", saveGeometry());
  settings.endGroup();
}


void
PSFEditorDialog
::LoadGUISettings() {
  QSettings settings;

  settings.beginGroup("PSFEditorDialog");
  restoreGeometry(settings.value("Geometry").toByteArray());
  settings.endGroup();
}


void
PSFEditorDialog
::on_gui_AddCalculatedGaussianPSFButton_clicked() {
  m_PSFListModel->GetPSFList()->AddGaussianPointSpreadFunction("Gaussian");
  m_PSFListModel->Refresh();
}


void
PSFEditorDialog
::on_gui_AddCalculatedGibsonLanniWidefieldPSFButton_clicked() {
  m_PSFListModel->GetPSFList()->AddGibsonLanniWidefieldPointSpreadFunction("Gibson-Lanni Widefield");
  m_PSFListModel->Refresh();
}


void
PSFEditorDialog
::on_gui_AddModifiedGibsonLanniWidefieldPSFButton_clicked() {
  m_PSFListModel->GetPSFList()->AddModifiedGibsonLanniWidefieldPointSpreadFunction("Modified Gibson-Lanni Widefield");
  m_PSFListModel->Refresh();
}


void
PSFEditorDialog
::on_gui_AddCalculatedHaeberleWidefieldPSFButton_clicked() {
  m_PSFListModel->GetPSFList()->AddHaeberlieWidefieldPointSpreadFunction("Haeberle Widefield");
  m_PSFListModel->Refresh();
}


void
PSFEditorDialog
::on_gui_ImportPSFButton_clicked() {
  QString fileName = QFileDialog::
    getOpenFileName(this, tr("Import PSF Image"), QString(),
                    tr("TIF Files (*.tif *.tiff);;LSM Files (*.lsm)"));
  if (fileName == "")
    return;

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
::on_gui_ApplyButton_clicked() {
  m_PSFTableModel->CopyCacheToPSF();
  UpdateImage();
  UpdateSliders();
  m_RenderWindow->Render();
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
  int slice = text.toInt();
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
  int slice = text.toInt();
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
  int slice = text.toInt();
  UpdatePlane(slice, m_ZImagePlaneVisualization, gui_ZPlaneSlider);
}


void
PSFEditorDialog
::on_gui_ZPlaneSlider_valueChanged(int value) {
  m_ZImagePlaneVisualization->SetSliceNumber(value);
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
    SetWidgetsEnabled(activePSF != NULL);
  }
}


void
PSFEditorDialog
::handle_PSFListModel_dataChanged(const QModelIndex&, const QModelIndex&) {
  UpdateImage();
  UpdateSliders();
  m_RenderWindow->Render();
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
  if (psf && psf->GetOutput()) {
    double* range = psf->GetOutput()->GetScalarRange();
    double minImageValue = range[0];
    double maxImageValue = range[1];

    return (normed * (maxImageValue - minImageValue)) + minImageValue;
  } else {
    return 0.0;
  }
}


void
PSFEditorDialog
::UpdatePlane(int slice, ImagePlaneVisualizationPipeline* vis, QSlider* slider) {
  if (slice < 0)
    slice = 0;
  if (slice >= slider->maximum() && slider->maximum() > slider->minimum())
    slice = slider->maximum();
  vis->SetSliceNumber(slice);

  if (slider->value() != slice) {
    slider->setValue(slice);
  }

  m_RenderWindow->Render();
}


void
PSFEditorDialog
::UpdateImage() {
  PointSpreadFunction* activePSF = m_PSFTableModel->GetPointSpreadFunction();
  activePSF->Update();
  activePSF->GetOutputPort()->GetProducer()->Update();
  activePSF->GetOutputPort()->GetProducer()->UpdateWholeExtent();
  activePSF->GetOutput()->Update();

  m_XImagePlaneVisualization->SetInputConnection(activePSF->GetOutputPort());
  m_XImagePlaneVisualization->SetToXPlane();
  m_XImagePlaneVisualization->Update();

  m_YImagePlaneVisualization->SetInputConnection(activePSF->GetOutputPort());
  m_YImagePlaneVisualization->SetToYPlane();
  m_YImagePlaneVisualization->Update();

  m_ZImagePlaneVisualization->SetInputConnection(activePSF->GetOutputPort());
  m_ZImagePlaneVisualization->SetToZPlane();
  m_ZImagePlaneVisualization->Update();

  m_OutlineVisualization->SetInputConnection(activePSF->GetOutputPort());
}


void
PSFEditorDialog
::UpdateSliders() {
  // Get bounds of image
  PointSpreadFunction* activePSF = m_PSFTableModel->GetPointSpreadFunction();
  activePSF->GetOutput()->Update();
  int *bounds = activePSF->GetOutput()->GetDimensions();

  m_XImagePlaneVisualization->SetSliceNumber(gui_XPlaneSlider->value());
  gui_XPlaneSlider->setMinimum(0);
  gui_XPlaneSlider->setMaximum(bounds[0] - 1);

  m_YImagePlaneVisualization->SetSliceNumber(gui_YPlaneSlider->value());
  gui_YPlaneSlider->setMinimum(0);
  gui_YPlaneSlider->setMaximum(bounds[1] - 1);

  m_ZImagePlaneVisualization->SetSliceNumber(gui_ZPlaneSlider->value());
  gui_ZPlaneSlider->setMinimum(0);
  gui_ZPlaneSlider->setMaximum(bounds[2] - 1);
}


void
PSFEditorDialog
::UpdatePSFVisualization() {
  m_Renderer->RemoveAllViewProps();
  m_XImagePlaneVisualization->AddToRenderer(m_Renderer);
  m_YImagePlaneVisualization->AddToRenderer(m_Renderer);
  m_ZImagePlaneVisualization->AddToRenderer(m_Renderer);
  m_OutlineVisualization->AddToRenderer(m_Renderer);

  if (m_FirstRender) {
    m_Renderer->ResetCamera();
    m_FirstRender = false;
  }
  m_Renderer->ResetCameraClippingRange();
  m_RenderWindow->Render();
}


void
PSFEditorDialog
::SetWidgetsEnabled(bool enabled) {
  gui_ImagePlanesGroupBox->setEnabled(enabled);
  gui_ContrastGroupBox->setEnabled(enabled);
  gui_CameraGroupBox->setEnabled(enabled);
  gui_DeletePSFButton->setEnabled(enabled);
  gui_ApplyButton->setEnabled(enabled);
}


void
PSFEditorDialog
::RescaleToFullDynamicRange() {
  QModelIndex index = m_PSFSelectionModel->currentIndex();

  PointSpreadFunction* psf = m_PSFTableModel->GetPointSpreadFunction();
  if (!psf)
    return;

  psf->GetOutput()->Update();

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

