Last update: 2010-08-11
Author: Cory Quammen <cquammen@cs.unc.edu>

This document gives instructions on how to build the MicroscopeSimulator2
project.


PRELIMINARIES

You'll need to obtain CMake, Qt, libxml2, VTK, and ITK.

1. Download CMake 2.8 or higher from
http://www.cmake.org/cmake/resources/software.html

If you are developing on Windows, you will also need to develop the NSIS
scriptable install system from http://nsis.sourceforge.net/Download.
CMake requires this to make installer programs.

2. On Mac OS X, you can download the Qt SDK binaries from

ftp://ftp.qt.nokia.com/qtsdk/qt-sdk-mac-opensource-2009-04.dmg

and install them.

On Windows, download Qt libraries and applications by going to
http://www.qtsoftware.com/downloads/downloads and choosing the LGPL/Free tab.
Download the Qt SDK appropriate for the platform on which you are developing.
Qt 4.6.3 is known to work and is available here:

ftp://ftp.qt.nokia.com/qt/source/qt-everywhere-opensource-src-4.6.3.zip

Unzip this file. The zip file contains the directory qt-everywhere-opensource-src-4.6.3,
so there is no need to place the zip file in a new directory.

Add the qt-everywhere-opensource-src-4.6.3/bin directory to PATH environment variable.
Open a command prompt through Start -> All Programs -> Microsoft Visual Studio 2005 -> 
Visual Studio Tools -> Visual Studio 2005 Command Prompt. Navigate to the 
qt-all-opensource-src-4.5.3 directory and type

configure -platform win32-msvc2005 -no-webkit -no-dbus -no-phonon
-debug-and-release -opensource -shared

Agree to the license terms by typing ‘y’ and pressing return. Qt will take a while
to configure. Type ‘nmake’ when the configuration is done and wait a long time for
the compilation to happen.

3. On Windows, download the libxml2 source code (on Mac OS X and linux, you can
skip this step). The best way to do this is via the git command:

git clone git://git.gnome.org/libxml2

Open a Visual Studio command prompt. Change the directory to the
libxml2/win32 directory in the libxml2 source code directory. Configure the
project with this command:

cscript configure.js ftp=no http=no iconv=no vcmanifest=yes prefix=<LIBXML2_BUILD_DIRECTORY>

Make the project with the command

nmake /f Makefile.msvc

Install it with the command

nmake /f Makefile.msvc install

4. Download the VTK source code. The best way to do this is to clone it from 
the git repository

git://vtk.org/VTK.git

You may optionally checkout a stable version of VTK from the git repository
using

git checkout v5.6.0

Configure VTK with CMake and build it. Turn on the option BUILD_SHARED_LIBS.
You'll need to enable GUI support (enable option VTK_USE_GUI_SUPPORT),
Qt (option VTK_USE_QT), VTK_USE_VIEWS, and VTK_USE_SYSTEM_LIBXML2. Set the following
variables to the given values:

LIBXML2_INCLUDE_DIR         <LIBXML2_BUILD_DIRECTORY>/include
LIBXML2_LIBRARIES           <LIBXML2_BUILD_DIRECTORY>/lib/libxml2.lib
LIBXML2_XMLLINT_EXECUTABLE  <LIBXML2_BUILD_DIRECTORY>/bin/xmllint.exe

On Windows, set QT_QMAKE_EXECUTABLE to QT_BUILD_DIRECTORY/bin/qmake.exe.

5. Download the ITK source code at

http://www.itk.org/ITK/resources/software.html.

Version 3.20 is known to work.
Choose either InsightToolkit*.tar.gz or InsightToolkit*.zip 
depending on what extraction applications you have available on your development
system. Configure ITK with CMake and build it. Turn off BUILD_TESTING and BUILD_EXAMPLES
and turn on BUILD_SHARED_LIBS.

6. Configure the MicroscopeSimulator2 project using CMake. Set VTK_DIR to
the VTK build directory from step 4. Set ITK_DIR to the ITK build directory from step 5.

