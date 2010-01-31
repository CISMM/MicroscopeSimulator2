#include <PSFEditorDialog.h>

#include <QFileDialog.h>
#include <QMessageBox.h>
#include <QPSFListModel.h>
#include <QPointSpreadFunctionPropertyTableModel.h>
#include <vtkAlgorithmOutput.h>
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


void
PSFEditorDialog
::UpdatePSFVisualization() {
  m_Renderer->RemoveAllViewProps();
  m_ImagePlaneVisualization->AddToRenderer(m_Renderer);
  m_Renderer->ResetCamera();  
  m_RenderWindow->Render();
}
