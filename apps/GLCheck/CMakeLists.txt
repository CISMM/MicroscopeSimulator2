INCLUDE(${VTK_USE_FILE})

SET (SourceFiles
  GLCheck.cxx
  Common.h
  Common.cxx
  TestRequiredExtensions.h
  TestRequiredExtensions.cxx
  Test16BitFloatingPointBlend.h
  Test16BitFloatingPointBlend.cxx
  Test32BitFloatingPointBlend.h
  Test32BitFloatingPointBlend.cxx
  TestFloatingPointTextureTrilinearInterpolation.h
  TestFloatingPointTextureTrilinearInterpolation.cxx
  TestGLSLUnsignedInts.h
  TestGLSLUnsignedInts.cxx
)

INCLUDE_DIRECTORIES(
  .
)

ADD_EXECUTABLE( GLCheck ${SourceFiles} )

TARGET_LINK_LIBRARIES( GLCheck
  msimFilter
  msimIO
  msimModel
  msimVisualization
  msimUtilities
  msvtkFiltering
  msvtkImaging
  msvtkRendering
  ${VTK_LIBRARIES}
  ${OPENGL_LIBRARIES}
)

INSTALL( TARGETS GLCheck
  RUNTIME DESTINATION bin
)
