SET(ioSrc
  ImageReader.h
  ImageReader.cxx
  ITKImageToVTKImage.h
  ITKImageToVTKImage.cxx
  VTKImageToITKImage.h
  VTKImageToITKImage.cxx
)

ADD_LIBRARY(msimIO ${ioSrc})
