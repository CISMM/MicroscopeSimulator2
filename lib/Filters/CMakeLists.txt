SET(filterSrc
  PassThroughFilter.cxx
  itkFluorescenceImageSource.cxx
  itkGibsonLanniPSFImageSource.cxx
  itkImageToParameterizedImageSourceMetric.cxx
  itkParameterizedImageSource.cxx
)

ADD_LIBRARY(msimFilter ${filterSrc})
