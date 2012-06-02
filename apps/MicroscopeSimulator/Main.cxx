/**
 * This file is where the program starts. The
 * main() function does very little other than
 * instantiate a QApplication and the application
 * main window.
 */

#include <QApplication>
#include <QPixmap>
#include <QSplashScreen>

#include <MicroscopeSimulator.h>


int main(int argc, char* argv[]) {

  // Set application information
  QCoreApplication::setOrganizationName("CISMM");
  QCoreApplication::setOrganizationDomain("cismm.org");
  QCoreApplication::setApplicationName("Microscope Simulator");

  // Qt initialization.
  QApplication app(argc, argv);
  QPixmap pixmap(":splash/SplashScreen.png");
  QSplashScreen splash(pixmap);
  splash.show();
  splash.showMessage("Loading libraries...", Qt::AlignBottom | Qt::AlignRight, Qt::white);
  app.processEvents();

  MicroscopeSimulator mainWindow(argc, argv);
  mainWindow.show();
  splash.finish(&mainWindow);

  mainWindow.ProcessCommandLineArguments(argc, argv);
  
  if (!mainWindow.IsBatchMode()) {
    return app.exec();
  }
}
