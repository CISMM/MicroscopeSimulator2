#
# Source files
#
# Here is where you can add the name of your local filtering classes.
#

SET (Filtering_SRCS
  vtkImageConvolvePoints.cxx
  #vtkPolyDataDistance.cxx
  #vtkPolyDataIntersection.cxx
  vtkPolyDataTexturizer.cxx
  vtkPolyDataUtilities.cxx
  vtkPrincipalCurvatures.cxx
  #vtkRefactorPolyData.cxx
  vtkUniformPointSampler.cxx
  vtkSurfaceUniformPointSampler.cxx
  vtkVolumeUniformPointSampler.cxx
  vtkPolyDataToTetrahedralGrid.cxx
  vtkPointRingSource.cxx
  tetgen.h
  tetgen.cxx
  predicates.cxx
)

SET_SOURCE_FILES_PROPERTIES(
  vtkPolyDataUtilities.cxx
  tetgen.h
  tetgen.cxx
  predicates.cxx
WRAP_EXCLUDE
)

# Needed to tell TetGen it is to produce a library in an application.
ADD_DEFINITIONS(-DTETLIBRARY)

# Create the msvtkFiltering C++ library
ADD_LIBRARY (msvtkFiltering ${Filtering_SRCS})
TARGET_LINK_LIBRARIES(msvtkFiltering
  ${VTK_LIBRARIES}
)
