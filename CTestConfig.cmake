## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
## # The following are required to uses Dart and the Cdash dashboard
##   ENABLE_TESTING()
##   INCLUDE(CTest)
set(CTEST_PROJECT_NAME "Microscope Simulator")
set(CTEST_NIGHTLY_START_TIME "00:00:00 EST")

set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "silver-cs.cs.unc.edu")
set(CTEST_DROP_LOCATION "/cdash/submit.php?project=Microscope+Simulator")
set(CTEST_DROP_SITE_CDASH TRUE)
