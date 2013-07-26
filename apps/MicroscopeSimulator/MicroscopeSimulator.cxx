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
#include <QByteArray>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QImageListModel.h>
#include <QItemEditorFactory>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QProcess>
#include <QPSFListModel.h>
#include <QSettings>
#include <QStandardItemEditorCreator>
#include <QStringList>
#include <QStringListIterator>
#include <QVariant>

#include <ErrorLogDialog.h>
#include <FluorescenceImageSource.h>
#include <FluorophoreModelDialog.h>
#include <FocalPlanePositionsDialog.h>
#include <ImageExportOptionsDialog.h>
#include <ModelObjectList.h>
#include "ModelObjectProperty.h"
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
#include <vtkMath.h>
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
#include <vtkXMLPolyDataWriter.h>

#include <vtkFramebufferObjectRenderer.h>
#include <vtkFramebufferObjectTexture.h>


// Constructor
MicroscopeSimulator
::MicroscopeSimulator(int argc, char* argv[], QWidget* p)
  : QMainWindow(p), m_ModelObjectPropertyListTableModel(NULL) {

  SetBatchMode(false); // Assume interactive mode by default

  gui = new Ui_MainWindow();
  gui->setupUi(this);

  // Turn off AFMSim tab for this release. It is not implemented yet.
  gui->controlPanelTabs->removePage(gui->AFMSimTab);

  // Set up error dialog box.
  m_ErrorDialog.setModal(true);

  // Check the capabilities of OpenGL on this system
  CheckOpenGLCapabilities();

  // Change the double item editor to QLineEdit
  QItemEditorFactory* factory = new QItemEditorFactory();
  factory->registerEditor(QVariant::Int,    new QStandardItemEditorCreator<QLineEdit>());
  factory->registerEditor(QVariant::Double, new QStandardItemEditorCreator<QLineEdit>());
  factory->registerEditor(QVariant::String, new QStandardItemEditorCreator<QLineEdit>());
  QItemEditorFactory::setDefaultFactory(factory);

  // Instantiate visualization pipelines.
  m_Visualization = new Visualization();

  QSettings prefs;
  prefs.beginGroup("OpenGLCapabilities");
  bool fp32Blending = prefs.value("32BitFloatingPointBlend", false).toBool();
  if (fp32Blending) {
    m_Visualization->SetBlendingTo32Bit();
  } else {
    m_Visualization->SetBlendingTo16Bit();
  }
  prefs.endGroup();

  // Need to manually set the interactor to the QVTK widget's interactor.
  // Otherwise, the default interactor may be used. On Mac with Carbon,
  // the default interactor sucks up all the keypress events, making it
  // impossible to type anything in Qt widgets.
  m_Visualization->GetModelObjectRenderWindow()->
    SetInteractor(gui->modelObjectQvtkWidget->GetInteractor());
  gui->modelObjectQvtkWidget->SetRenderWindow(m_Visualization->GetModelObjectRenderWindow());

  // Set line smoothing on
  m_Visualization->GetModelObjectRenderWindow()->LineSmoothingOn();

  gui->fluorescenceQvtkWidget->SetRenderWindow(m_Visualization->GetFluorescenceRenderWindow());
  gui->fluorescenceQvtkWidget->setMaximumSize(200, 200);
  gui->fluorescenceQvtkWidget->setMinimumSize(200, 200);
  gui->fluorescenceQvtkWidget->setHidden(true);
  gui->fluorescenceBackgroundWidget->setMaximumHeight(200);

  // Instantiate data model.
  m_SimulationNeedsSaving  = false;
  m_Simulation = new Simulation(this);

  m_Visualization->SetSimulation(m_Simulation);

  m_PSFMenuListModel = new QPSFListModel();
  m_PSFMenuListModel->SetHasNone(true);
  m_PSFMenuListModel->SetPSFList(m_Simulation->GetFluorescenceSimulation()->GetPSFList());
  gui->fluoroSimPSFMenuComboBox->setModel(m_PSFMenuListModel);

  m_ImageListModel = new QImageListModel();
  m_ImageListModel->SetModelObjectList(m_Simulation->GetModelObjectList());
  gui->fluoroSimComparisonImageComboBox->setModel(m_ImageListModel);

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

  m_FocalPlanePositionsDialog = new FocalPlanePositionsDialog();
  m_FocalPlanePositionsDialog->setModal(true);
  m_FocalPlanePositionsDialog->
    SetFluorescenceSimulation(m_Simulation->GetFluorescenceSimulation());

  m_OptimizerSettingsDialog = new OptimizerSettingsDialog();
  m_OptimizerSettingsDialog->setModal(true);
  m_OptimizerSettingsDialog->SetFluorescenceOptimizer(m_Simulation->GetFluorescenceOptimizer());

  m_Preferences = new Preferences();

  m_PreferencesDialog = new PreferencesDialog(this, this, m_Preferences);
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
  delete m_OptimizerSettingsDialog;
  delete m_FocalPlanePositionsDialog;
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
::CheckOpenGLCapabilities() {
  QSettings prefs;
  prefs.beginGroup("OpenGLCapabilities");

  if (!prefs.value("Checked", false).toBool()) {

    QMessageBox message(QMessageBox::Information,
                        "Checking Graphics Card Support",
                        "Please wait a moment while Microscope Simulator "
                        "determines the capabilities of your graphics card.",
                        QMessageBox::NoButton, this);
    message.show();
    message.raise();
    qApp->processEvents(); // makes sure the message box is repainted

    // Run the GLCheck program to see what features are supported by the GPU
    QString appName = QCoreApplication::applicationDirPath();
    appName.append("/GLCheck");

    // Now parse the output of the test
    QStringList knownFeatureNames;
    knownFeatureNames
      << "RequiredExtensions"
      << "16BitFloatingPointBlend"
      << "32BitFloatingPointBlend"
      << "FloatingPointTextureTrilinearInterpolation"
      << "GLSLUnsignedInts";

    QStringListIterator iter(knownFeatureNames);
    while (iter.hasNext()) {
      QString featureName = iter.next();
      QStringList args;
      args << featureName;
      QProcess glCheckProcess;
      glCheckProcess.start(appName, args);
      if (glCheckProcess.waitForFinished(60000)) {
        QString output(glCheckProcess.readAllStandardOutput());

        QStringList outputColumns = output.split(" ");
        if (outputColumns.size() >= 2 && outputColumns[0] == featureName) {
          bool supported =
            outputColumns[1].indexOf(tr("PASSED"), 0, Qt::CaseInsensitive) > -1;
          prefs.setValue(featureName, supported);
          std::cout << featureName.toStdString() << " "
                    << (supported ? "supported" : "not supported") << std::endl;
        }
      } else {
        prefs.setValue(featureName, false);
      }
    }

    message.hide();
    qApp->processEvents(); // makes sure anything covered by the message box is repainted
  }
  prefs.endGroup();
  prefs.sync();
  prefs.beginGroup("OpenGLCapabilities");

  // Bail out with an error message if the user's OpenGL implementation
  // won't support the fluorescence simulator.
  bool extensionsSupported =
    prefs.value("RequiredExtensions", false).toBool();
  bool fp16BlendSupported =
    prefs.value("16BitFloatingPointBlend", false).toBool();
  bool fpTextureTrilerpSupported =
    prefs.value("FloatingPointTextureTrilinearInterpolation", false).toBool();
  bool glslUnsignedIntsSupported =
    prefs.value("GLSLUnsignedInts", false).toBool();
  if (!extensionsSupported || !fp16BlendSupported) {
    QMessageBox::critical
      (this, tr("Error"),
       tr("Your graphics card does not support the FluoroSim module. The "
          "Microscope Simulator will now exit."));
    qApp->exit();
    exit(-1);
  }
  if (!fpTextureTrilerpSupported) {
    QMessageBox::critical
      (this, tr("Warning"),
       tr("Your graphics card lacks the trilinear interpolation feature "
          "necessary to compute accurate images. You will not be able to "
          "use this program for quantitative analysis on this computer, "
          "but it may be useful for qualitative hypothesis testing."));
  }

  gui->fluoroSimNoiseGroupBox->setEnabled(glslUnsignedIntsSupported);
  gui->fluoroSimNoiseGroupBox->setToolTip("Disabled because your graphics card does not support noise generation.");

  // Mark that we've checked the OpenGL capabilities.
  prefs.setValue("Checked", true);
  prefs.endGroup();
}


// Process command-line arguments. This program is a little unusual
// in that arguments are treated as commands to execute. Arguments
// are processed in the order in which they appear on the command
// line.
void
MicroscopeSimulator
::ProcessCommandLineArguments(int argc, char* argv[]) {
  for (int i = 1; i < argc; i++) {

    if (strcmp(argv[i], "--batch-mode") == 0) {

      SetBatchMode(true);

    } else if (strcmp(argv[i], "--open-simulation") == 0) {

      i++;
      if (i < argc) {
        m_SimulationNeedsSaving = false; // Kind of a hack to prevent file save dialog box from appearing.
        NewSimulation();
        OpenSimulationFile(std::string(argv[i]));
      } else {
        std::cerr << "No simulation file provided for command --open-simulation" << std::endl;
        return;
      }

    } else if (strcmp(argv[i], "--save-simulation") == 0) {

      i++;
      if (i < argc) {
        SaveSimulationFile(std::string(argv[i]));
      } else {
        std::cerr << "No simulation file provided for command --save-simulation" << std::endl;
        return;
      }

    } else if (strcmp(argv[i], "--optimize-fluorescence") == 0) {

      m_Simulation->OptimizeToFluorescence();

    } else if (strcmp(argv[i], "--save-fluorescence-stack") == 0) {

      i++;

      bool exportRed   = false;
      bool exportGreen = false;
      bool exportBlue  = false;
      bool regenerateFluorophores   = false;
      bool randomizeObjectPositions = false;
      bool randomizeStagePosition   = false;
      double xRange = 0.0;
      double yRange = 0.0;
      double zRange = 0.0;
      int numberOfCopies = 1;

      // See if there are any other options for saving the stack
      bool doneParsingSaveOptions = false;
      while (!doneParsingSaveOptions) {
        if ( argv[i][0] == '-' && argv[i][1] == '-' ) {
          if (strcmp(argv[i], "--red") == 0) {
            exportRed = true;
            std::cout << "--red" << std::endl;
          } else if (strcmp(argv[i], "--green") == 0) {
            exportGreen = true;
            std::cout << "--green" << std::endl;
          } else if (strcmp(argv[i], "--blue") == 0) {
            exportBlue = true;
            std::cout << "--blue" << std::endl;
          } else if (strcmp(argv[i], "--regenerateFluorophores") == 0) {
            regenerateFluorophores = true;
            std::cout << "--regenerateFluorophores" << std::endl;
          } else if (strcmp(argv[i], "--randomizeObjectPositions") == 0) {
            randomizeObjectPositions = true;
            std::cout << "--randomizeObjectPositions" << std::endl;
          } else if (strcmp(argv[i], "--randomizeStagePosition") == 0) {
            randomizeStagePosition = true;
            std::cout << "--randomizeStagePosition" << std::endl;
          } else if (strcmp(argv[i], "--xrange") == 0) {
            if (i+1 < argc) {
              xRange = atof(argv[i+1]);
              std::cout << "--xrange " << argv[i+1] << std::endl;
              i++;
            } else {
              doneParsingSaveOptions = true;
            }
          } else if (strcmp(argv[i], "--yrange") == 0) {
            if (i+1 < argc) {
              yRange = atof(argv[i+1]);
              std::cout << "--yrange " << argv[i+1] << std::endl;
              i++;
            } else {
              doneParsingSaveOptions = true;
            }
          } else if (strcmp(argv[i], "--zrange") == 0) {
            if (i+1 < argc) {
              zRange = atof(argv[i+1]);
              std::cout << "--zrange " << argv[i+1] << std::endl;
              i++;
            } else {
              doneParsingSaveOptions = true;
            }
          } else if (strcmp(argv[i], "--numberOfCopies") == 0) {
            if (i+1 < argc) {
              numberOfCopies = atoi(argv[i+1]);
              std::cout << "--numberOfCopies " << argv[i+1] << std::endl;
              i++;
            } else {
              doneParsingSaveOptions = true;
            }
          } else {
            std::cerr << "Unknown --save-fluorescence-stack sub-option '" << argv[i] << "'" << std::endl;
          }
          i++;
        } else {
          doneParsingSaveOptions = true;
        }
      }
      if (i < argc) {
        m_Simulation->ExportFluorescenceStack(std::string(argv[i]),
          "tif", exportRed, exportGreen, exportBlue, regenerateFluorophores,
          randomizeObjectPositions, randomizeStagePosition, xRange, yRange, zRange,
          numberOfCopies);
      } else {
        std::cerr << "No stack name provided for command --save-fluorescence-stack" << std::endl;
      }

    } else if (strcmp(argv[i], "--save-fluorescence-objective-function-value") == 0) {

      i++;
      if (i < argc) {
        m_Simulation->SaveFluorescenceObjectiveFunctionValue(std::string(argv[i]));
      } else {
        std::cerr << "No file name provided for command --save-fluorescence-objective-function-value" << std::endl;
      }

    } else if (strcmp(argv[i], "--set-parameter") == 0 ||
               strcmp(argv[i], "--optimize-parameter") == 0) {

      if (i+1 >= argc) {
        std::cerr << "No model object name provided." << std::endl;
        continue;
      }

      if (i+2 >= argc) {
        std::cerr << "No parameter name provided." << std::endl;
        continue;
      }

      if (i+3 >= argc) {
        std::cerr << "No parameter value provided." << std::endl;
        continue;
      }

      ModelObject* mo = m_Simulation->GetModelObjectList()->GetModelObjectByName(argv[i+1]);
      if (!mo) {
        std::cerr << "No model object named '" << argv[i+1] << "' exists." << std::endl;
      } else {
        ModelObjectProperty* mop = mo->GetProperty(argv[i+2]);
        if (!mop) {
          std::cerr << "No model object parameter named '" << argv[i+2] << "' exists in model object '" << argv[i+1] << "'." << std::endl;
        } else {

          if (strcmp(argv[i+3], "--set-parameter") == 0) {
            switch (mop->GetType()) {
            case ModelObjectProperty::BOOL_TYPE:
              mop->SetBoolValue(strcmp(argv[i+3], "true") == 0);
              break;

            case ModelObjectProperty::INT_TYPE:
              mop->SetIntValue(atoi(argv[i+3]));
              break;

            case ModelObjectProperty::DOUBLE_TYPE:
              mop->SetDoubleValue(atoi(argv[i+3]));
              break;

            case ModelObjectProperty::STRING_TYPE:
              mop->SetStringValue(std::string(argv[i+3]));
              break;
            }
          } else {
            mop->SetOptimize(strcmp(argv[i+3], "true") == 0);
          }
        }
      }
    }
  }
}


void
MicroscopeSimulator
::SetBatchMode(bool mode) {
  m_BatchMode = mode;
}


bool
MicroscopeSimulator
::IsBatchMode() {
  return m_BatchMode;
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
  QSettings prefs;
  prefs.beginGroup("FileDialogs");
  QString directory = prefs.value("OpenSimulationDirectory").toString();

  QFileDialog fileDialog(this, "Open Simulation File", directory,
                         "XML Files (*.xml)");
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

  QString extension = QString('.').append(fileDialog.selectedNameFilter().right(4).left(3).toLower());
  if (!selectedFileName.endsWith(extension))
    selectedFileName.append(extension);

  // Now restore the Simulation from the XML tree.
  NewSimulation();
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
                         "XML Files (*.xml)");
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

  QString extension = QString('.').append(fileDialog.selectedNameFilter().right(4).left(3).toLower());
  if (!selectedFileName.endsWith(extension))
    selectedFileName.append(extension);

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
    m_FocalPlanePositionsDialog->Refresh();
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
::on_actionAddEllipsoid_triggered() {
  AddNewModelObject("EllipsoidModel");
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
                         "PNG Files (*.png)");
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  int result = fileDialog.exec();

  prefs.setValue("SaveImageDirectory", fileDialog.directory().absolutePath());
  prefs.endGroup();

  if (result == QDialog::Rejected)
    return;

  QString selectedFileName = fileDialog.selectedFiles()[0];
  if (selectedFileName.isEmpty())
    return;

  QString extension = QString('.').append(fileDialog.selectedNameFilter().right(4).left(3).toLower());
  if (!selectedFileName.endsWith(extension))
    selectedFileName.append(extension);

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
  bool enabled = gui->actionEnableDisableAxes->isChecked();
  m_Visualization->SetShowOrientationWidgetInModelObjectRenderer(enabled);
  m_Visualization->ModelObjectViewRender();
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

  QSettings prefs;
  prefs.beginGroup("FileDialogs");
  QString directory  = prefs.value("ExportGeometryDirectory").toString();
  QString nameFilter = prefs.value("ExportGeometryNameFilter").toString();

  QFileDialog fileDialog(this, tr("Export Model Geometry"), directory,
                         "VTK File (*.vtk);;VTK Poly Data File (*.vtp);;PLY File (*.ply);;BYU File (*.byu)");
  fileDialog.selectNameFilter(nameFilter);
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  int result = fileDialog.exec();
  prefs.setValue("ExportGeometryDirectory", fileDialog.directory().absolutePath());
  prefs.setValue("ExportGeometryNameFilter", fileDialog.selectedNameFilter());
  prefs.endGroup();

  if (result == QDialog::Rejected)
    return;

  QString selectedFileName = fileDialog.selectedFiles()[0];
  if (selectedFileName.isEmpty())
    return;

  QString extension = QString('.').append(fileDialog.selectedNameFilter().right(4).left(3).toLower());
  if (!selectedFileName.endsWith(extension))
    selectedFileName.append(extension);

  vtkPolyDataAlgorithm* geometrySource = object->GetAllGeometryTransformed();

  if (selectedFileName.endsWith(tr(".vtk"))) {
    vtkSmartPointer<vtkPolyDataWriter> writer = vtkSmartPointer<vtkPolyDataWriter>::New();
    writer->SetInputConnection(geometrySource->GetOutputPort());
    writer->SetFileName(selectedFileName.toStdString().c_str());
    writer->Update();
  } else if (selectedFileName.endsWith(tr(".vtp"))) {
    vtkSmartPointer<vtkXMLPolyDataWriter> vtpWriter =
      vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    vtpWriter->SetInputConnection(geometrySource->GetOutputPort());
    vtpWriter->SetFileName(selectedFileName.toStdString().c_str());
    vtpWriter->Update();
  } else if (selectedFileName.endsWith(tr(".ply"))) {
    vtkSmartPointer<vtkPLYWriter> plyWriter = vtkSmartPointer<vtkPLYWriter>::New();
    plyWriter->SetInputConnection(geometrySource->GetOutputPort());
    plyWriter->SetFileName(selectedFileName.toStdString().c_str());
    plyWriter->Update();
  } else if (selectedFileName.endsWith(tr(".byu"))) {
    vtkSmartPointer<vtkBYUWriter> byuWriter = vtkSmartPointer<vtkBYUWriter>::New();
    byuWriter->SetGeometryFileName(selectedFileName.toStdString().c_str());
    byuWriter->WriteDisplacementOff();
    byuWriter->WriteScalarOff();
    byuWriter->WriteTextureOff();
  } else {
    SetStatusMessage("Error: Could not export geometry");
  }

  geometrySource->Delete();

  QString message = QString().append(tr("Exported model geometry to file '")).
    append(selectedFileName.toStdString().c_str()).append(tr("'."));
  SetStatusMessage(message.toStdString());
}


void
MicroscopeSimulator
::on_actionDeleteModelObject_triggered() {
  // Get the model object to delete.
  int row = gui->fluoroSimModelObjectList->currentIndex().row();
  ModelObjectListPtr mol = m_Simulation->GetModelObjectList();
  ModelObject* mo = NULL;
  if (row >= 0 && row < static_cast<int>(mol->GetSize())) {
    mo = mol->GetModelObjectAtIndex(row);
  }

  if (mo == NULL)
    return;

  QString objectName(mo->GetName().c_str());

  // Get confirmation first.
  QString message = QString("Really delete model object '").
    append(objectName).append("'?");
  QMessageBox messageBox;
  messageBox.setText(message);
  messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  messageBox.setDefaultButton(QMessageBox::No);

  int selected = messageBox.exec();
  if (selected == QMessageBox::No)
    return;

  mol->Delete(mo);

  RefreshModelObjectViews();
  m_ModelObjectListModel->Refresh();
  m_ModelObjectPropertyListTableModel->SetModelObject(NULL);
  m_ModelObjectPropertyListTableModel->Refresh();
  m_ImageListModel->Refresh();

  QString statusMessage = QString().append(tr("Deleted model object '")).
    append(objectName).append(tr("'."));
  SetStatusMessage(statusMessage.toStdString());
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
  FluorescenceSimulation* fluoroSim = m_Simulation->GetFluorescenceSimulation();
  fluoroSim->SetFocalPlaneIndex(static_cast<unsigned int>(value-1));

  // Set text in fluoroSimFocusEdit to slider value
  gui->fluoroSimCurrentPlaneEdit->setText(QVariant(value).toString());

  QString focalDepth = QVariant(fluoroSim->GetFocalPlanePosition()).toString();
  focalDepth.append(" nm");
  gui->fluoroSimFocalDepthEdit->setText(focalDepth);

  RenderViews();

  Sully();
}


void
MicroscopeSimulator
::on_fluoroSimCurrentPlaneEdit_editingFinished() {
  unsigned int plane = static_cast<unsigned int>
    (gui->fluoroSimCurrentPlaneEdit->text().toInt()) - 1;
  if (plane >= 0 && plane < m_Simulation->GetFluorescenceSimulation()->GetNumberOfFocalPlanes()) {
    m_Simulation->GetFluorescenceSimulation()->SetFocalPlaneIndex(plane);
  }
  RefreshUI();
}


void
MicroscopeSimulator
::on_fluoroSimNumberOfFocalPlanesEdit_editingFinished() {
  unsigned int planes = static_cast<unsigned int>
    (gui->fluoroSimNumberOfFocalPlanesEdit->text().toInt());
  m_Simulation->GetFluorescenceSimulation()->SetNumberOfFocalPlanes(planes);

  gui->fluoroSimFocusSlider->setMaximum(planes);

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimFocalPlaneSpacingEdit_editingFinished() {
  m_Simulation->GetFluorescenceSimulation()->SetFocalPlaneSpacing
    (gui->fluoroSimFocalPlaneSpacingEdit->text().toDouble());

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimUseCustomFocalPlanePositionsCheckBox_toggled(bool value) {
  m_Simulation->GetFluorescenceSimulation()->SetUseCustomFocalPlanePositions(value);

  RefreshUI();
  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimEditCustomFocalPlanePositionsButton_clicked() {
  if (m_FocalPlanePositionsDialog->exec() == QDialog::Rejected) {
    return;
  }

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
::on_fluoroSimGainEdit_textChanged(QString text) {
  double gain = text.toDouble();
  m_Simulation->GetFluorescenceSimulation()->SetGain(gain);

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimOffsetEdit_textChanged(QString text) {
  double offset = text.toDouble();
  m_Simulation->GetFluorescenceSimulation()->SetOffset(offset);

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimMaximumVoxelIntensityEdit_editingFinished() {
  double maxIntensity = gui->fluoroSimMaximumVoxelIntensityEdit->text().toDouble();
  m_Simulation->GetFluorescenceSimulation()->SetMaximumVoxelIntensity(maxIntensity);
}


void
MicroscopeSimulator
::on_fluoroSimUpdateIntensitySettingsButton_clicked() {

  // Get the scalar range of the 3D image
  double scalarRange[2];
  m_Visualization->Get3DFluorescenceImageScalarRange(scalarRange);
  double currentMaxIntensity = scalarRange[1];

  // Subtract off the offset from the current and desired maximum intensity value
  FluorescenceSimulation* fluoroSim = m_Simulation->GetFluorescenceSimulation();
  double desiredMaxIntensity = fluoroSim->GetMaximumVoxelIntensity();
  desiredMaxIntensity -= fluoroSim->GetOffset();
  currentMaxIntensity -= fluoroSim->GetOffset();

  // Figure out the gain scaling factor and apply it
  double scale = desiredMaxIntensity / currentMaxIntensity;

  fluoroSim->SetGain(scale*fluoroSim->GetGain());

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
::on_fluoroSimShearInXEdit_editingFinished() {
  double shearInX = gui->fluoroSimShearInXEdit->text().toDouble();
  m_Simulation->GetFluorescenceSimulation()->SetShearInX(shearInX);

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimShearInYEdit_editingFinished() {
  double shearInY = gui->fluoroSimShearInYEdit->text().toDouble();
  m_Simulation->GetFluorescenceSimulation()->SetShearInY(shearInY);

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
::on_fluoroSimShowGridCheckBox_toggled(bool show) {
  m_Simulation->GetFluorescenceSimulation()->SetShowReferenceGrid(show);

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimSuperimposeSimulatedImageCheckBox_toggled(bool show) {
  m_Simulation->GetFluorescenceSimulation()->SetSuperimposeSimulatedImage(show);

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimSuperimposeComparisonImageCheckBox_toggled(bool show) {
  m_Simulation->GetFluorescenceSimulation()->SetSuperimposeComparisonImage(show);

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

  RefreshUI();
}


void
MicroscopeSimulator
::on_fluoroSimMinLevelSlider_sliderMoved(int value) {
  double dValue = static_cast<double>(value);
  m_Simulation->GetFluorescenceSimulation()->SetMinimumIntensityLevel(dValue);
  m_Visualization->FluorescenceViewRender();

  RefreshUI();
}


void
MicroscopeSimulator
::on_fluoroSimMaxLevelEdit_editingFinished() {
  double level = gui->fluoroSimMaxLevelEdit->text().toDouble();
  m_Simulation->GetFluorescenceSimulation()->SetMaximumIntensityLevel(level);
  m_Visualization->FluorescenceViewRender();

  RefreshUI();
}


void
MicroscopeSimulator
::on_fluoroSimMaxLevelSlider_sliderMoved(int value) {
  double dValue = static_cast<double>(value);
  m_Simulation->GetFluorescenceSimulation()->SetMaximumIntensityLevel(dValue);
  m_Visualization->FluorescenceViewRender();

  RefreshUI();
}


void
MicroscopeSimulator
::on_fluoroSimSetToFullIntensityRange_clicked() {
  m_Visualization->FluorescenceViewRender();
  double scalarRange[2];
  m_Visualization->Get2DFluorescenceImageScalarRange(scalarRange);

  m_Simulation->GetFluorescenceSimulation()->
    SetMinimumIntensityLevel(scalarRange[0]);
  m_Simulation->GetFluorescenceSimulation()->
    SetMaximumIntensityLevel(scalarRange[1]);

  RefreshUI();
}


void
MicroscopeSimulator
::on_fluoroSimRegenerateFluorophores_clicked() {
  m_Simulation->RegenerateFluorophores();

  RenderViews();
}


void
MicroscopeSimulator
::on_fluoroSimExportImageButton_clicked() {
  QSettings prefs;
  prefs.beginGroup("FileDialogs");
  QString directory  = prefs.value("ExportImageDirectory").toString();
  QString nameFilter = prefs.value("ExportImageNameFilter").toString();

  int result;

  // Now get the options for the export
  result = m_ImageExportOptionsDialog->exec();
  if (result == QDialog::Rejected)
    return;

  QFileDialog fileDialog(this, tr("Export Fluorescence Image"), directory,
                         "PNG File (*.png);;BMP File (*.bmp);;JPG File (*.jpg);;16-bit TIFF File (*.tif)");
  fileDialog.selectNameFilter(nameFilter);
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  if (fileDialog.exec() == QDialog::Rejected)
    return;
  prefs.setValue("ExportImageDirectory", fileDialog.directory().absolutePath());
  prefs.setValue("ExportImageNameFilter", fileDialog.selectedNameFilter());
  prefs.endGroup();

  QString selectedFileName = fileDialog.selectedFiles()[0];
  if (selectedFileName.isEmpty())
    return;

  QString extension = QString().append(fileDialog.selectedNameFilter().right(4).left(3).toLower());
  if (selectedFileName.endsWith(QString('.').append(extension)))
    selectedFileName.chop(extension.size() + 1);

  // Save the original object positions
  std::vector< double > originalPositions;
  for (unsigned int i = 0; i < m_Simulation->GetModelObjectList()->GetSize(); i++) {
    ModelObject* mo = m_Simulation->GetModelObjectList()->GetModelObjectAtIndex(i);
    if (mo->GetProperty(ModelObject::X_POSITION_PROP)) {
      originalPositions.push_back(mo->GetProperty(ModelObject::X_POSITION_PROP)->GetDoubleValue());
      originalPositions.push_back(mo->GetProperty(ModelObject::Y_POSITION_PROP)->GetDoubleValue());
      originalPositions.push_back(mo->GetProperty(ModelObject::Z_POSITION_PROP)->GetDoubleValue());
    }
  }

  int numberOfImages = m_ImageExportOptionsDialog->GetNumberOfCopies();
  if (numberOfImages <= 0) numberOfImages = 1;

  for (int i = 0; i < numberOfImages; i++) {

    if (m_ImageExportOptionsDialog->IsRegenerateFluorophoresEnabled()) {
      m_Simulation->RegenerateFluorophores();
    }

    double xRange = m_ImageExportOptionsDialog->GetObjectRandomPositionRangeX();
    double yRange = m_ImageExportOptionsDialog->GetObjectRandomPositionRangeY();
    double zRange = m_ImageExportOptionsDialog->GetObjectRandomPositionRangeZ();

    if (m_ImageExportOptionsDialog->IsRandomizeObjectPositionsEnabled()) {
      // Randomize each object's position by a different offset.
      unsigned int mIndex = 0;
      for (unsigned int mi = 0; mi < m_Simulation->GetModelObjectList()->GetSize(); mi++) {
        ModelObject* mo = m_Simulation->GetModelObjectList()->GetModelObjectAtIndex(mi);
        if (!mo->GetProperty(ModelObject::X_POSITION_PROP))
          continue;

        double newX = originalPositions[mIndex++] + xRange * (vtkMath::Random() - 0.5);
        double newY = originalPositions[mIndex++] + yRange * (vtkMath::Random() - 0.5);
        double newZ = originalPositions[mIndex++] + zRange * (vtkMath::Random() - 0.5);

        mo->GetProperty(ModelObject::X_POSITION_PROP)->SetDoubleValue(newX);
        mo->GetProperty(ModelObject::Y_POSITION_PROP)->SetDoubleValue(newY);
        mo->GetProperty(ModelObject::Z_POSITION_PROP)->SetDoubleValue(newZ);
      }
    }

    if (m_ImageExportOptionsDialog->IsRandomizeStagePositionEnabled()) {
      // Randomize all object's positions by the same random offset.
      double offsetX = xRange * (vtkMath::Random() - 0.5);
      double offsetY = yRange * (vtkMath::Random() - 0.5);
      double offsetZ = zRange * (vtkMath::Random() - 0.5);

      for (unsigned int mi = 0; mi < m_Simulation->GetModelObjectList()->GetSize(); mi++) {
        ModelObject* mo = m_Simulation->GetModelObjectList()->GetModelObjectAtIndex(mi);
        if (!mo->GetProperty(ModelObject::X_POSITION_PROP))
          continue;
        double x = mo->GetProperty(ModelObject::X_POSITION_PROP)->GetDoubleValue() + offsetX;
        double y = mo->GetProperty(ModelObject::Y_POSITION_PROP)->GetDoubleValue() + offsetY;
        double z = mo->GetProperty(ModelObject::Z_POSITION_PROP)->GetDoubleValue() + offsetZ;
        mo->GetProperty(ModelObject::X_POSITION_PROP)->SetDoubleValue(x);
        mo->GetProperty(ModelObject::Y_POSITION_PROP)->SetDoubleValue(y);
        mo->GetProperty(ModelObject::Z_POSITION_PROP)->SetDoubleValue(z);
      }
    }

    vtkImageData* image = m_Visualization->GenerateFluorescenceImage();

    vtkSmartPointer<vtkImageShiftScale> scaler = vtkSmartPointer<vtkImageShiftScale>::New();
    scaler->SetOutputScalarTypeToUnsignedChar();
    scaler->ClampOverflowOn();
    scaler->SetInputData(image);
    image->Delete();

    double minIntensity = m_Simulation->GetFluorescenceSimulation()->GetMinimumIntensityLevel();
    double maxIntensity = m_Simulation->GetFluorescenceSimulation()->GetMaximumIntensityLevel();
    scaler->SetShift(-minIntensity);
    scaler->SetScale(255.0 / (maxIntensity - minIntensity));

    vtkSmartPointer<vtkImageWriter> writer;
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
      QString fileName;
      vtkSmartPointer<vtkImageExtractComponents> extractor =
        vtkSmartPointer<vtkImageExtractComponents>::New();
      extractor->SetInputConnection(scaler->GetOutputPort());
      writer->SetInputConnection(extractor->GetOutputPort());

      if (m_ImageExportOptionsDialog->IsExportRedEnabled()) {
        fileName.sprintf("%s%04d_R.%s", selectedFileName.toStdString().c_str(),
                         i, extension.toStdString().c_str());
        extractor->SetComponents(0);
        writer->SetFileName(fileName.toStdString().c_str());
        writer->Write();
      }

      if (m_ImageExportOptionsDialog->IsExportGreenEnabled()) {
        fileName.sprintf("%s%04d_G.%s", selectedFileName.toStdString().c_str(),
                         i, extension.toStdString().c_str());
        extractor->SetComponents(1);
        writer->SetFileName(fileName.toStdString().c_str());
        writer->Write();
      }

      if (m_ImageExportOptionsDialog->IsExportBlueEnabled()) {
        fileName.sprintf("%s%04d_B.%s", selectedFileName.toStdString().c_str(),
                         i, extension.toStdString().c_str());
        extractor->SetComponents(2);
        writer->SetFileName(fileName.toStdString().c_str());
        writer->Write();
      }
    }
  }

  // Restore the original object positions
  unsigned int index = 0;
  for (unsigned int i = 0; i < m_Simulation->GetModelObjectList()->GetSize(); i++) {
    ModelObject* mo = m_Simulation->GetModelObjectList()->GetModelObjectAtIndex(i);
    if (mo->GetProperty(ModelObject::X_POSITION_PROP)) {
      mo->GetProperty(ModelObject::X_POSITION_PROP)->SetDoubleValue(originalPositions[index++]);
      mo->GetProperty(ModelObject::Y_POSITION_PROP)->SetDoubleValue(originalPositions[index++]);
      mo->GetProperty(ModelObject::Z_POSITION_PROP)->SetDoubleValue(originalPositions[index++]);
    }
  }

}


void
MicroscopeSimulator
::on_fluoroSimExportStackButton_clicked() {
  QSettings prefs;
  prefs.beginGroup("FileDialogs");
  QString directory  = prefs.value("ExportStackDirectory").toString();
  QString nameFilter = prefs.value("ExportStackNameFilter").toString();

  // Now get the options for the export
  if (m_ImageExportOptionsDialog->exec() == QDialog::Rejected)
    return;

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

  QString extension = QString().append(fileDialog.selectedNameFilter().right(4).left(3).toLower());
  if (selectedFileName.endsWith(QString('.').append(extension)))
    selectedFileName.chop(extension.size() + 1);

  bool exportRed   = m_ImageExportOptionsDialog->IsExportRedEnabled();
  bool exportGreen = m_ImageExportOptionsDialog->IsExportGreenEnabled();
  bool exportBlue  = m_ImageExportOptionsDialog->IsExportBlueEnabled();
  bool regenerateFluorophores = m_ImageExportOptionsDialog->
    IsRegenerateFluorophoresEnabled();
  bool randomizeObjectPositions = m_ImageExportOptionsDialog->
    IsRandomizeObjectPositionsEnabled();
  bool randomizeStagePosition = m_ImageExportOptionsDialog->
    IsRandomizeStagePositionEnabled();
  double xRange = m_ImageExportOptionsDialog->GetObjectRandomPositionRangeX();
  double yRange = m_ImageExportOptionsDialog->GetObjectRandomPositionRangeY();
  double zRange = m_ImageExportOptionsDialog->GetObjectRandomPositionRangeZ();
  int numberOfCopies = m_ImageExportOptionsDialog->GetNumberOfCopies();

  m_Simulation->ExportFluorescenceStack(
    selectedFileName.toStdString(), ".tif",
    exportRed, exportGreen, exportBlue,
    regenerateFluorophores, randomizeObjectPositions,
    randomizeStagePosition, xRange, yRange, zRange, numberOfCopies );

  ////

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
  sim->SetNumberOfFocalPlanes(dims[2]);

  if (sim->GetFocalPlaneIndex() >= sim->GetNumberOfFocalPlanes())
    sim->SetFocalPlaneIndex(sim->GetNumberOfFocalPlanes()-1);

  sim->SetPixelSize(spacing[0]);
  sim->SetFocalPlaneSpacing(spacing[2]);

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
  if (optimizer) {
    optimizer->SetObjectiveFunctionByName
      (gui->fluoroSimObjectiveFunctionComboBox->currentText().toStdString());
  }
}


void
MicroscopeSimulator
::on_fluoroSimOptimizeButton_clicked() {
  QMessageBox messageBox;

  if (m_Simulation->GetFluorescenceOptimizer()->GetNumberOfParameters() <= 0) {
    messageBox.setText(tr("No model object properties have been selected "
                          "for optimization. Please select one or more "
                          "properties to be optimized and try again."));
    messageBox.setStandardButtons(QMessageBox::Ok);
    messageBox.setDefaultButton(QMessageBox::Ok);
    messageBox.exec();
  } else if (m_Simulation->GetComparisonImageModelObject()) {
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
::on_fluoroSimUpdateObjectiveFunctionValueButton_clicked() {
  double value = m_Simulation->GetFluorescenceOptimizer()->GetObjectiveFunctionValue();

  QString valueStr; valueStr.sprintf("%e", value);
  gui->fluoroSimObjectiveFunctionValueEdit->setText(valueStr);
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
  gui->fluoroSimFocusSlider->setMinimum(1);
  gui->fluoroSimFocusSlider->setMaximum(fluoroSim->GetNumberOfFocalPlanes());
  gui->fluoroSimFocusSlider->setValue(static_cast<int>(fluoroSim->GetFocalPlaneIndex()+1));

  QString focalDepth = QVariant(fluoroSim->GetFocalPlanePosition()).toString();
  focalDepth.append(" nm");
  gui->fluoroSimFocalDepthEdit->setText(focalDepth);

  gui->fluoroSimCurrentPlaneEdit->setText(QVariant(fluoroSim->GetFocalPlaneIndex()+1).toString());
  gui->fluoroSimNumberOfFocalPlanesEdit->setText(QVariant(fluoroSim->GetNumberOfFocalPlanes()).toString());
  gui->fluoroSimFocalPlaneSpacingEdit->setText(QVariant(fluoroSim->GetFocalPlaneSpacing()).toString());
  gui->fluoroSimUseCustomFocalPlanePositionsCheckBox->setChecked(fluoroSim->GetUseCustomFocalPlanePositions());

  gui->fluoroSimPSFMenuComboBox->
    setCurrentIndex(fluoroSim->GetActivePSFIndex()+1);

  // Simulator Settings group box
  gui->fluoroSimGainEdit->setText(QVariant(fluoroSim->GetGain()).toString());
  gui->fluoroSimOffsetEdit->setText(QVariant(fluoroSim->GetOffset()).toString());
  gui->fluoroSimMaximumVoxelIntensityEdit->setText(QVariant(fluoroSim->GetMaximumVoxelIntensity()).toString());
  gui->fluoroSimPixelSizeEdit->setText(QVariant(fluoroSim->GetPixelSize()).toString());
  gui->fluoroSimImageWidthEdit->setText(QVariant(fluoroSim->GetImageWidth()).toString());
  gui->fluoroSimImageHeightEdit->setText(QVariant(fluoroSim->GetImageHeight()).toString());
  gui->fluoroSimShearInXEdit->setText(QVariant(fluoroSim->GetShearInX()).toString());
  gui->fluoroSimShearInYEdit->setText(QVariant(fluoroSim->GetShearInY()).toString());

  gui->fluorescenceQvtkWidget->setMinimumWidth(fluoroSim->GetImageWidth());
  gui->fluorescenceQvtkWidget->setMaximumWidth(fluoroSim->GetImageWidth());
  gui->fluorescenceQvtkWidget->setMinimumHeight(fluoroSim->GetImageHeight());
  gui->fluorescenceQvtkWidget->setMaximumHeight(fluoroSim->GetImageHeight());

  gui->fluoroSimAddGaussianNoiseCheckBox->setChecked(QVariant(fluoroSim->GetAddGaussianNoise()).toBool());
  gui->fluoroSimStdDevEdit->setText(QVariant(fluoroSim->GetNoiseStdDev()).toString());

  // Fluorescence Display group box
  gui->fluoroSimShowGridCheckBox->setChecked(QVariant(fluoroSim->GetShowReferenceGrid()).toBool());
  gui->fluoroSimGridSpacingEdit->setText(QVariant(fluoroSim->GetReferenceGridSpacing()).toString());
  gui->fluoroSimSuperimposeSimulatedImageCheckBox->setChecked(QVariant(fluoroSim->GetSuperimposeSimulatedImage()).toBool());
  gui->fluoroSimSuperimposeComparisonImageCheckBox->setChecked(QVariant(fluoroSim->GetSuperimposeComparisonImage()).toBool());
  gui->fluoroSimShowImageVolumeOutlineCheckBox->setChecked(QVariant(fluoroSim->GetShowImageVolumeOutline()).toBool());
  gui->fluoroSimMinLevelSlider->setValue((int) fluoroSim->GetMinimumIntensityLevel());
  gui->fluoroSimMaxLevelSlider->setValue((int) fluoroSim->GetMaximumIntensityLevel());
  gui->fluoroSimMinLevelEdit->setText(QVariant(fluoroSim->GetMinimumIntensityLevel()).toString());
  gui->fluoroSimMaxLevelEdit->setText(QVariant(fluoroSim->GetMaximumIntensityLevel()).toString());

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

  RefreshObjectiveFunctions();

  RenderViews();
}

void
MicroscopeSimulator
::RefreshObjectiveFunctions() {
  gui->fluoroSimObjectiveFunctionComboBox->blockSignals(true);
  gui->fluoroSimObjectiveFunctionComboBox->clear();

  FluorescenceOptimizer* optimizer = m_Simulation->GetFluorescenceOptimizer();
  if (!optimizer)
    return;

  // Repopulate by the objective functions available in the optimizer
  for (int i = 0; i < optimizer->GetNumberOfAvailableObjectiveFunctions(); i++) {
    std::string name = optimizer->GetAvailableObjectiveFunctionName(i);
    gui->fluoroSimObjectiveFunctionComboBox->addItem(QString(name.c_str()));

    if (name == optimizer->GetObjectiveFunctionName()) {
      gui->fluoroSimObjectiveFunctionComboBox->setCurrentIndex(i);
    }
  }

  gui->fluoroSimObjectiveFunctionComboBox->blockSignals(false);
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
