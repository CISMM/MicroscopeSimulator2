SET(ioSrc
  ImageReader.h
  ImageReader.cxx
  ImageWriter.h
  ImageWriter.cxx
  ITKImageToVTKImage.h
  ITKImageToVTKImage.cxx
  VTKImageToITKImage.h
  VTKImageToITKImage.cxx
)

ADD_LIBRARY(msimIO ${ioSrc})

TARGET_LINK_LIBRARIES(msimIO ${ITK_LIBRARIES})
