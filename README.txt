Last update: 2009-09-15
Author: Cory Quammen <cquammen@cs.unc.edu>

This document gives instructions on how to create a new Qt/VTK/ITK-based
application from this generic example.


PRELIMINARIES

You'll need to obtain CMake, Qt, VTK, and ITK.

1. Download CMake 2.6 or higher from
http://www.cmake.org/cmake/resources/software.html

If you are developing on Windows, you will also need to develop the NSIS
scriptable install system from http://nsis.sourceforge.net/Download.
CMake requires this to make installer programs.

2. Download Qt libraries and applications by going to
http://www.qtsoftware.com/downloads/downloads and choosing the LGPL/Free tab.
Download the Qt SDK appropriate for the platform on which you are developing.

3. Download the VTK source code. The best way to do this is to access it from 
the CVS repository. Instructions for doing so are at 
http://www.vtk.org/VTK/resources/software.html#cvs. Configure VTK with CMake
and build it. You'll need to enable GUI support (option VTK_USE_GUI_SUPPORT),
Qt (option VTK_USE_QVTK), and Qt version 4 (set option DESIRED_QT_VERSION to 4).

4. Download the ITK source code at
http://www.itk.org/ITK/resources/software.html. Version 3.14 is known to work 
with this example. Choose either InsightToolkit*.tar.gz or InsightToolkit*.zip 
depending on what extraction applications you have available on your development
system. Configure ITK with CMake and build it. No non-default options are 
required for this application example to work.


CREATING A NEW PROJECT

Copy this project root directory to a new location in your filesystem where you 
will be able to import it into CVS. Make sure you delete the CVS directories in
this project. A one-liner for this task in linux (while the current working
directory is the root of the MicroscopeSimulator) is

  find -type d -wholename '*CVS' | xargs /bin/rm -rf
  
Better yet, export the MicroscopeSimulator using your CVS client, e.g.

  cvs export -DNOW MicroscopeSimulator
  
or in TortoiseCVS (version 1.10 and above), select "Export" under "Purpose of checkout"
in the "Options" tab.

Next, you'll need to change your project name. Find and replace these strings 
(exluding quotes) throughout the text files in the source tree (within files and
file names) with a string appropriate to your project:

  "MicroscopeSimulator"   - used in variables and paths
  "MICROSCOPE_SIMULATOR" - used in a header definition
  "Microscope Simulator" - used for presentation to humans
  "msim"                        - prefix for library names
  "MSIM"                        - prefix for shell script variables in Mac OS X

A one-liner for finding and replacing a string in a hierarchy of directories on
linux and Mac OS X is:

  grep -rl oldstring . | xargs sed 's/oldstring/newstring/'

On Windows, the TextCrawler application (http://www.digitalvolcano.co.uk/content/textcrawler)
works nicely.

You'll also need to change the names of some files and directories to match the string you
used to replace "MicroscopeSimulator" (let's call it "MyApplication"). Change the
following:

  MicroscopeSimulator
  apps/MicroscopeSimulator
  apps/MicroscopeSimulator/MicroscopeSimulator.cxx
  apps/MicroscopeSimulator/MicroscopeSimulator.h
  apps/MicroscopeSimulator/MicroscopeSimulator.ui
  apps/MicroscopeSimulator/installer/licenses/MicroscopeSimulator-license.txt


CHANGE ICONS

There are two icon files, one for Mac OS X and one for Windows. These are
located in 
  apps/MicroscopeSimulator/installer/MacOSX/MicroscopeSimulator.icns
  apps/MicroscopeSimulator/installer/Win32/Win32Icon.ico

Modify these icons using appropriate tools. On Mac OS X, Icon Composer works
well. On Windows, you can download one of numerous icon editors.


SPECIAL INSTRUCTIONS FOR WINDOWS USERS

QtVTKITK is currently set up to changed the default installation directory to C:\NSRG.
By default CMake 2.6 does not support changing the default installation directory on
Windows. To enable this, you MUST replace all occurences of $PROGRAMFILES with 
@CPACK_INSTALL_DIRECTORY@ in the text file
C:\Program Files\CMake 2.6\share\cmake-2.6\Modules\NSIS.template.in. If you do not
take this step, the installer will default to C:\ProgramFiles on most systems
(technically, it will install in the directory specified by the ProgramFiles
environment variable).

If you'd like to change the default install directory, open the file
MicroscopeSimulator/apps/MicroscopeSimulator/installer/Win32/CMake/Win32Package.txt
and change the value of CPACK_INSTALL_DIRECTORY. Note that the four backslashes
("\\\\") are necessary because both CMake and NSIS require the backslash to be
escaped.


BUILD YOUR NEW APPLICATION

Create a directory for the build and configure the new project with CMake.
Congratulations, you are now ready to specialize your new Qt/VTK/ITK-based
application!
