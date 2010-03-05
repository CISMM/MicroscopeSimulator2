INCLUDE(${VTK_USE_FILE})

# use what QVTK built with
SET(QT_MOC_EXECUTABLE ${VTK_QT_MOC_EXECUTABLE} CACHE FILEPATH "")
SET(QT_UIC_EXECUTABLE ${VTK_QT_UIC_EXECUTABLE} CACHE FILEPATH "")
SET(QT_QMAKE_EXECUTABLE ${VTK_QT_QMAKE_EXECUTABLE} CACHE FILEPATH "")
SET(DESIRED_QT_VERSION ${VTK_DESIRED_QT_VERSION} CACHE FILEPATH "")
FIND_PACKAGE(Qt)
IF(QT_USE_FILE)
  INCLUDE(${QT_USE_FILE})
ELSE(QT_USE_FILE)
  SET(QT_LIBRARIES   ${QT_QT_LIBRARY})
ENDIF(QT_USE_FILE)

SET (QtGuiSrcs
  Main.cxx
  MicroscopeSimulator.cxx
  ErrorLogDialog.cxx
  FluorophoreModelDialog.cxx
  PSFEditorDialog.cxx
  Preferences.cxx
  PreferencesDialog.cxx
  QModelObjectListModel.cxx
  QModelObjectPropertyListTableModel.cxx
  QModelObjectPropertyListItemDelegate.cxx
  QPointSpreadFunctionPropertyTableModel.cxx
  QPSFListModel.cxx
  vtkQtOutputLogger.cxx
)

# Use the include path and library for Qt that is used by VTK.
INCLUDE_DIRECTORIES(
  ${QT_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

SET(UIS
  MicroscopeSimulator.ui
  ErrorLogDialog.ui
  FluorophoreModelDialog.ui
  PSFEditorDialog.ui
  PreferencesDialog.ui
)
SET(RCCS
  resources/icons/icons.qrc
  resources/splash/splash.qrc
)
SET (ApplicationHeaders
  MicroscopeSimulator.h
  ErrorLogDialog.h
  FluorophoreModelDialog.h
  PSFEditorDialog.h
  PreferencesDialog.h
  QModelObjectListModel.h
  QModelObjectPropertyListTableModel.h
  QModelObjectPropertyListItemDelegate.h
  QPointSpreadFunctionPropertyTableModel.h
  QPSFListModel.h
)
QT4_WRAP_UI(UIHeaders ${UIS})
QT4_ADD_RESOURCES(RCCSrcs ${RCCS})
QT4_WRAP_CPP(MOCSrcs ${ApplicationHeaders} )
ADD_DEFINITIONS(-DQT_GUI_LIBS -DQT_CORE_LIB -DQT3_SUPPORT)
SET_SOURCE_FILES_PROPERTIES(${QtGuiSrcs} PROPERTIES
                          OBJECT_DEPENDS "${UIHeaders}")


# It's nice to have the ui in the windows project file...just double click on it
# and designer comes up on that ui file :)
IF (${CMAKE_BUILD_TOOL} MATCHES "msdev")
  SET (QtGuiSrcs ${QtGuiSrcs} ${UIS})
ENDIF (${CMAKE_BUILD_TOOL} MATCHES "msdev")
IF (${CMAKE_BUILD_TOOL} MATCHES "devenv")
  SET (QtGuiSrcs ${QtGuiSrcs} ${UIS})
ENDIF (${CMAKE_BUILD_TOOL} MATCHES "devenv")

ADD_EXECUTABLE( MicroscopeSimulator ${QtGuiSrcs} ${UISrcs} ${RCCSrcs} ${MOCSrcs} )

TARGET_LINK_LIBRARIES( MicroscopeSimulator
  QVTK
  ${QT_LIBRARIES}
  vtksys
  vtkCommon
  vtkFiltering
  vtkGraphics
  vtkHybrid
  vtkImaging
  vtkRendering
  vtkVolumeRendering
  vtkmsCommon
  vtkmsFiltering
  vtkmsImaging
  vtkmsRendering
  itksys
  ITKCommon
  ITKBasicFilters
  ITKIO
  msimFilter
  msimIO
  msimModel
  msimVisualization
)

IF( WIN32 )
  TARGET_LINK_LIBRARIES( MicroscopeSimulator
      msimWinIcon
  )
ENDIF( WIN32 )

# Set package properties
SET( CPACK_PACKAGE_NAME "MicroscopeSimulator" )
SET( CPACK_PACKAGE_VENDOR "CISMM" )
SET( CPACK_PACKAGE_EXECUTABLES "MicroscopeSimulator;Microscope Simulator ${MicroscopeSimulator_VERSION_STRING}" )
SET( CPACK_PACKAGE_DESCRIPTION_SUMMARY "Microscope Simulator" )
SET( CPACK_PACKAGE_VERSION "${MicroscopeSimulator_VERSION_STRING}" )
SET( CPACK_PACKAGE_VERSION_MAJOR "${MicroscopeSimulator_MAJOR_NUMBER}" )
SET( CPACK_PACKAGE_VERSION_MINOR "${MicroscopeSimulator_MINOR_NUMBER}" )
SET( CPACK_PACKAGE_VERSION_PATCH "${MicroscopeSimulator_REVISION_NUMBER}" )
SET( CPACK_PACKAGE_INSTALL_DIRECTORY "Microscope Simulator ${MicroscopeSimulator_VERSION_STRING}" )
SET( CPACK_PACKAGE_INSTALL_REGISTRY_KEY "Microscope Simulator ${MicroscopeSimulator_VERSION_STRING}" )
SET( CPACK_RESOURCE_FILE_LICENSE "${MicroscopeSimulator_SOURCE_DIR}/apps/MicroscopeSimulator/installer/licenses/MicroscopeSimulator-license.txt" )
SET( CPACK_RESOURCE_FILE_README  "${MicroscopeSimulator_SOURCE_DIR}/apps/MicroscopeSimulator/installer/licenses/README.txt" )

INCLUDE( installer/CMakeLists.txt )

INSTALL( TARGETS MicroscopeSimulator
  RUNTIME DESTINATION bin
)

# This must always be last!
INCLUDE( CPack )

# This includes Microsoft runtime libraries when run in Windows dev environment.
INCLUDE( InstallRequiredSystemLibraries )
