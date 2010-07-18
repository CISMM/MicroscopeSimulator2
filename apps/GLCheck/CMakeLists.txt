INCLUDE(${VTK_USE_FILE})

SET (SourceFiles
  GLCheck.cxx
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
  vtkmsCommon
  vtkmsFiltering
  vtkmsImaging
  vtkmsRendering
)

