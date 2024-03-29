#
# Source files
#
# Here is where you can add the name of your local rendering classes.
#

SET (Rendering_SRCS
  vtkAFMTipDilationMapper.cxx
  vtkFramebufferObjectRenderer.cxx
  vtkFramebufferObjectTexture.cxx
  vtkFluorescencePolyDataMapper.cxx
  vtkFluorescencePointsGradientPolyDataMapper.cxx
  vtkFluorescencePointsGradientRenderer.cxx
  vtkBlendingPolyDataMapper.cxx
  vtkBlendingFluorescencePolyDataMapper.cxx
  vtkFluorescenceRenderer.cxx
  vtkGatherFluorescencePolyDataMapper.cxx
  vtkOpenGL3DTexture.cxx
  vtkOpenGLAlphaTestTexture.cxx
  vtkOpenGLShadowRenderer.cxx  
)

SET( SHADER_FILES
   "lib/VTK/Rendering/Shaders/BackgroundIntensity_fragment.glsl"
   "lib/VTK/Rendering/Shaders/GaussianNoise_fragment.glsl"
   "lib/VTK/Rendering/Shaders/vtkFluorescencePointsGradientPolyDataMapper_fragment.glsl"
   "lib/VTK/Rendering/Shaders/vtkFluorescencePointsGradientPolyDataMapper_vertex.glsl"
   "lib/VTK/Rendering/Shaders/vtkGatherFluorescencePolyDataMapper_fragment.glsl"
   "lib/VTK/Rendering/Shaders/vtkGatherFluorescencePolyDataMapper_vertex.glsl"
)

# This formats the shaders in a form that is suitable for including
# in a C++ source file and assigning the resulting contents to a string.
FOREACH( SHADER_FILE ${SHADER_FILES})
  # This call is here simply to trigger CMake to run again if any of the
  # shader input files are modified.
  CONFIGURE_FILE( "${MicroscopeSimulator_SOURCE_DIR}/${SHADER_FILE}"
                  "${MicroscopeSimulator_BINARY_DIR}/${SHADER_FILE}" )

  # These calls actually trigger the addition of quotes and endlines
  FILE( READ "${MicroscopeSimulator_SOURCE_DIR}/${SHADER_FILE}" SHADER )
  STRING( REGEX REPLACE "([^\n]*)\n" "\"\\1\\\\n\"\n" PROCESSED_SHADER "${SHADER}" )
  SET( HEADER_MSG "// WARNING: These files are autogenerated by CMake. All edits to them will be lost.\n\n" )
  FILE( WRITE "${MicroscopeSimulator_BINARY_DIR}/${SHADER_FILE}" "${HEADER_MSG}${PROCESSED_SHADER}" )
ENDFOREACH( SHADER_FILE )

INCLUDE_DIRECTORIES( "${MicroscopeSimulator_BINARY_DIR}/lib/VTK/Rendering/Shaders" )

IF (BUILD_WITH_CUDA)
  SET (Rendering_SRCS
    ${Rendering_SRCS}
    vtkBinningFluorescencePolyDataMapper.cxx
    vtkBinningFluorescenceRenderer.cxx
  )
ENDIF (BUILD_WITH_CUDA)

SET (RenderingCUDA_SRCS
  vtkBinningFluorescenceRendererCUDA.cu
  stopwatch/stopwatch.cpp
  stopwatch/stopwatch_win.cpp
)

INCLUDE_DIRECTORIES(${MicroscopeSimulator_SOURCE_DIR}/lib/VTK/Rendering/stopwatch)

SET_SOURCE_FILES_PROPERTIES(
  vtkFluorescencePolyDataMapper.cxx
ABSTRACT
)

IF(BUILD_WITH_CUDA)
  # Include CUDA files
  INCLUDE_DIRECTORIES( ${CUDA_INCLUDE} )
  LINK_DIRECTORIES( ${CUDA_TARGET_LINK} )
  CUDA_INCLUDE_DIRECTORIES( ${VTKMS_SOURCE_DIR}/include )
  LINK_LIBRARIES( cufft.lib )
ENDIF(BUILD_WITH_CUDA)


# Create the msvtkRendering C++ library
IF(BUILD_WITH_CUDA)
  CUDA_ADD_LIBRARY(msvtkRendering ${Rendering_SRCS} ${RenderingCUDA_SRCS})
ELSE(BUILD_WITH_CUDA)
  ADD_LIBRARY (msvtkRendering ${Rendering_SRCS})
ENDIF(BUILD_WITH_CUDA)
TARGET_LINK_LIBRARIES(msvtkRendering
  ${VTK_LIBRARIES}
)
