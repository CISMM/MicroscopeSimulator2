SET(ioSrc
  ITKImageToVTKImage.cxx
  VTKImageToITKImage.cxx
)

ADD_LIBRARY(msimIO ${ioSrc})
