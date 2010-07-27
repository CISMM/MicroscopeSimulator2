INCLUDE(${VTK_USE_FILE})

SET (SourceFiles
  GLCheck.cxx
  TestRequiredExtensions.cxx
  Test16BitFloatingPointBlend.cxx
  Test32BitFloatingPointBlend.cxx
  TestFloatingPointTextureTrilinearInterpolation.cxx
  TestGLSLUnsignedInts.cxx
)

INCLUDE_DIRECTORIES(
  .
)

ADD_EXECUTABLE( GLCheck ${SourceFiles} )

TARGET_LINK_LIBRARIES( GLCheck
  vtksys
  vtkCommon
  vtkFiltering
  vtkGraphics
  vtkHybrid
  vtkImaging
  vtkRendering
  vtkVolumeRendering
  msvtkCommon
  msvtkFiltering
  msvtkImaging
  msvtkRendering
)

INSTALL( TARGETS GLCheck
  RUNTIME DESTINATION bin
)
