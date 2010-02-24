SET(filterSrc
  PassThroughFilter.cxx
  itkGibsonLanniPSFImageSource.cxx
  itkParameterizedImageSource.cxx
)

ADD_LIBRARY(msimFilter ${filterSrc})
