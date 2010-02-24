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


int main(int argc, char** argv) {

  // Qt initialization.
  QApplication app(argc, argv);
  QPixmap pixmap(":splash/SplashScreen.png");
  QSplashScreen splash(pixmap);
  splash.show();
  app.processEvents();

  MicroscopeSimulator mainWindow;
  mainWindow.show();
  splash.finish(&mainWindow);
  
  return app.exec();
}
