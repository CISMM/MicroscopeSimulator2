#include <MicroscopeSimulator.h>
#include <Simulation.h>
#include <Visualization.h>
#include <Version.h>
#include <AFMSimulation.h>
#include <FluorescenceOptimizer.h>
#include <FluorescenceSimulation.h>
#include <FluorophoreModelObjectProperty.h>
#include <PointSpreadFunctionList.h>
#include <ImageWriter.h>

#if defined(_WIN32) // Turn off deprecation warnings in Visual Studio
#pragma warning( disable : 4996 )
#endif

#include <QActionGroup>
#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QImageListModel.h>
#include <QItemEditorFactory>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QPSFListModel.h>
#include <QSettings>
#include <QStandardItemEditorCreator>
#include <QVariant>

#include <ErrorLogDialog.h>
#include <FluorophoreModelDialog.h>
#include <ImageExportOptionsDialog.h>
#include <OptimizerSettingsDialog.h>
#include <PSFEditorDialog.h>
#include <Preferences.h>
#include <PreferencesDialog.h>

#include <vtkActor.h>
#include <vtkAlgorithmOutput.h>
#include <vtkBMPWriter.h>
#include <vtkBYUWriter.h>
#include <vtkCamera.h>
#include <vtkContourFilter.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageAppend.h>
#include <vtkImageShiftScale.h>
#include <vtkImageWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkOutputWindow.h>
#include <vtkPLYWriter.h>
#include <vtkPNGWriter.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataWriter.h>
#include <vtkQtOutputLogger.h>
#include <vtkRenderedSurfaceRepresentation.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkTIFFWriter.h>
#include <vtkWindowToImageFilter.h>

#include <vtkFramebufferObjectRenderer.h>
#include <vtkFramebufferObjectTexture.h>


// Constructor
MicroscopeSimulator
::MicroscopeSimulator(QWidget* p)
  : QMainWindow(p), m_ModelObjectPropertyListTableModel(NULL) {

  gui = new Ui_MainWindow();
  gui->setupUi(this);

  // Change the double item editor to QLineEdit
  QItemEditorFactory* factory = new QItemEditorFactory();
  factory->registerEditor(QVariant::Int,    new QStandardItemEditorCreator<QLineEdit>());
  factory->registerEditor(QVariant::Double, new QStandardItemEditorCreator<QLineEdit>());
  factory->registerEditor(QVariant::String, new QStandardItemEditorCreator<QLineEdit>());
  QItemEditorFactory::setDefaultFactory(factory);

  // Instantiate visualization pipelines.
  m_Visualization = new Visualization();

  // Need to manually set the interactor to the QVTK widget's interactor.
  // Otherwise, the default interactor may be used. On Mac with Carbon,
  // the default interactor sucks up all the keypress events, making it
  // impossible to type anything in Qt widgets.
  m_Visualization->GetModelObjectRenderWindow()->
    SetInteractor(gui->modelObjectQvtkWidget->GetInteractor());
  gui->modelObjectQvtkWidget->SetRenderWindow(m_Visualization->GetModelObjectRenderWindow());

  gui->fluorescenceQvtkWidget->SetRenderWindow(m_Visualization->GetFluorescenceRenderWindow());
  gui->fluorescenceQvtkWidget->setMaximumSize(200, 200);
  gui->fluorescenceQvtkWidget->setMinimumSize(200, 200);
  gui->fluorescenceQvtkWidget->setHidden(true);
  gui->fluorescenceBackgroundWidget->setMaximumHeight(200);

  // Instantiate data model.
  m_SimulationNeedsSaving  = false;
  m_Simulation = new Simulation(this);

  m_Visualization->SetSimulation(m_Simulation);

  // Set application information
  QCoreApplication::setOrganizationName("CISMM");
  QCoreApplication::setOrganizationDomain("cismm.org");
  QCoreApplication::setApplicationName("Microscope Simulator");

  m_PSFMenuListModel = new QPSFListModel();
  m_PSFMenuListModel->SetHasNone(true);
  m_PSFMenuListModel->SetPSFList(m_Simulation->GetFluorescenceSimulation()->GetPSFList());
  gui->fluoroSimPSFMenuComboBox->setModel(m_PSFMenuListModel);
  
  m_ImageListModel = new QImageListModel();
  m_ImageListModel->SetModelObjectList(m_Simulation->GetModelObjectList());
  gui->fluoroSimComparisonImageComboBox->setModel(m_ImageListModel);

  // Set up error dialog box.
  m_ErrorDialog.setModal(true);

  m_ViewModeActionGroup = new QActionGroup(this);
  m_ViewModeActionGroup->setEnabled(true);
  m_ViewModeActionGroup->setVisible(true);
  m_ViewModeActionGroup->setExclusive(true);
  m_ViewModeActionGroup->addAction(gui->actionViewModelOnly);
  gui->actionViewModelOnly->setChecked(true);
  m_ViewModeActionGroup->addAction(gui->actionViewModelAndPoints);
  m_ViewModeActionGroup->addAction(gui->actionViewModelAndScan);
  m_ViewModeActionGroup->addAction(gui->actionViewAFMScanOnly);
  m_ViewModeActionGroup->addAction(gui->actionViewModelsWithFluorescenceComparison);
  m_ViewModeActionGroup->addAction(gui->actionViewFluorescenceComparisonOnly);

  m_InteractionActionGroup = new QActionGroup(this);
  m_InteractionActionGroup->setEnabled(true);
  m_InteractionActionGroup->setVisible(true);
  m_InteractionActionGroup->setExclusive(true);
  m_InteractionActionGroup->addAction(gui->actionMoveCamera);
  gui->actionMoveCamera->setChecked(true);
  m_InteractionActionGroup->addAction(gui->actionMoveObjects);
  
  m_ModelObjectListModel = new QModelObjectListModel();
  m_ModelObjectListModel->SetModelObjectList(m_Simulation->GetModelObjectList());
  gui->fluoroSimModelObjectList->setModel(m_ModelObjectListModel);

  m_ModelObjectListSelectionModel = gui->fluoroSimModelObjectList->selectionModel();
  connect(m_ModelObjectListSelectionModel,
          SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
          this,
          SLOT(handle_ModelObjectListSelectionModel_selectionChanged(const QItemSelection&, const QItemSelection&)));

  m_ModelObjectPropertyListTableModel = new QModelObjectPropertyListTableModel();
  gui->fluoroSimModelObjectPropertiesTable->setModel(m_ModelObjectPropertyListTableModel);
  connect(m_ModelObjectPropertyListTableModel,
          SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
          this, 
          SLOT(handle_ModelObjectPropertyListTableModel_dataChanged(const QModelIndex&, const QModelIndex&)));

  // Set up error reporter
  m_ErrorLogDialog = new ErrorLogDialog();
  m_ErrorLogDialog->setModal(false);
  m_ErrorLogger = vtkQtOutputLogger::New();
  m_ErrorLogger->SetTextWidget(m_ErrorLogDialog->GetPlainTextEdit());
  vtkOutputWindow::SetInstance(m_ErrorLogger);
  connect(m_ErrorLogDialog, SIGNAL(accepted()), this,
          SLOT(handle_ErrorLogDialog_accepted()));

  m_PSFEditorDialog = new PSFEditorDialog();
  m_PSFEditorDialog->setModal(true);
  m_PSFEditorDialog->SetPSFList
    (m_Simulation->GetFluorescenceSimulation()->GetPSFList());

  m_ImageExportOptionsDialog = new ImageExportOptionsDialog();
  m_ImageExportOptionsDialog->setModal(true);

  m_OptimizerSettingsDialog = new OptimizerSettingsDialog();
  m_OptimizerSettingsDialog->setModal(true);
  m_OptimizerSettingsDialog->SetFluorescenceOptimizer(m_Simulation->GetFluorescenceOptimizer());

  m_Preferences = new Preferences();
  
  m_PreferencesDialog = new PreferencesDialog(this, m_Preferences);
  m_PreferencesDialog->setModal(true);

  // Query for data directory if it is not set.
  if (m_Preferences->GetDataDirectoryPath() == "") {
    QString dataDirectoryPath = QDir::homePath();
    dataDirectoryPath.append(QDir::separator());
    dataDirectoryPath.append("MicroscopeSimulatorData");
    
    // Notify user that a data directory is needed
    QString message =
      QString("Microscope Simulator needs a directory in which to store "
              "certain data files. Use data directory '");
    message.append(dataDirectoryPath);
    message.append("'? If you click 'No', you will be able to select a "
                   "different data directory.");
    QMessageBox::StandardButton buttonClicked = 
      QMessageBox::question(this, tr("Create data directory?"), message,
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::Yes);
    if (buttonClicked == QMessageBox::Yes) {
      QDir().mkdir(dataDirectoryPath);
      m_Preferences->SetDataDirectoryPath(dataDirectoryPath.toStdString());
    } else {
      QFileDialog fileDialog(this, tr("Select data directory"),
                             QDir::homePath());
      fileDialog.setFileMode(QFileDialog::DirectoryOnly);
      if (fileDialog.exec()) {
        m_Preferences->SetDataDirectoryPath(fileDialog.directory().absolutePath().toStdString());
      }
    }
  }

  // Restore inter-session GUI settings.
  ReadProgramSettings();
  RefreshUI();
  RefreshObjectiveFunctions();
  RefreshModelObjectViews();
  on_actionResetCamera_triggered();
  gui->modelObjectQvtkWidget->GetRenderWindow()->Render();

  // Some of the above steps invoke the Sully() method which sets
  // m_SimulationNeedsSaving to true. We'll assume if the user makes
  // no changes, then he/she won't want to save the default settings.
  m_SimulationNeedsSaving = false;
  UpdateMainWindowTitle();
}


// Destructor
MicroscopeSimulator
::~MicroscopeSimulator() {
  delete m_Simulation;

  // The following line is necessary because the error logger may not be
  // deleted by VTK's garbage collector before the QPlainTextEdit in the
  // error log dialog is deleted.
  m_ErrorLogger->SetTextWidget(NULL);

  m_ErrorLogger->Delete();
  delete m_ErrorLogDialog;
  delete m_ImageExportOptionsDialog;
  delete m_PSFEditorDialog;
 
  delete m_Preferences;
  delete m_PreferencesDialog;
 
  delete m_Visualization;
  delete m_ViewModeActionGroup;
  delete m_InteractionActionGroup;
  delete m_ModelObjectPropertyListTableModel;

  delete gui;
}


void
MicroscopeSimulator
::on_actionNewSimulation_triggered() {
  NewSimulation();

  SetStatusMessage("Created new simulation.");

  RefreshModelObjectViews();
  RefreshUI();

  on_actionResetCamera_triggered();
  m_SimulationNeedsSaving = false;
  UpdateMainWindowTitle();
}


void
MicroscopeSimulator
::on_actionOpenSimulation_triggered() {
  NewSimulation();

  QSettings prefs;
  prefs.beginGroup("FileDialogs");
  QString directory = prefs.value("OpenSimulationDirectory").toString();

  QFileDialog fileDialog(this, "Open Simulation File", directory,
                         "XML Files (*.xml);;All Files (*)");
  fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
  int result = fileDialog.exec();
  prefs.setValue("OpenSimulationDirectory", fileDialog.directory().absolutePath());
  prefs.endGroup();

  if (result == QDialog::Rejected) 
    return;

  QString selectedFileName = fileDialog.selectedFiles()[0];

  if (selectedFileName.isEmpty()) {
    return;
  }

  // Now restore the Simulation from the XML tree.
  OpenSimulationFile(selectedFileName.toStdString());
}


void
MicroscopeSimulator
::on_actionSaveSimulation_triggered() {
  // See if we have already saved the file name. If so, overwrite the previous file.
  std::string fileName;
  if (m_Simulation->GetSimulationAlreadySaved()) {
    fileName = m_Simulation->GetSimulationFileName();
    SaveSimulationFile(fileName);
  } else {
    on_actionSaveSimulationAs_triggered();
  }
}


void
MicroscopeSimulator
::on_actionSaveSimulationAs_triggered() {
  // Get a file name.
  QSettings prefs;
  prefs.beginGroup("FileDialogs");
  QString directory = prefs.value("SaveSimulationDirectory").toString();
  
  QFileDialog fileDialog(this, "Save Simulation File As", directory, 
                         "XML Files (*.xml);;All Files (*)");
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  int result = fileDialog.exec();
  prefs.setValue("SaveSimulationDirectory",
                 fileDialog.directory().absolutePath());
  prefs.endGroup();

  if (result == QDialog::Rejected)
    return;

  QString selectedFileName = fileDialog.selectedFiles()[0];

  // Now read the file
  if (selectedFileName == "") {
    return;
  }
    
  SaveSimulationFile(selectedFileName.toStdString());
}


void
MicroscopeSimulator
::NewSimulation() {
  // Ask if user really wants to quit if the simulation has been modified.
  if (m_SimulationNeedsSaving) {
    if (PromptToSaveChanges() == QMessageBox::Cancel)
      return;
  }

  m_ModelObjectPropertyListTableModel->SetModelObject(NULL);
  m_Simulation->NewSimulation();
  m_Visualization->RefreshModelObjectView();
}


void
MicroscopeSimulator
::OpenSimulationFile(const std::string& fileName) {
  if (m_Simulation->OpenXMLConfiguration(fileName) < 0) {
    QString message = QString("Could not load simulation '").
      append(fileName.c_str()).append("'.");
    m_ErrorDialog.
      showMessage(tr("Could not open simulation file. Please make sure you "
                     "have permission to open the file."),
                  tr("FileOpenError"));
  } else {
    QString message = QString("Opened simulation '").append(fileName.c_str()).append("'.");
    SetStatusMessage(message.toStdString());

    m_Simulation->SetSimulationAlreadySaved(true);
    m_Simulation->SetSimulationFileName(fileName);
    m_SimulationNeedsSaving = false;

    m_ModelObjectListModel->Refresh();    
    RefreshModelObjectViews();
    on_actionResetCamera_triggered();
    RefreshUI();

    m_SimulationNeedsSaving = false;
    UpdateMainWindowTitle();
  }
}


void
MicroscopeSimulator
::SaveSimulationFile(const std::string& fileName) {
  QString message = tr("Saved simulation '").append(fileName.c_str()).append("'.");
  SetStatusMessage(message.toStdString());
  
  if (m_Simulation->SaveXMLConfiguration(fileName) < 0) {
    m_ErrorDialog.
      showMessage(tr("Could not save simulation file. Please make sure the "
                     "file is not open by another program and that you have "
                     "permission to write to the chosen directory."),
                  tr("FileSaveError"));
  } else {
    m_Simulation->SetSimulationAlreadySaved(true);
    m_Simulation->SetSimulationFileName(fileName);
  }
  m_SimulationNeedsSaving = false;

  UpdateMainWindowTitle();
}


void
MicroscopeSimulator
::AddNewModelObject(const std::string& objectName) {
  m_Simulation->AddNewModelObject(objectName);

  RefreshModelObjectViews();
  m_ModelObjectListModel->Refresh();

  // m_SelectedModelObjectIndex is set anytime the selection in the
  // model object list changes
  m_ModelObjectListSelectionModel->
    select(m_SelectedModelObjectIndex, QItemSelectionModel::Select);
}


void
MicroscopeSimulator
::on_actionExit_triggered() {
  Exit();
}


void
MicroscopeSimulator
::on_actionAddCylinder_triggered() {
  AddNewModelObject("CylinderModel");
}


void
MicroscopeSimulator
::on_actionAddHollowCylinder_triggered() {
  AddNewModelObject("HollowCylinderModel");
}


void
MicroscopeSimulator
::on_actionAddDisk_triggered() {
  AddNewModelObject("DiskModel");
}


void
MicroscopeSimulator
::on_actionAddFlexibleTube_triggered() {
  AddNewModelObject("FlexibleTubeModel");
}


void
MicroscopeSimulator
::on_actionAddPlane_triggered() {
  AddNewModelObject("PlaneModel");
}


void
MicroscopeSimulator
::on_actionAddPointRing_triggered() {
  AddNewModelObject("PointRingModel");
}


void
MicroscopeSimulator
::on_actionAddPointSet_triggered() {
  AddNewModelObject("PointSetModel");
}


void
MicroscopeSimulator
::on_actionAddSphere_triggered() {
  AddNewModelObject("SphereModel");
}


void
MicroscopeSimulator
::on_actionAddTorus_triggered() {
  AddNewModelObject("TorusModel");
}


void
MicroscopeSimulator
::on_actionImportImageData_triggered() {
  QSettings prefs;
  prefs.beginGroup("FileDialogs");
  QString directory = prefs.value("ImportImageDataDirectory").toString();

  QFileDialog fileDialog(this, "Open Image File", directory, 
                         "TIF Files (*.tif);;LSM Files (*.lsm);;All Files (*)");
  fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
  int result = fileDialog.exec();
  prefs.setValue("ImportImageDataDirectory", fileDialog.directory().absolutePath());
  prefs.endGroup();

  if (result == QDialog::Rejected)
    return;

  QString selectedFileName = fileDialog.selectedFiles()[0];
  if (selectedFileName.isEmpty()) {
    return;
  }
  
  m_Simulation->ImportModelObject("ImageModel", selectedFileName.toStdString());

  QString previousSelection = gui->fluoroSimComparisonImageComboBox->
    currentText();
  m_ImageListModel->Refresh();
  int index = gui->fluoroSimComparisonImageComboBox->findText(previousSelection);
  if (index == -1) {
    gui->fluoroSimComparisonImageComboBox->setCurrentIndex(0);
  } else {
    gui->fluoroSimComparisonImageComboBox->setCurrentIndex(index);
  }

  RefreshModelObjectViews();
  m_ModelObjectListModel->Refresh();

  // m_SelectedModelObjectIndex is set anytime the selection in the
  // model object list changes
  m_ModelObjectListSelectionModel->
    select(m_SelectedModelObjectIndex, QItemSelectionModel::Select);
}


void
MicroscopeSimulator
::on_actionImportGeometryFile_triggered() {
  QSettings prefs;
  prefs.beginGroup("FileDialogs");
  QString directory = prefs.value("ImportGeometryFileDirectory").toString();

  QFileDialog fileDialog(this, "Import Geometry File", directory,
                         "VTK Files (*.vtk);;VTK Poly Data XML Files (*.vtp);;OBJ Files (*.obj);;PLY Files (*.ply)");
  fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
  int result = fileDialog.exec();
  prefs.setValue("ImportGeometryFileDirectory", fileDialog.directory().absolutePath());
  prefs.endGroup();

  if (result == QDialog::Rejected)
    return;

  QString selectedFileName = fileDialog.selectedFiles()[0];
  if (selectedFileName.isEmpty()) {
    return;
  }

  m_Simulation->ImportModelObject("ImportedGeometry", selectedFileName.toStdString());

  RefreshModelObjectViews();
  m_ModelObjectListModel->Refresh();

  // m_SelectedModelObjectIndex is set anytime the selection in the
  // model object list changes
  m_ModelObjectListSelectionModel->
    select(m_SelectedModelObjectIndex, QItemSelectionModel::Select);
}


void
MicroscopeSimulator
::on_actionAboutApplication_triggered() {
  QString version = QString().sprintf("%d.%d.%d", 
				      MicroscopeSimulator_MAJOR_NUMBER,
				      MicroscopeSimulator_MINOR_NUMBER,
				      MicroscopeSimulator_REVISION_NUMBER);
  QChar copyright(169);
  QString title = QString("About Microscope Simulator ").append(version);
  QString text  = QString("Microscope Simulator ").append(version).append("\n");
  text.append(copyright).append(" 2009, UNC CISMM\n\n");
  text.append("Developed by:\n");
  text.append("Cory Quammen and Alvin Richardson");
  QMessageBox::about(this, title, text);
}


void
MicroscopeSimulator
::on_actionPreferences_triggered() {
  m_PreferencesDialog->exec();
}


void
MicroscopeSimulator
::on_actionSaveImage_triggered() {
  QSettings prefs;
  prefs.beginGroup("FileDialogs");
  QString directory = prefs.value("SaveImageDirectory").toString();

  // Get a file name.
  QFileDialog fileDialog(this, "Save Image File", directory, 
                         "PNG Files (*.png);;");
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  int result = fileDialog.exec();

  prefs.setValue("SaveImageDirectory", fileDialog.directory().absolutePath());
  prefs.endGroup();

  if (result == QDialog::Rejected)
    return;

  QString selectedFileName = fileDialog.selectedFiles()[0];
  if (selectedFileName.isEmpty())
    return;

  // Grab screen shot and save it.
  vtkSmartPointer<vtkWindowToImageFilter> w2if = 
    vtkSmartPointer<vtkWindowToImageFilter>::New();
  w2if->SetInput(gui->modelObjectQvtkWidget->GetRenderWindow());
  w2if->SetMagnification(1);
  w2if->Update();
  
  vtkSmartPointer<vtkPNGWriter> pngWriter =
    vtkSmartPointer<vtkPNGWriter>::New();
  pngWriter->SetInputConnection(w2if->GetOutputPort());
  pngWriter->SetFileName(selectedFileName.toStdString().c_str());
  pngWriter->Write();
}


void
MicroscopeSimulator
::on_actionResetCamera_triggered() {
  m_Visualization->ResetModelObjectCamera();
  m_Visualization->ModelObjectViewRender();
}


void
MicroscopeSimulator
::on_actionViewModelOnly_triggered() {
  m_Visualization->SetShowFluorophoresInModelObjectRenderer(false);
  m_Visualization->ModelObjectViewRender();
}


void
MicroscopeSimulator
::on_actionViewModelAndPoints_triggered() {
  m_Visualization->SetShowFluorophoresInModelObjectRenderer(true);
  m_Visualization->ModelObjectViewRender();
}


void
MicroscopeSimulator
::on_actionViewModelAndScan_triggered(){
  m_Visualization->SetShowFluorophoresInModelObjectRenderer(false);
  m_Visualization->ModelObjectViewRender();
}


void
MicroscopeSimulator
::on_actionViewAFMScanOnly_triggered() {
  m_Visualization->SetShowFluorophoresInModelObjectRenderer(false);
  m_Visualization->ModelObjectViewRender();
}


void
MicroscopeSimulator
::on_actionViewModelsWithFluorescenceComparison_triggered() {
  m_Visualization->SetShowFluorophoresInModelObjectRenderer(false);
  m_Visualization->ModelObjectViewRender();
}


void
MicroscopeSimulator
::on_actionViewFluorescenceComparisonOnly_triggered() {
  m_Visualization->SetShowFluorophoresInModelObjectRenderer(false);
  m_Visualization->ModelObjectViewRender();
}


void
MicroscopeSimulator
::on_actionEnableDisableAxes_triggered() {

}


void
MicroscopeSimulator
::on_actionMoveCamera_triggered() {
  m_Visualization->SetInteractionModeToCamera();
}


void
MicroscopeSimulator
::on_actionMoveObjects_triggered() {
  m_Visualization->SetInteractionModeToActor();
}


void
MicroscopeSimulator
::on_fluoroSimModelObjectList_customContextMenuRequested(QPoint point) {
  QMenu menu(tr("Model Object Menu"));
  menu.addAction(gui->actionFocusOnObject);
  menu.addAction(gui->actionExportGeometry);
  menu.addSeparator();
  menu.addAction(gui->actionDeleteModelObject);
  menu.exec(gui->fluoroSimModelObjectList->mapToGlobal(point));
}


void
MicroscopeSimulator
::on_actionFocusOnObject_triggered() {
  // See which object is selected in the list.
  int row = gui->fluoroSimModelObjectList->currentIndex().row();
  ModelObjectListPtr mol = m_Simulation->GetModelObjectList();
  ModelObject* object = mol->GetModelObjectAtIndex(row);
  if (!object) {
    SetStatusMessage("Error: could not focus on selected model object");
  }
  m_Visualization->FocusOnObject(object);
}


void
MicroscopeSimulator
::on_actionExportGeometry_triggered() {
  // See which object is selected in the list.
  int row = gui->fluoroSimModelObjectList->currentIndex().row();
  ModelObjectListPtr mol = m_Simulation->GetModelObjectList();
  ModelObject* object = mol->GetModelObjectAtIndex(row);
  if (!object) {
    SetStatusMessage("Error: could not find geometry for selected model oject");
  }

  QString selectedFileName = 
    QFileDialog::getSaveFileName(this, "Export Model Geometry", "", "VTK File (*.vtk);;VTK Poly Data File (*.vtp);;PLY File (*.ply);;BYU File (*.byu)");
  if (selectedFileName.isEmpty())
    return;

  vtkSmartPointer<vtkPolyDataWriter> writer = NULL;
  if (selectedFileName.endsWith(tr(".vtk"))) {
    writer = vtkSmartPointer<vtkPolyDataWriter>::New();
  } else if (selectedFileName.endsWith(tr(".ply"))) {
    writer = vtkSmartPointer<vtkPLYWriter>::New();
  } else if (selectedFileName.endsWith(tr(".byu"))) {
    vtkSmartPointer<vtkBYUWriter> byuWriter = vtkSmartPointer<vtkBYUWriter>::New();
    byuWriter->SetGeometryFileName(selectedFileName.toStdString().c_str());
    byuWriter->WriteDisplacementOff();
    byuWriter->WriteScalarOff();
    byuWriter->WriteTextureOff();
    writer = byuWriter;
  } else {
    SetStatusMessage("Error: Could not export geometry");
  }

  if (writer) {
            
    vtkPolyDataAlgorithm* geometrySource = object->GetAllGeometryTransformed();

    writer->SetInputConnection(geometrySource->GetOutputPort());
    geometrySource->Delete();
    writer->SetFileName(selectedFileName.toStdString().c_str());
    writer->Update();
  }

  QString message = QString().append(tr("Exported model geometry to file '")).
    append(selectedFileName.toStdString().c_str()).append(tr("'."));
  SetStatusMessage(message.toStdString());
}


void
MicroscopeSimulator
::on_actionDeleteModelObject_triggered() {
  // See which object is selected in the list.
  int row = gui->fluoroSimModelObjectList->currentIndex().row();

  ModelObjectListPtr mol = m_Simulation->GetModelObjectList();
  if (row >= 0 && row < static_cast<int>(mol->GetSize())) {
    mol->Delete(mol->GetModelObjectAtIndex(row));

    RefreshModelObjectViews();
    m_ModelObjectListModel->Refresh();
  }
  m_ModelObjectPropertyListTableModel->SetModelObject(NULL);
  m_ModelObjectPropertyListTableModel->Refresh();
  m_ImageListModel->Refresh();
}


void
MicroscopeSimulator
::handle_ModelObjectListSelectionModel_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
  QModelIndexList indexes = selected.indexes();
  if (!indexes.isEmpty() && indexes[0].row() >= 0) {
    m_SelectedModelObjectIndex = indexes[0];

    ModelObjectListPtr mol = m_Simulation->GetModelObjectList();
    ModelObjectPtr mo = mol->GetModelObjectAtIndex(indexes[0].row());

    m_ModelObjectPropertyListTableModel->SetModelObject(mo);

  } else {
    m_ModelObjectPropertyListTableModel->SetModelObject(NULL);
  }
  m_ModelObjectPropertyListTableModel->Refresh();
}


void
MicroscopeSimulator
::on_fluoroSimModelObjectPropertiesTable_clicked(const QModelIndex& index) {
  ModelObject* object = m_ModelObjectPropertyListTableModel->GetModelObject();
  if (object) {
    ModelObjectProperty* prop = object->GetProperty(index.row());
    
    if (prop && prop->GetType() == ModelObjectProperty::FLUOROPHORE_MODEL_TYPE) {
      FluorophoreModelObjectProperty* fluorProp =
        dynamic_cast<FluorophoreModelObjectProperty*>(prop);
      if (fluorProp) {
        FluorophoreModelDialog* dialog = new FluorophoreModelDialog(fluorProp);
        int result = dialog->exec();
        if (result == QDialog::Accepted) {

          // Save the dialog values to the fluorophore model property
          dialog->SaveSettingsToProperty();
          Sully();
          RefreshModelObjectViews();
          RenderViews();
        }
      }
    }
  }
}


void
MicroscopeSimulator
::handle_ModelObjectPropertyListTableModel_dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight) {
  RefreshModelObjectViews();

  // This ensures that when an object's name is changed that it shows up
  // immediately in the model object list.
  gui->fluoroSimModelObjectList->update(m_ModelObjectListSelectionModel->currentIndex());
}


void
MicroscopeSimulator
::on_experimentDescriptionTextArea_textChanged() {
  QString text = gui->experimentDescriptionTextArea->toPlainText();
  m_Simulation->SetSimulationDescription(text.toStdString());
}


void
MicroscopeSimulator
::on_afmSimRadiusSpinBox_valueChanged(int i) {
}


void
MicroscopeSimulator
::on_afmSimUseConeSphereModelCheckBox_stateChanged(int state) {

}


void
MicroscopeSimulator
::on_afmSimConeAngleEdit_textEdited(const QString& text) {

}


void
MicroscopeSimulator
::on_afmSimPixelSizeEdit_textEdited(const QString& text) {
  m_Simulation->GetAFMSimulation()->SetPixelSize(text.toDouble());
}


void
MicroscopeSimulator
::on_afmSimImageWidthEdit_textEdited(const QString& text) {
  m_Simulation->GetAFMSimulation()->SetImageWidth(text.toInt());
}


void
MicroscopeSimulator
::on_afmSimImageHeightEdit_textEdited(const QString& text) {
  m_Simulation->GetAFMSimulation()->SetImageHeight(text.toInt());
}


void
MicroscopeSimulator
::on_afmSimClipGroundPlaneCheckBox_stateChanged(int state) {
  m_Simulation->GetAFMSimulation()->SetClipGroundPlane(state == Qt::Checked);
}


void
MicroscopeSimulator
::on_afmSimDisplayAsWireframeCheckBox_stateChanged(int state) {
  m_Simulation->GetAFMSimulation()->SetDisplayAsWireframe(state == Qt::Checked);
}


void
MicroscopeSimulator
::on_afmSimSurfaceOpacityEdit_textEdited(const QString& text) {
  m_Simulation->GetAFMSimulation()->SetSurfaceOpacity(text.toDouble());
}


void
MicroscopeSimulator
::on_afmSimCompareScanToComboBox_currentIndexChanged(int index) {
  // TODO
}


void
MicroscopeSimulator
::on_afmSimSaveSimulatedScan_clicked() {
  // TODO
}


void
MicroscopeSimulator
::on_afmSimScanStatistics_clicked() {
  // TODO
}


void
MicroscopeSimulator
::on_actionShowErrors_toggled(bool visible) {
  if (visible) {
    m_ErrorLogDialog->show();
  } else {
    m_ErrorLogDialog->hide();
  }
}


void
MicroscopeSimulator
::handle_ErrorLogDialog_accepted() {
  gui->actionShowErrors->setChecked(false);
}


void
MicroscopeSimulator
::on_actionFluorescenceWindow_toggled(bool visible) {
  gui->fluorescenceQvtkWidget->setHidden(!visible);
  if (visible) {
    m_Visualization->FluorescenceViewRender();
  }
}


void
MicroscopeSimulator
::on_fluoroSimFocusSlider_valueChanged(int value) {
  // Convert value into focal plane position
  float focusSpacing = gui->fluoroSimFocusSpacingEdit->text().toFloat();
  float focalPlaneDepth = static_cast<float>(value) * focusSpacing;

  m_Simulation->GetFluorescenceSimulation()->SetFocalPlaneDepth(focalPlaneDepth);

  // Set text in fluoroSimFocusEdit to slider value
  gui->fluoroSimFocusEdit->setText(QString().sprintf("%0.1f", focalPlaneDepth));

  RenderViews();

  Sully();
}


void
MicroscopeSimulator
::on_fluoroSimFocusMaxEdit_editingFinished() {
  UpdateFocalPlaneUIControls(gui->fluoroSimFocusMinEdit->text().toFloat(),
    gui->fluoroSimFocusMaxEdit->text().toFloat(),
    gui->fluoroSimFocusSpacingEdit->text().toFloat());

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimFocusMinEdit_editingFinished() {
  UpdateFocalPlaneUIControls(gui->fluoroSimFocusMinEdit->text().toFloat(),
    gui->fluoroSimFocusMaxEdit->text().toFloat(),
    gui->fluoroSimFocusSpacingEdit->text().toFloat());

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimFocusSpacingEdit_editingFinished() {
  UpdateFocalPlaneUIControls(gui->fluoroSimFocusMinEdit->text().toFloat(),
    gui->fluoroSimFocusMaxEdit->text().toFloat(),
    gui->fluoroSimFocusSpacingEdit->text().toFloat());

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimPSFMenuComboBox_currentIndexChanged(int selected) {
  m_Simulation->GetFluorescenceSimulation()->SetActivePSFIndex(selected-1);
}


void
MicroscopeSimulator
::on_fluoroSimEditPSFsButton_clicked() {
  QString previousSelection = gui->fluoroSimPSFMenuComboBox->currentText();
  int result = m_PSFEditorDialog->exec();
  if (result == QDialog::Accepted) {
    m_PSFMenuListModel->Refresh();
    // Need to set the selected index back to what it was originally.
    // Apparently, resetting the model via Refresh affects the selection
    // model.
    int index = gui->fluoroSimPSFMenuComboBox->findText(previousSelection);
    if (index == -1) {
      gui->fluoroSimPSFMenuComboBox->setCurrentIndex(0);
    } else {
      gui->fluoroSimPSFMenuComboBox->setCurrentIndex(index);
    }
  }

  // Save the PSF file
  WritePSFSettings();
}


void
MicroscopeSimulator
::on_fluoroSimShowImageVolumeOutlineCheckBox_toggled(bool show) {
  m_Simulation->GetFluorescenceSimulation()->SetShowImageVolumeOutline(show);

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimExposureEdit_textChanged(QString text) {
  double exposure = text.toDouble();
  m_Simulation->GetFluorescenceSimulation()->SetExposure(exposure);

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimPixelSizeEdit_editingFinished() {
  double pixelSize = gui->fluoroSimPixelSizeEdit->text().toDouble();
  m_Simulation->GetFluorescenceSimulation()->SetPixelSize(pixelSize);

  m_Visualization->FluorescenceViewRender();

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimImageWidthEdit_editingFinished() {
  int width = gui->fluoroSimImageWidthEdit->text().toInt();
  gui->fluorescenceQvtkWidget->setMinimumWidth(width);
  gui->fluorescenceQvtkWidget->setMaximumWidth(width);

  m_Simulation->GetFluorescenceSimulation()->SetImageWidth(width);

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimImageHeightEdit_editingFinished() {
  int height = gui->fluoroSimImageHeightEdit->text().toInt();
  gui->fluorescenceQvtkWidget->setMinimumHeight(height);
  gui->fluorescenceQvtkWidget->setMaximumHeight(height);
  gui->fluorescenceBackgroundWidget->setMaximumHeight(height);

  m_Simulation->GetFluorescenceSimulation()->SetImageHeight(height);

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimAddGaussianNoiseCheckBox_toggled(bool value) {
  m_Simulation->GetFluorescenceSimulation()->SetAddGaussianNoise(value);

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimStdDevEdit_editingFinished() {
  double stdDev = gui->fluoroSimStdDevEdit->text().toDouble();
  m_Simulation->GetFluorescenceSimulation()->SetNoiseStdDev(stdDev);

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimMeanEdit_editingFinished() {
  double mean = gui->fluoroSimMeanEdit->text().toDouble();
  m_Simulation->GetFluorescenceSimulation()->SetNoiseMean(mean);

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimShowReferencePlaneCheckBox_toggled(bool show) {
  m_Simulation->GetFluorescenceSimulation()->SetShowReferencePlane(show);

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimShowGridCheckBox_toggled(bool show) {
  m_Simulation->GetFluorescenceSimulation()->SetShowReferenceGrid(show);

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimSuperimposeFluorescenceImageCheckBox_toggled(bool show) {
  m_Simulation->GetFluorescenceSimulation()->SetSuperimposeFluorescenceImage(show);

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimGridSpacingEdit_editingFinished() {
  double spacing = gui->fluoroSimGridSpacingEdit->text().toDouble();
  m_Simulation->GetFluorescenceSimulation()->SetReferenceGridSpacing(spacing);

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimMinLevelEdit_editingFinished() {
  double level = gui->fluoroSimMinLevelEdit->text().toDouble();
  m_Simulation->GetFluorescenceSimulation()->SetMinimumIntensityLevel(level);

  m_Visualization->FluorescenceViewRender();
}


void
MicroscopeSimulator
::on_fluoroSimMinLevelSlider_valueChanged(int value) {
  double dValue = static_cast<double>(value);
  m_Simulation->GetFluorescenceSimulation()->SetMinimumIntensityLevel(dValue);
  m_Visualization->FluorescenceViewRender();
  
  gui->fluoroSimMinLevelEdit->setText(QString().sprintf("%0.1f", dValue));
}


void
MicroscopeSimulator
::on_fluoroSimMaxLevelEdit_editingFinished() {
  double level = gui->fluoroSimMaxLevelEdit->text().toDouble();
  m_Simulation->GetFluorescenceSimulation()->SetMaximumIntensityLevel(level);
  m_Visualization->FluorescenceViewRender();
}


void
MicroscopeSimulator
::on_fluoroSimMaxLevelSlider_valueChanged(int value) {
  double dValue = static_cast<double>(value);
  m_Simulation->GetFluorescenceSimulation()->SetMaximumIntensityLevel(dValue);
  m_Visualization->FluorescenceViewRender();

  gui->fluoroSimMaxLevelEdit->setText(QString().sprintf("%0.1f", dValue));
}


void
MicroscopeSimulator
::on_fluoroSimScaleToFullDynamicRange_clicked() {
  m_Visualization->FluorescenceViewRender();
  double scalarRange[2];
  m_Visualization->GetFluorescenceScalarRange(scalarRange);

  gui->fluoroSimMinLevelEdit->setText(QVariant(scalarRange[0]).toString());
  gui->fluoroSimMinLevelSlider->setValue(static_cast<int>(scalarRange[0]));
  gui->fluoroSimMaxLevelEdit->setText(QVariant(scalarRange[1]).toString());
  gui->fluoroSimMaxLevelSlider->setValue(static_cast<int>(scalarRange[1]));
}


void
MicroscopeSimulator
::on_fluoroSimExportImageButton_clicked() {
  QSettings prefs;
  prefs.beginGroup("FileDialogs");
  QString directory  = prefs.value("ExportImageDirectory").toString();
  QString nameFilter = prefs.value("ExportImageNameFilter").toString();
  
  int result;

#if 0
  // Now get the options for the export
  result = m_ImageExportOptionsDialog->exec();
  if (result == QDialog::Rejected)
    return;
#endif

  QFileDialog fileDialog(this, tr("Export Fluorescence Image"), directory,
                         "PNG File (*.png);;BMP File (*.bmp);;JPG File (*.jpg);;16-bit TIFF File (*.tif)");
  fileDialog.selectNameFilter(nameFilter);
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  result = fileDialog.exec();
  prefs.setValue("ExportImageDirectory", fileDialog.directory().absolutePath());
  prefs.setValue("ExportImageNameFilter", fileDialog.selectedNameFilter());
  prefs.endGroup();

  if (result == QDialog::Rejected)
    return;

  QString selectedFileName = fileDialog.selectedFiles()[0];
  if (selectedFileName.isEmpty())
    return;

  vtkImageData* image = m_Visualization->GenerateFluorescenceImage();

  vtkSmartPointer<vtkImageShiftScale> scaler = vtkSmartPointer<vtkImageShiftScale>::New();
  scaler->SetOutputScalarTypeToUnsignedChar();
  scaler->ClampOverflowOn();
  scaler->SetInput(image);
  image->Delete();

  double minIntensity = m_Simulation->GetFluorescenceSimulation()->GetMinimumIntensityLevel();
  double maxIntensity = m_Simulation->GetFluorescenceSimulation()->GetMaximumIntensityLevel();
  scaler->SetShift(-minIntensity);
  scaler->SetScale(255.0 / (maxIntensity - minIntensity));

  vtkSmartPointer<vtkImageWriter> writer;
  QString extension = fileDialog.selectedNameFilter().right(4).left(3).toLower();
  if (extension == QString("png")) {
    writer = vtkSmartPointer<vtkPNGWriter>::New();
  } else if (extension == QString("bmp")) {
    writer = vtkSmartPointer<vtkBMPWriter>::New();
  } else if (extension == QString("jpg")) {
    writer = vtkSmartPointer<vtkJPEGWriter>::New();
  } else if (extension == QString("tif")) {
    vtkSmartPointer<vtkTIFFWriter> tiffWriter = vtkSmartPointer<vtkTIFFWriter>::New();
    tiffWriter->SetCompressionToNoCompression();
    writer = tiffWriter;
    
    scaler->SetShift(0.0);
    scaler->SetScale(1.0);
    scaler->SetOutputScalarTypeToUnsignedShort();
  }
  
  if (writer) {
    writer->SetInputConnection(scaler->GetOutputPort());
    writer->SetFileName(selectedFileName.toStdString().c_str());
    writer->Write();
  }
}


void
MicroscopeSimulator
::on_fluoroSimExportStackButton_clicked() {
  QSettings prefs;
  prefs.beginGroup("FileDialogs");
  QString directory  = prefs.value("ExportStackDirectory").toString();
  QString nameFilter = prefs.value("ExportStackNameFilter").toString();

  int result;

#if 0
  // Now get the options for the export
  result = m_ImageExportOptionsDialog->exec();
  if (result == QDialog::Rejected)
    return;
#endif
  
  QFileDialog fileDialog(this, tr("Export Fluorescence Stack"), directory,
                         "16-bit TIFF File (*.tif)");
  fileDialog.selectNameFilter(nameFilter);
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  if (fileDialog.exec() == QDialog::Rejected)
    return;
  prefs.setValue("ExportStackDirectory", fileDialog.directory().absolutePath());
  prefs.setValue("ExportStackNameFilter", fileDialog.selectedNameFilter());
  prefs.endGroup();

  QString selectedFileName = fileDialog.selectedFiles()[0];
  if (selectedFileName.isEmpty())
    return;

  FluorescenceSimulation* fluoroSim = m_Simulation->GetFluorescenceSimulation();
  double originalDepth = fluoroSim->GetFocalPlaneDepth();

  vtkSmartPointer<vtkImageExtractComponents> extractor = vtkSmartPointer<vtkImageExtractComponents>::New();
  extractor->SetComponents(0);

  vtkImageData* rawStack = m_Visualization->GenerateFluorescenceStackImage();
  extractor->SetInput(rawStack);
  rawStack->Delete();

  try {
    ImageWriter writer;
    writer.SetFileName(selectedFileName.toStdString());
    writer.SetInputConnection(extractor->GetOutputPort());
    writer.WriteUShortImage();
  } catch (itk::ExceptionObject e) {
    std::cout << "Error on write:" << std::endl;
    std::cout << e.GetDescription() << std::endl;
  }

  // Reset to original focal plane depth
  fluoroSim->SetFocalPlaneDepth(originalDepth);
  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimComparisonImageComboBox_currentIndexChanged(int selected) {
  m_Simulation->SetComparisonImageModelObjectIndex(selected-1);
}


void
MicroscopeSimulator
::on_fluoroSimCopyImageSettingsButton_clicked() {
  ImageModelObject* mo = m_Simulation->GetComparisonImageModelObject();
  if (!mo)
    return;

  int dims[3];
  mo->GetDimensions(dims);

  double spacing[3];
  mo->GetSpacing(spacing);

  FluorescenceSimulation* sim = m_Simulation->GetFluorescenceSimulation();
  sim->SetImageWidth(dims[0]);
  sim->SetImageHeight(dims[1]);
  sim->SetPixelSize(spacing[0]);

  sim->SetFocalPlaneDepthMinimum(0.0);
  sim->SetFocalPlaneDepthMaximum(static_cast<double>((dims[2]-1)*spacing[2]));
  sim->SetFocalPlaneDepthSpacing(spacing[2]);

  RefreshUI();
}


void
MicroscopeSimulator
::on_fluoroSimOptimizationMethodComboBox_currentIndexChanged(int selected) {
  if (selected == 0) {
    m_Simulation->SetFluorescenceOptimizerToNelderMead();
  } else if (selected == 1) {
    m_Simulation->SetFluorescenceOptimizerToGradientDescent();
  } else if (selected == 2) {
    m_Simulation->SetFluorescenceOptimizerToPointsGradient();
  } else {
    QMessageBox messageBox;
    QString message("Optimization method '");
    message.append(gui->fluoroSimOptimizationMethodComboBox->currentText());
    message.append("' not yet implemented.");
    messageBox.setText(message);
    messageBox.setStandardButtons(QMessageBox::Ok);
    messageBox.setDefaultButton(QMessageBox::Ok);
    messageBox.exec();

    gui->fluoroSimOptimizationMethodComboBox->setCurrentIndex(0);
  }

  RefreshObjectiveFunctions();
}


void
MicroscopeSimulator
::on_fluoroSimOptimizerSettingsButton_clicked() {
  m_OptimizerSettingsDialog->SetFluorescenceOptimizer(m_Simulation->GetFluorescenceOptimizer());
  m_OptimizerSettingsDialog->Update();
  m_OptimizerSettingsDialog->exec();
}


void
MicroscopeSimulator
::on_fluoroSimObjectiveFunctionComboBox_currentIndexChanged(int selected) {
  FluorescenceOptimizer* optimizer = m_Simulation->GetFluorescenceOptimizer();
  if (selected == 0) {
    //optimizer->SetCostFunctionToGaussianNoise();
  } else if (selected == 1) {
    //optimizer->SetCostFunctionToPoissonNoise();
  } else if (selected == 2) {
    //optimizer->SetCostFunctionToNormalizedCorrelation();
  }
}


void
MicroscopeSimulator
::on_fluoroSimOptimizeButton_clicked() {
  QMessageBox messageBox;

  if (m_Simulation->GetComparisonImageModelObject()) {
    messageBox.setText(tr("WARNING: Optimization may take a long time. Run the optimizer?"));
    messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    messageBox.setDefaultButton(QMessageBox::Yes);
    
    int selected = messageBox.exec();
    if (selected == QMessageBox::Yes) {
      m_Simulation->OptimizeToFluorescence();
    }
  } else {
    messageBox.setText(tr("Please select an image in the 'Comparison image' menu."));
    messageBox.setStandardButtons(QMessageBox::Ok);
    messageBox.setDefaultButton(QMessageBox::Ok);
    messageBox.exec();
  }
}


void
MicroscopeSimulator
::UpdateFocalPlaneUIControls(float minValue, float maxValue, float spacing) {
  // Round values.
  int minIndex = floor(minValue / spacing);
  minValue = static_cast<float>(minIndex) * spacing;
  int maxIndex = ceil(maxValue / spacing);
  maxValue = static_cast<float>(maxIndex) * spacing;

  // Update widgets
  gui->fluoroSimFocusSlider->setMinimum(minIndex);
  gui->fluoroSimFocusSlider->setMaximum(maxIndex);
  gui->fluoroSimFocusMinEdit->setText(QString().sprintf("%0.1f", minValue));
  gui->fluoroSimFocusMaxEdit->setText(QString().sprintf("%0.1f", maxValue));
  gui->fluoroSimFocusSpacingEdit->setText(QString().sprintf("%0.1f", spacing));

  m_Simulation->GetFluorescenceSimulation()->
    SetFocalPlaneDepthSpacing(gui->fluoroSimFocusSpacingEdit->text().toDouble());

  m_Simulation->GetFluorescenceSimulation()->
    SetFocalPlaneDepthMinimum(gui->fluoroSimFocusMinEdit->text().toDouble());

  m_Simulation->GetFluorescenceSimulation()->
    SetFocalPlaneDepthMaximum(gui->fluoroSimFocusMaxEdit->text().toDouble());
}


int
MicroscopeSimulator
::PromptToSaveChanges() {
  QString message = QString("Save changes to '").
    append(m_Simulation->GetSimulationFileName().c_str()).append("'?");
  QMessageBox messageBox;
  messageBox.setText(message);
  messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
  messageBox.setDefaultButton(QMessageBox::Yes);

  int selected = messageBox.exec();
  if (selected == QMessageBox::Yes) {
    on_actionSaveSimulation_triggered();
  }

  return selected;
}


void
MicroscopeSimulator
::UpdateMainWindowTitle() {
  ///////////////// Update window title /////////////////
  QString version = QString().sprintf("%d.%d.%d", 
    MicroscopeSimulator_MAJOR_NUMBER,
    MicroscopeSimulator_MINOR_NUMBER,
		MicroscopeSimulator_REVISION_NUMBER);
  QString windowTitle("Microscope Simulator ");
  windowTitle.append(version);
  std::string fileName = m_Simulation->GetSimulationFileName();
  if (fileName != "")
    windowTitle.append(tr(" - ").append(fileName.c_str()));
  if (m_SimulationNeedsSaving) {
    windowTitle.append(tr("*"));
  }
  setWindowTitle(windowTitle);
}


void
MicroscopeSimulator
::RefreshModelObjectViews() {
  m_Visualization->RefreshModelObjectView();

  RenderViews();
}


void
MicroscopeSimulator
::RenderViews() {
  m_Visualization->ModelObjectViewRender();

  if (!gui->fluorescenceQvtkWidget->isHidden()) {
    m_Visualization->FluorescenceViewRender();
  }
}


void
MicroscopeSimulator
::RefreshSimulationDescription() {
  gui->experimentDescriptionTextArea->
    setPlainText(QString(m_Simulation->GetSimulationDescription().c_str()));
}


void
MicroscopeSimulator
::RefreshUI() {
  RefreshSimulationDescription();

  m_SimulationNeedsSaving = false;
  UpdateMainWindowTitle();

  // Refresh AFMSim UI widgets
  AFMSimulation* afmSim = m_Simulation->GetAFMSimulation();
  
  gui->afmSimPixelSizeEdit->setText(QVariant(afmSim->GetPixelSize()).toString());
  gui->afmSimImageWidthEdit->setText(QVariant(afmSim->GetImageWidth()).toString());
  gui->afmSimImageHeightEdit->setText(QVariant(afmSim->GetImageHeight()).toString());
  gui->afmSimClipGroundPlaneCheckBox->setChecked(QVariant(afmSim->GetClipGroundPlane()).toBool());
  gui->afmSimDisplayAsWireframeCheckBox->setChecked(QVariant(afmSim->GetDisplayAsWireframe()).toBool());
  gui->afmSimSurfaceOpacityEdit->setText(QVariant(afmSim->GetSurfaceOpacity()).toString());

  // Refresh FluoroSim UI widgets
  FluorescenceSimulation* fluoroSim = m_Simulation->GetFluorescenceSimulation();

  // Focus group box
  UpdateFocalPlaneUIControls(fluoroSim->GetFocalPlaneDepthMinimum(),
                             fluoroSim->GetFocalPlaneDepthMaximum(),
                             fluoroSim->GetFocalPlaneDepthSpacing());
  int focusSliderValue = (int) ((fluoroSim->GetFocalPlaneDepth() / fluoroSim->GetFocalPlaneDepthSpacing()));
  gui->fluoroSimFocusSlider->setValue(focusSliderValue);

  gui->fluoroSimPSFMenuComboBox->
    setCurrentIndex(fluoroSim->GetActivePSFIndex()+1);

  // Simulator Settings group box
  gui->fluoroSimExposureEdit->setText(QVariant(fluoroSim->GetExposure()).toString());
  gui->fluoroSimPixelSizeEdit->setText(QVariant(fluoroSim->GetPixelSize()).toString());
  gui->fluoroSimImageWidthEdit->setText(QVariant(fluoroSim->GetImageWidth()).toString());
  gui->fluoroSimImageHeightEdit->setText(QVariant(fluoroSim->GetImageHeight()).toString());

  gui->fluorescenceQvtkWidget->setMinimumWidth(fluoroSim->GetImageWidth());
  gui->fluorescenceQvtkWidget->setMaximumWidth(fluoroSim->GetImageWidth());
  gui->fluorescenceQvtkWidget->setMinimumHeight(fluoroSim->GetImageHeight());
  gui->fluorescenceQvtkWidget->setMaximumHeight(fluoroSim->GetImageHeight());

  gui->fluoroSimAddGaussianNoiseCheckBox->setChecked(QVariant(fluoroSim->GetAddGaussianNoise()).toBool());
  gui->fluoroSimStdDevEdit->setText(QVariant(fluoroSim->GetNoiseStdDev()).toString());
  gui->fluoroSimMeanEdit->setText(QVariant(fluoroSim->GetNoiseMean()).toString());

  // Fluorescence Display group box
  gui->fluoroSimShowReferencePlaneCheckBox->setChecked(QVariant(fluoroSim->GetShowReferencePlane()).toBool());
  gui->fluoroSimShowGridCheckBox->setChecked(QVariant(fluoroSim->GetShowReferenceGrid()).toBool());
  gui->fluoroSimGridSpacingEdit->setText(QVariant(fluoroSim->GetReferenceGridSpacing()).toString());
  gui->fluoroSimSuperimposeFluorescenceImageCheckBox->setChecked(QVariant(fluoroSim->GetSuperimposeFluorescenceImage()).toBool());
  gui->fluoroSimShowImageVolumeOutlineCheckBox->setChecked(QVariant(fluoroSim->GetShowImageVolumeOutline()).toBool());
  gui->fluoroSimMinLevelSlider->setValue((int) fluoroSim->GetMinimumIntensityLevel());
  gui->fluoroSimMaxLevelSlider->setValue((int) fluoroSim->GetMaximumIntensityLevel());

  // Select the chosen fluorescence comparison image model object
  ImageModelObject* comparisonImage = m_Simulation->GetComparisonImageModelObject();
  if (!comparisonImage) {
    gui->fluoroSimComparisonImageComboBox->setCurrentIndex(0);
  } else {
    for (int i = 1; i < m_ImageListModel->rowCount(); i++) {
      QModelIndex index = m_ImageListModel->index(i, 0);
      QVariant data = m_ImageListModel->data(index);
      if (data.isValid()) {
        QString name = m_ImageListModel->data(index).toString();
        if (name.toStdString() == comparisonImage->GetName()) {
          gui->fluoroSimComparisonImageComboBox->setCurrentIndex(i);
          break;
        }
      }
    }
  }

  RenderViews();
}

void
MicroscopeSimulator
::RefreshObjectiveFunctions() {
  QString selectedObjectiveFunction = gui->fluoroSimObjectiveFunctionComboBox->currentText();
  gui->fluoroSimObjectiveFunctionComboBox->clear();

  FluorescenceOptimizer* optimizer = m_Simulation->GetFluorescenceOptimizer();
  if (!optimizer)
    return;

  // Repopulate by the objective functions available in the optimizer
  for (int i = 0; i < optimizer->GetNumberOfAvailableObjectiveFunctions(); i++) {
    std::string name = optimizer->GetAvailableObjectiveFunctionName(i);
    gui->fluoroSimObjectiveFunctionComboBox->addItem(QString(name.c_str()));

    if (name == selectedObjectiveFunction.toStdString()) {
      gui->fluoroSimObjectiveFunctionComboBox->setCurrentIndex(i);
    }
  }


}


void
MicroscopeSimulator
::SetStatusMessage(const std::string& status) {
  gui->statusbar->showMessage(QString(status.c_str()));
}


void
MicroscopeSimulator
::Sully() {
  if (!m_Simulation) {
    return;
  }

  m_SimulationNeedsSaving = true;

  UpdateMainWindowTitle();

  if (m_ModelObjectPropertyListTableModel) {
    m_ModelObjectPropertyListTableModel->Refresh();
  }
  
  RenderViews();
}


void
MicroscopeSimulator
::Exit() {
  // Ask if user really wants to quit if the simulation has been modified.
  if (m_SimulationNeedsSaving) {
    int selected = PromptToSaveChanges();
    if (selected == QMessageBox::Cancel)
      return;
  }

  WriteProgramSettings();
  qApp->exit();
}


void
MicroscopeSimulator
::WriteProgramSettings() {
  QSettings settings;

  // Save size and position of the main window.
  settings.beginGroup("MainWindow");
  settings.setValue("WindowSettings", saveState());
  settings.setValue("Geometry", saveGeometry());
  settings.setValue("ModelObjectPanelSplitterSizes",
                    gui->fluoroSimModelObjectSplitter->saveState());
  settings.endGroup();

  m_PSFEditorDialog->SaveGUISettings();
  m_OptimizerSettingsDialog->SaveGUISettings();

  WritePSFSettings();
}


void
MicroscopeSimulator
::WritePSFSettings() {
  // Write out PSF list settings
  
  /* Create a new XML DOM tree, to which the XML document will be written. */
  xmlDocPtr doc = xmlNewDoc(BAD_CAST XML_DEFAULT_VERSION);
  if (doc == NULL) {
    std::cout << "MicroscopeSimulator: Error creating the PSF XML document tree" << std::endl;
    return;
  }

  // Create a new XML node for the PSF list
  xmlNodePtr node = xmlNewDocNode(doc, NULL, BAD_CAST "PSFList", NULL);
  if (node == NULL) {
    std::cout << "Simulation: Error creating the XML node" << std::endl;
    return;
  }

  m_Simulation->GetFluorescenceSimulation()->GetPSFList()->
    GetXMLConfiguration(node);

  xmlDocSetRootElement(doc, node);

  // Now save PSF list file
  QString psfSettingsFileName(m_Preferences->GetDataDirectoryPath().c_str());
  psfSettingsFileName.append(QDir::separator()).append("PSFList.xml");
  xmlSaveFileEnc(psfSettingsFileName.toStdString().c_str(), doc, "ISO-8859-1");
  xmlFreeDoc(doc);
}


void
MicroscopeSimulator
::ReadProgramSettings() {
  QSettings settings;

  // Read main window settings
  settings.beginGroup("MainWindow");
  restoreState(settings.value("WindowSettings").toByteArray());
  restoreGeometry(settings.value("Geometry").toByteArray());
  gui->fluoroSimModelObjectSplitter->
    restoreState(settings.value("ModelObjectPanelSplitterSizes").toByteArray());
  settings.endGroup();

  m_PSFEditorDialog->LoadGUISettings();
  m_OptimizerSettingsDialog->LoadGUISettings();

  ReadPSFSettings();
}


void
MicroscopeSimulator
::ReadPSFSettings() {
  // Read PSF list XML file
  QString psfSettingsFileName(m_Preferences->GetDataDirectoryPath().c_str());
  psfSettingsFileName.append(QDir::separator()).append("PSFList.xml");
  xmlDocPtr doc = xmlReadFile(psfSettingsFileName.toStdString().c_str(),
                              "ISO-8859-1", XML_PARSE_RECOVER);
  if (!doc) {
    return;
  }
  
  xmlNodePtr rootNode = xmlDocGetRootElement(doc);
  
  // Restore the simulation from the XML tree.
  m_Simulation->GetFluorescenceSimulation()->GetPSFList()->
    RestoreFromXML(rootNode);
  
  xmlFreeDoc(doc);
}


void
MicroscopeSimulator
::closeEvent(QCloseEvent* event) {
  Exit();

  // If we made it past the call above, the user clicked cancel.
  event->ignore();
}
