#include <MicroscopeSimulator.h>
#include <Simulation.h>
#include <Visualization.h>
#include <Version.h>
#include <FluorescenceSimulation.h>
#include <FluorophoreModelObjectProperty.h>
#include <PointSpreadFunctionList.h>


#if defined(_WIN32) // Turn off deprecation warnings in Visual Studio
#pragma warning( disable : 4996 )
#endif

#include <QActionGroup>
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QPSFListModel.h>
#include <QSettings>
#include <QVariant>

#include <ErrorLogDialog.h>
#include <FluorophoreModelDialog.h>
#include <PSFEditorDialog.h>
#include <Preferences.h>
#include <PreferencesDialog.h>

#include <vtkActor.h>
#include <vtkAlgorithmOutput.h>
#include <vtkBMPWriter.h>
#include <vtkBYUWriter.h>
#include <vtkCamera.h>
#include <vtkContourFilter.h>
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

  m_Preferences = new Preferences();
  
  m_PreferencesDialog = new PreferencesDialog(this, m_Preferences);
  m_PreferencesDialog->setModal(true);

  // Restore inter-session GUI settings.
  ReadProgramSettings();

  RefreshUI();
  RefreshModelObjectViews();
  on_actionResetCamera_triggered();
  gui->modelObjectQvtkWidget->GetRenderWindow()->Render();
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
  delete m_PSFEditorDialog;
 
  delete m_Preferences;
  delete m_PreferencesDialog;
 
  delete m_Visualization;
  delete m_ViewModeActionGroup;
  delete m_InteractionActionGroup;
  delete m_ModelObjectPropertyListTableModel;
  delete m_ModelObjectListSelectionModel;

  delete gui;
}


void
MicroscopeSimulator
::on_actionNewSimulation_triggered() {
  NewSimulation();

  SetStatusMessage("Created new simulation.");

  RefreshModelObjectViews();
  RefreshUI();
}


void
MicroscopeSimulator
::on_actionOpenSimulation_triggered() {
  NewSimulation();
  
  // Prompt for simulation file to open.
  QString selectedFileName = 
      QFileDialog::getOpenFileName(this, "Open Simulation File", "", "XML Files (*.xml);;");
  if (selectedFileName == "") {
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
  } else {
    // Get a file name.
    QString selectedFileName = 
      QFileDialog::getSaveFileName(this, "Save Simulation File", "", "XML Files (*.xml);;");

    // Now read the file
    if (selectedFileName == "") {
      return;
    }
    fileName = selectedFileName.toStdString();
  }

  SaveSimulationFile(fileName);
}


void
MicroscopeSimulator
::on_actionSaveSimulationAs_triggered() {
  // Always get a new file name.
  QString selectedFileName = 
    QFileDialog::getSaveFileName(this, "Save Simulation File As", "", "XML Files (*.xml);;");

  // Now read the file
  if (selectedFileName == "") {
    return;
  }
  
  SaveSimulationFile(selectedFileName.toStdString());
}


void
MicroscopeSimulator
::on_actionOpenImage_triggered() {
  // Ask if user really wants to quit if the simulation has been modified.
  if (m_SimulationNeedsSaving) {
    int selected = PromptToSaveChanges();
    if (selected == QMessageBox::Cancel)
      return;
  }

  // Locate file.
  QString fileName = QFileDialog::getOpenFileName(this, "Open Image Data", "", "TIF Images (*.tif);;VTK Images (*.vtk);;LSM Images (*.lsm);;");

  // Now read the file
  if (fileName == "") {
    return;
  }

  OpenSimulationFile(fileName.toStdString());

  // Set up visualization pipeline.

  // Refresh the UI
  RefreshUI();

  // Reset camera
  m_ModelObjectRenderer->ResetCamera();
  
  // Render
  //this->modelObjectQvtkWidget->GetRenderWindow()->Render();
}


void
MicroscopeSimulator
::NewSimulation() {
  // Ask if user really wants to quit if the simulation has been modified.
  if (m_SimulationNeedsSaving) {
    if (PromptToSaveChanges() == QMessageBox::Cancel)
      return;
  }

  m_ModelObjectListModel->SetModelObjectList(NULL);
  m_ModelObjectPropertyListTableModel->SetModelObject(NULL);

  PointSpreadFunctionList* psfList = m_Simulation->GetFluorescenceSimulation()->GetPSFList();

  // Delete the old Simulation and replace with a new one.
  delete m_Simulation;
  m_Simulation = new Simulation(this);
  m_Visualization->SetSimulation(m_Simulation);

  // Set PSFList in the PSFMenuListModel to the previous one.
  m_Simulation->GetFluorescenceSimulation()->SetPSFList(psfList);

  m_PSFMenuListModel->SetPSFList(m_Simulation->GetFluorescenceSimulation()->GetPSFList());
  m_ModelObjectListModel->SetModelObjectList(m_Simulation->GetModelObjectList());  
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
    
    RefreshModelObjectViews();
    RefreshUI();
  }
}


void
MicroscopeSimulator
::SaveSimulationFile(const std::string& fileName) {
  QString message = tr("Saved image '").append(fileName.c_str()).append("'.");
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
::on_actionExit_triggered() {
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
::on_actionAddCylinder_triggered() {
  m_Simulation->AddNewModelObject("CylinderModel");

  RefreshModelObjectViews();
}


void
MicroscopeSimulator
::on_actionAddHollowCylinder_triggered() {
  m_Simulation->AddNewModelObject("HollowCylinderModel");

  RefreshModelObjectViews();
}


void
MicroscopeSimulator
::on_actionAddDisk_triggered() {
  m_Simulation->AddNewModelObject("DiskModel");
  
  RefreshModelObjectViews();
}


void
MicroscopeSimulator
::on_actionAddFlexibleTube_triggered() {
  m_Simulation->AddNewModelObject("FlexibleTubeModel");

  RefreshModelObjectViews();
}


void
MicroscopeSimulator
::on_actionAddPlane_triggered() {
  m_Simulation->AddNewModelObject("PlaneModel");

  RefreshModelObjectViews();
}


void
MicroscopeSimulator
::on_actionAddPointSet_triggered() {
  m_Simulation->AddNewModelObject("PointSetModel");

  RefreshModelObjectViews();
}


void
MicroscopeSimulator
::on_actionAddSphere_triggered() {
  m_Simulation->AddNewModelObject("SphereModel");

  RefreshModelObjectViews();
}


void
MicroscopeSimulator
::on_actionAddTorus_triggered() {
  m_Simulation->AddNewModelObject("TorusModel");

  RefreshModelObjectViews();
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
  // Get a file name.
  QString selectedFileName = 
    QFileDialog::getSaveFileName(this, "Save Image File", "", "PNG Files (*.png);;");

  if (selectedFileName.toStdString() != "") {
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

}


void
MicroscopeSimulator
::on_actionViewModelAndPoints_triggered() {

}


void
MicroscopeSimulator
::on_actionViewModelAndScan_triggered(){

}


void
MicroscopeSimulator
::on_actionViewAFMScanOnly_triggered() {

}


void
MicroscopeSimulator
::on_actionViewModelsWithFluorescenceComparison_triggered() {

}


void
MicroscopeSimulator
::on_actionViewFluorescenceComparisonOnly_triggered() {

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
    QFileDialog::getSaveFileName(this, "Export Model Geometry", "", "VTK File (*.vtk);;PLY File (*.ply);;BYU File (*.byu)");
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
  }
  m_ModelObjectPropertyListTableModel->SetModelObject(NULL);
  m_ModelObjectPropertyListTableModel->Refresh();
}


void
MicroscopeSimulator
::handle_ModelObjectListSelectionModel_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
  QModelIndexList indexes = selected.indexes();
  if (!indexes.isEmpty() && indexes[0].row() >= 0) {
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
}


void
MicroscopeSimulator
::on_experimentDescriptionTextArea_textChanged() {
  QString text = gui->experimentDescriptionTextArea->toPlainText();
  m_Simulation->SetSimulationDescription(text.toStdString());
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
  QFileDialog fileDialog(this, tr("Export Fluorescence Image"), tr(""),
                         "PNG File (*.png);;BMP File (*.bmp);;JPG File (*.jpg);;16-bit TIFF File (*.tif)");
  fileDialog.setDefaultSuffix(tr("png"));
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);

  if (fileDialog.exec() == QDialog::Rejected)
    return;

  QString selectedFileName = fileDialog.selectedFiles()[0];
  if (selectedFileName.isEmpty())
    return;

  vtkImageData* image = m_Visualization->GetFluorescenceImage();

  vtkSmartPointer<vtkImageShiftScale> scaler = vtkSmartPointer<vtkImageShiftScale>::New();
  scaler->SetOutputScalarTypeToUnsignedChar();
  scaler->ClampOverflowOn();
  scaler->SetInput(image);
  double minIntensity = m_Simulation->GetFluorescenceSimulation()->GetMinimumIntensityLevel();
  double maxIntensity = m_Simulation->GetFluorescenceSimulation()->GetMaximumIntensityLevel();
  scaler->SetShift(-minIntensity);
  scaler->SetScale(255.0 / (maxIntensity - minIntensity));

  vtkSmartPointer<vtkImageWriter> writer;
  QString extension = QFileInfo(selectedFileName).suffix().toLower();
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

  writer->SetInputConnection(scaler->GetOutputPort());
  writer->SetFileName(selectedFileName.toStdString().c_str());
  writer->Write();
}


void
MicroscopeSimulator
::on_fluoroSimExportStackButton_clicked() {

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
  m_ModelObjectListModel->Refresh();
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

  // Refresh FluoroSim UI widgets
  FluorescenceSimulation* fluoroSim = m_Simulation->GetFluorescenceSimulation();

  // Focus group box
  UpdateFocalPlaneUIControls(fluoroSim->GetFocalPlaneDepthMinimum(),
                             fluoroSim->GetFocalPlaneDepthMaximum(),
                             fluoroSim->GetFocalPlaneDepthSpacing());
  int minValue = fluoroSim->GetFocalPlaneDepthMinimum();
  int maxValue = fluoroSim->GetFocalPlaneDepthMaximum();
  int focusSliderValue = (int) ((fluoroSim->GetFocalPlaneDepth() / fluoroSim->GetFocalPlaneDepthSpacing()));
  gui->fluoroSimFocusSlider->setValue(focusSliderValue);

  // Simulator Settings group box
  gui->fluoroSimExposureEdit->setText(QVariant(fluoroSim->GetExposure()).toString());
  gui->fluoroSimPixelSizeEdit->setText(QVariant(fluoroSim->GetPixelSize()).toString());
  gui->fluoroSimImageWidthEdit->setText(QVariant(fluoroSim->GetImageWidth()).toString());
  gui->fluoroSimImageHeightEdit->setText(QVariant(fluoroSim->GetImageHeight()).toString());
  gui->fluoroSimAddGaussianNoiseCheckBox->setChecked(QVariant(fluoroSim->GetAddGaussianNoise()).toBool());
  gui->fluoroSimStdDevEdit->setText(QVariant(fluoroSim->GetNoiseStdDev()).toString());
  gui->fluoroSimMeanEdit->setText(QVariant(fluoroSim->GetNoiseMean()).toString());

  // Fluorescence Display group box
  gui->fluoroSimShowReferencePlaneCheckBox->setChecked(QVariant(fluoroSim->GetShowReferencePlane()).toBool());
  gui->fluoroSimShowGridCheckBox->setChecked(QVariant(fluoroSim->GetShowReferenceGrid()).toBool());
  gui->fluoroSimGridSpacingEdit->setText(QVariant(fluoroSim->GetReferenceGridSpacing()).toString());
  gui->fluoroSimSuperimposeFluorescenceImageCheckBox->setChecked(QVariant(fluoroSim->GetSuperimposeFluorescenceImage()).toBool());
  gui->fluoroSimShowImageVolumeOutlineCheckBox->setChecked(QVariant(fluoroSim->GetShowImageVolumeOutline()).toBool());
  //gui->fluoroSimMinLevelEdit->setText(QVariant(fluoroSim->GetMinimumIntensityLevel()).toString());
  gui->fluoroSimMinLevelSlider->setValue((int) fluoroSim->GetMinimumIntensityLevel());
  //gui->fluoroSimMaxLevelEdit->setText(QVariant(fluoroSim->GetMaximumIntensityLevel()).toString());
  gui->fluoroSimMaxLevelSlider->setValue((int) fluoroSim->GetMaximumIntensityLevel());

  RenderViews();
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
::WriteProgramSettings() {
  QSettings settings;

  // Save size and position of the main window.
  settings.beginGroup("MainWindow");
  settings.setValue("size", this->size());
  settings.setValue("pos", this->pos());
  settings.endGroup();

  // Save geometry and docking info of dock widgets
  QList<QDockWidget*> widgets = this->findChildren<QDockWidget*>();
  QListIterator<QDockWidget*> iterator(widgets);
  while (iterator.hasNext()) {
    QDockWidget* dockWidget = iterator.next();
    settings.beginGroup(dockWidget->objectName());
    settings.setValue("size", dockWidget->size());
    settings.setValue("pos", dockWidget->pos());
    settings.setValue("visible", dockWidget->isVisible());
    settings.setValue("floating", dockWidget->isFloating());
    settings.setValue("dockArea", this->dockWidgetArea(dockWidget));
    settings.endGroup();
  }

  // Save PSF editor geometry
  settings.beginGroup("PSFEditorDialog");
  settings.setValue("size", m_PSFEditorDialog->size());
  settings.setValue("pos", m_PSFEditorDialog->pos());

  QList<QVariant> windowSplitterSizes = 
    m_PSFEditorDialog->GetWindowSplitterSizes();
  settings.setValue("windowSplitterSizes", windowSplitterSizes);

  settings.endGroup();

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
  int rc = xmlSaveFileEnc(psfSettingsFileName.toStdString().c_str(), doc, "ISO-8859-1");
  xmlFreeDoc(doc);

  settings.endGroup();
}


void
MicroscopeSimulator
::ReadProgramSettings() {
  QSettings settings;

  // Read main window settings
  settings.beginGroup("MainWindow");
  this->resize(settings.value("size", QSize(1000, 743)).toSize());
  this->move(settings.value("pos", QPoint(0, 20)).toPoint());
  settings.endGroup();

  // Read geometry from the docks
  QList<QDockWidget*> widgets = this->findChildren<QDockWidget*>();
  QListIterator<QDockWidget*> iterator(widgets);
  while (iterator.hasNext()) {
    QDockWidget* dockWidget = iterator.next();
    settings.beginGroup(dockWidget->objectName());
    dockWidget->resize(settings.value("size", QSize(340, 200)).toSize());
    dockWidget->move(settings.value("pos", QPoint(0, 0)).toPoint());
    dockWidget->setVisible(settings.value("visible", true).toBool());
    dockWidget->setFloating(settings.value("floating", false).toBool());
    this->addDockWidget(static_cast<Qt::DockWidgetArea>(settings.value("dockArea", Qt::LeftDockWidgetArea).toUInt()), dockWidget);
    settings.endGroup();
  }

  // Read PSF editor geometry
  settings.beginGroup("PSFEditorDialog");
  m_PSFEditorDialog->
    resize(settings.value("size", QSize(640, 480)).toSize());
  m_PSFEditorDialog->
    move(settings.value("pos", QPoint(0, 0)).toPoint());

  QList<QVariant> defaultWindowSplitterSizes;
  defaultWindowSplitterSizes.push_back(QVariant(300));
  defaultWindowSplitterSizes.push_back(QVariant(340));
  m_PSFEditorDialog->SetWindowSplitterSizes
    (settings.value("windowSplitterSizes", defaultWindowSplitterSizes).toList());

  settings.endGroup();

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
