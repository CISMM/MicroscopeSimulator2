#
# Source files
#
# Here is where you can add the name of your local rendering classes.
#

SET (Common_SRCS
  #vtkImplicitPolyData.cxx
)

# Create the msvtkCommon C++ library
ADD_LIBRARY (msvtkCommon ${Common_SRCS})
TARGET_LINK_LIBRARIES(
  msvtkCommon
  vtkGraphics
  vtkCommon
  vtkFiltering
  vtkRendering
  vtkHybrid
  vtkImaging
  vtkIO
  ${OPENGL_LIBRARIES}
)
