SET(utilitiesSrc
  Matrix.h
  Matrix.cxx
  StringUtils.h
  StringUtils.cxx
)

ADD_LIBRARY(msimUtilities ${utilitiesSrc})

TARGET_LINK_LIBRARIES( msimUtilities
  lapack
)