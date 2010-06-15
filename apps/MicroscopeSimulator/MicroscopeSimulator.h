#ifndef _MICROSCOPE_SIMULATOR_H_
#define _MICROSCOPE_SIMULATOR_H_

#include <QErrorMessage>
#include <QModelObjectListModel.h>
#include <QModelObjectPropertyListTableModel.h>

#include "ui_MicroscopeSimulator.h"

#include "DirtyListener.h"

// Forward class declarations
class vtkRenderer;
class vtkFramebufferObjectTexture;
class vtkFramebufferObjectRenderer;
class vtkGatherFluorescencePolyDataMapper;
class vtkInteractorStyleTrackballCamera;
class vtkQtOutputLogger;
class vtkRenderedSurfaceRepresentation;
class vtkRenderView;

// These forward declarations save a lot of recompile time when
// the classes they declare are modified.
class Simulation;
class Visualization;

class ErrorLogDialog;
class OptimizerSettingsDialog;
class PSFEditorDialog;
class Preferences;
class PreferencesDialog;
class QImageListModel;
class QPSFListModel;

#include <vtkSmartPointer.h>


class MicroscopeSimulator : public QMainWindow, public DirtyListener {
  Q_OBJECT

public:

  static const char* SIMULATION_ELEM;
  static const char* XML_ENCODING;

  // Constructor/destructor
  MicroscopeSimulator(QWidget* parent=0);
  virtual ~MicroscopeSimulator();
  
protected:
  Ui_MainWindow* gui;

  void Exit();
  void WriteProgramSettings();
  void WritePSFSettings();
  void ReadProgramSettings();
  void ReadPSFSettings();

  // Override the closeEvent handler.
  void closeEvent(QCloseEvent* event);

public slots:

  // Use Qt's auto-connect magic to tie GUI widgets to slots.
  // Names of the methods must follow the naming convention
  // on_<widget name>_<signal name>(<signal parameters>).
  virtual void on_actionNewSimulation_triggered();
  virtual void on_actionOpenSimulation_triggered();
  virtual void on_actionSaveSimulation_triggered();
  virtual void on_actionSaveSimulationAs_triggered();
  virtual void on_actionExit_triggered();

  // Add geometry
  virtual void on_actionAddCylinder_triggered();
  virtual void on_actionAddHollowCylinder_triggered();
  virtual void on_actionAddDisk_triggered();
  virtual void on_actionAddFlexibleTube_triggered();
  virtual void on_actionAddPlane_triggered();
  virtual void on_actionAddPointRing_triggered();
  virtual void on_actionAddPointSet_triggered();
  virtual void on_actionAddSphere_triggered();
  virtual void on_actionAddTorus_triggered();

  // Import
  virtual void on_actionImportImageData_triggered();
  virtual void on_actionImportGeometryFile_triggered();

  virtual void on_actionShowErrors_toggled(bool visible);
  virtual void handle_ErrorLogDialog_accepted();
  virtual void on_actionFluorescenceWindow_toggled(bool visible);  
  virtual void on_actionAboutApplication_triggered();
  virtual void on_actionPreferences_triggered();

  // Button action handlers.
  virtual void on_actionSaveImage_triggered();
  virtual void on_actionResetCamera_triggered();

  virtual void on_actionViewModelOnly_triggered();
  virtual void on_actionViewModelAndPoints_triggered();
  virtual void on_actionViewModelAndScan_triggered();
  virtual void on_actionViewAFMScanOnly_triggered();
  virtual void on_actionViewModelsWithFluorescenceComparison_triggered();
  virtual void on_actionViewFluorescenceComparisonOnly_triggered();
  virtual void on_actionEnableDisableAxes_triggered();
  virtual void on_actionMoveCamera_triggered();
  virtual void on_actionMoveObjects_triggered();

  // Model object and propery list event handlers.
  virtual void on_fluoroSimModelObjectList_customContextMenuRequested(QPoint point);
  virtual void on_actionFocusOnObject_triggered();
  virtual void on_actionExportGeometry_triggered();
  virtual void on_actionDeleteModelObject_triggered();

  virtual void handle_ModelObjectListSelectionModel_selectionChanged
    (const QItemSelection& selected, const QItemSelection& deselected);

  virtual void on_fluoroSimModelObjectPropertiesTable_clicked(const QModelIndex& index);

  virtual void handle_ModelObjectPropertyListTableModel_dataChanged
    (const QModelIndex& topLeft, const QModelIndex& bottomRight);

  // GUI widget event handlers.
  virtual void on_experimentDescriptionTextArea_textChanged();

  virtual void on_afmSimRadiusSpinBox_valueChanged(int i);
  virtual void on_afmSimUseConeSphereModelCheckBox_stateChanged(int state);
  virtual void on_afmSimConeAngleEdit_textEdited(const QString& text);
  virtual void on_afmSimPixelSizeEdit_textEdited(const QString& text);
  virtual void on_afmSimImageWidthEdit_textEdited(const QString& text);
  virtual void on_afmSimImageHeightEdit_textEdited(const QString& text);
  virtual void on_afmSimClipGroundPlaneCheckBox_stateChanged(int state);
  virtual void on_afmSimDisplayAsWireframeCheckBox_stateChanged(int state);
  virtual void on_afmSimSurfaceOpacityEdit_textEdited(const QString& text);
  virtual void on_afmSimCompareScanToComboBox_currentIndexChanged(int index);
  virtual void on_afmSimSaveSimulatedScan_clicked();
  virtual void on_afmSimScanStatistics_clicked();

  virtual void on_fluoroSimFocusSlider_valueChanged(int value);

  virtual void on_fluoroSimFocusMaxEdit_editingFinished();
  virtual void on_fluoroSimFocusMinEdit_editingFinished();
  virtual void on_fluoroSimFocusSpacingEdit_editingFinished();
  virtual void on_fluoroSimPSFMenuComboBox_currentIndexChanged(int i);
  virtual void on_fluoroSimEditPSFsButton_clicked();
  virtual void on_fluoroSimShowImageVolumeOutlineCheckBox_toggled(bool);
  virtual void on_fluoroSimExposureEdit_textChanged(QString text);
  virtual void on_fluoroSimPixelSizeEdit_editingFinished();
  virtual void on_fluoroSimImageWidthEdit_editingFinished();
  virtual void on_fluoroSimImageHeightEdit_editingFinished();
  virtual void on_fluoroSimAddGaussianNoiseCheckBox_toggled(bool);
  virtual void on_fluoroSimStdDevEdit_editingFinished();
  virtual void on_fluoroSimMeanEdit_editingFinished();
  virtual void on_fluoroSimShowReferencePlaneCheckBox_toggled(bool);
  virtual void on_fluoroSimShowGridCheckBox_toggled(bool);
  virtual void on_fluoroSimSuperimposeFluorescenceImageCheckBox_toggled(bool);
  virtual void on_fluoroSimGridSpacingEdit_editingFinished();
  virtual void on_fluoroSimMinLevelEdit_editingFinished();
  virtual void on_fluoroSimMinLevelSlider_valueChanged(int value);
  virtual void on_fluoroSimMaxLevelEdit_editingFinished();
  virtual void on_fluoroSimMaxLevelSlider_valueChanged(int value);
  virtual void on_fluoroSimScaleToFullDynamicRange_clicked();
  virtual void on_fluoroSimExportImageButton_clicked();
  virtual void on_fluoroSimExportStackButton_clicked();
  virtual void on_fluoroSimComparisonImageComboBox_currentIndexChanged(int selected);
  virtual void on_fluoroSimCopyImageSettingsButton_clicked();

  virtual void on_fluoroSimOptimizationMethodComboBox_currentIndexChanged(int selected);
  virtual void on_fluoroSimOptimizerSettingsButton_clicked();

  virtual void on_fluoroSimObjectiveFunctionComboBox_currentIndexChanged(int selected);
  
  virtual void on_fluoroSimOptimizeButton_clicked();

protected:
  Simulation* m_Simulation;
  
  Visualization* m_Visualization;

  QActionGroup* m_ViewModeActionGroup;
  QActionGroup* m_InteractionActionGroup;
  
  QModelObjectListModel*              m_ModelObjectListModel;
  QItemSelectionModel*                m_ModelObjectListSelectionModel;
  QModelObjectPropertyListTableModel* m_ModelObjectPropertyListTableModel;
  QImageListModel*                    m_ImageListModel;
  QPSFListModel*                      m_PSFMenuListModel;

  void NewSimulation();
  void OpenSimulationFile(const std::string& fileName);
  void SaveSimulationFile(const std::string& fileName);
  
  void AddNewModelObject(const std::string& objectName);

  void UpdateFocalPlaneUIControls(float minValue, float maxValue, float spacing);

  int PromptToSaveChanges();
  void UpdateMainWindowTitle();
  void RefreshModelObjectViews();
  void RenderViews();
  void RefreshSimulationDescription();
  void RefreshUI();
  void RefreshObjectiveFunctions();

  void SetStatusMessage(const std::string& status);
  void Sully();

protected slots:

private:
  bool m_SimulationNeedsSaving;

  vtkSmartPointer<vtkRenderer>                  m_ModelObjectRenderer;
  vtkSmartPointer<vtkRenderView>                m_ModelObjectRenderView;
  vtkSmartPointer<vtkFramebufferObjectTexture>  m_FluorescenceRenderTexture;
  vtkSmartPointer<vtkFramebufferObjectRenderer> m_FluorescenceRenderer;
  
  QErrorMessage      m_ErrorDialog;
  ErrorLogDialog*    m_ErrorLogDialog;
  vtkQtOutputLogger* m_ErrorLogger;

  PSFEditorDialog*   m_PSFEditorDialog;

  OptimizerSettingsDialog* m_OptimizerSettingsDialog;

  Preferences*       m_Preferences;
  PreferencesDialog* m_PreferencesDialog;

  QModelIndex m_SelectedModelObjectIndex;
};

#endif // _MICROSCOPE_SIMULATOR_H_
