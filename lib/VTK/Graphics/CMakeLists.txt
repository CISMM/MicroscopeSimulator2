#
# Source files
#
# Here is where you can add the name of your local rendering classes.
#

SET (Graphics_SRCS
  vtkDiskSource2.cxx
  vtkRectangleSource.cxx
  vtkVolumetricCylinderSource.cxx
  vtkVolumetricHollowCylinderSource.cxx
  vtkVolumetricEllipsoidSource.cxx
  vtkVolumetricTorusSource.cxx
)

# Create the msvtkGraphics C++ library
ADD_LIBRARY (msvtkGraphics ${Graphics_SRCS})
TARGET_LINK_LIBRARIES(msvtkGraphics 
  ${VTK_LIBRARIES}
)
