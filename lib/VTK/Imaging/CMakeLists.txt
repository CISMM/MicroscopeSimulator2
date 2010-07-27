#
# Source files
#
# Here is where you can add the name of your local rendering classes.
#

SET (Imaging_SRCS
  vtkImageConstantSource.cxx
)

# Create the msvtkImaging C++ library
ADD_LIBRARY (msvtkImaging ${Imaging_SRCS})
TARGET_LINK_LIBRARIES(msvtkImaging vtkImaging)

