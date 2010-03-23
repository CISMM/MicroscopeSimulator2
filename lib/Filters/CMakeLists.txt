SET(filterSrc
  PassThroughFilter.cxx
  itkFluorescenceImageSource.cxx
  itkGibsonLanniPSFImageSource.cxx
  itkImageToParameterizedImageSourceMetric.cxx
  itkParameterizedImageSource.cxx
  itkPoissonNoiseImageToImageMetric.cxx
)

ADD_LIBRARY(msimFilter ${filterSrc})
