Last update: 2010-08-11
Author: Cory Quammen <cquammen@cs.unc.edu>

This document gives instructions on how to build the MicroscopeSimulator2
project.


PRELIMINARIES

You'll need to obtain CMake, Qt, libxml2, VTK, and ITK.

1. Download CMake 2.8.10 or higher from
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
Qt 4.8.4 is known to work and is available here:

http://releases.qt-project.org/qt4/source/qt-everywhere-opensource-src-4.8.4.tar.gz

Unzip this file. The tar.gz file contains the directory qt-everywhere-opensource-src-4.8.4,
so there is no need to place the zip file in a new directory.

Add the qt-everywhere-opensource-src-4.8.4/bin directory to PATH environment variable.
Open a command prompt through Start -> All Programs -> Microsoft Visual Studio 2008 -> 
Visual Studio Tools -> Visual Studio 2008 Command Prompt. Navigate to the 
qt-everywhere-opensource-src-4.8.4 directory and type

configure -platform win32-msvc2008 -no-webkit -no-dbus -no-phonon
-debug-and-release -opensource -shared

Agree to the license terms by typing ‘y’ and pressing return. Qt will take a while
to configure. Type ‘nmake’ when the configuration is done and wait a long time for
the compilation to happen.

3. On Windows, download the libxml2 source code (on Mac OS X and linux, you can
skip this step). The best way to do this is via the git command:

git clone git://git.gnome.org/libxml2

Change the version to v2.7.6 with the following:

git checkout v2.7.6

Open a Visual Studio command prompt. For 64-bit builds, open a Visual Studio x64 Win64
command prompt instead. Change the directory to the libxml2/win32 directory in the
libxml2 source code directory. Configure the project with this command:

cscript configure.js ftp=no http=no iconv=no vcmanifest=yes prefix=<LIBXML2_BUILD_DIRECTORY>

In the same directory, make the project with the command

nmake /f Makefile.msvc

Install it with the command

nmake /f Makefile.msvc install

4. Download the VTK source code. The best way to do this is to clone it from 
the git repository

git clone git://vtk.org/VTK.git

A stable version that is known to work with the Microscope Simulator 2 is commit
0700cfe8229c.
To obtain VTK, run

git checkout 0700cfe8229c
git submodule update

Configure VTK with CMake and build it. You'll need to configure it once first.
Then, you'll need to enable BUILD_SHARED_LIBS, disable BUILD_TESTING
(if you want to speed up the build process), enable VTK_USE_GUI_SUPPORT,
enable VTK_USE_QT, enable VTK_USE_VIEWS, and enable VTK_USE_SYSTEM_LIBXML2.
Configure the project again.

After configuration is finished, CMake will complain that LIBXML2_LIBRARY is not found.
No worries. Set the following variables to the given values

LIBXML2_INCLUDE_DIR         <LIBXML2_BUILD_DIRECTORY>/include
LIBXML2_LIBRARIES           <LIBXML2_BUILD_DIRECTORY>/lib/libxml2.lib
LIBXML2_XMLLINT_EXECUTABLE  <LIBXML2_BUILD_DIRECTORY>/bin/xmllint.exe

On Windows, set QT_QMAKE_EXECUTABLE to QT_BUILD_DIRECTORY/bin/qmake.exe.
For faster builds using multiple cores on Windows, enable CMAKE_CXX_MP_FLAG and
set CMAKE_CXX_MP_NUM_PROCESSORS to the number of cores on your machine.

Configure again. You should be able to now generate the Visual Studio solution file
by clicking on the "Generate" button. Build VTK in Visual Studio.

5. Download the ITK source code. The best way to do this is to clone it from
the git repository

git clone git://itk.org/ITK.git
git checkout v4.3.1
git submodule update

Configure ITK with CMake. Disable BUILD_EXAMPLES, enable BUILD_SHARED_LIBS,
and disable BUILD_TESTING. Click the "Generate" button. Build ITK in Visual Studio.

6. Download CLAPACK from

http://www.netlib.org/clapack/clapack-3.2.1-CMAKE.tgz

Unzip and untar the archive. Configure it with CMake. No non-default options
are required. Generate the Visual Studio solution file and build CLAPACK in
Visual Studio.

7. Configure the MicroscopeSimulator2 project using CMake. Set VTK_DIR to the VTK build directory
from step 4. Set ITK_DIR to the ITK build directory from step 5. Set CLAPACK_DIR to the
CLAPACK build directory in step 6.

