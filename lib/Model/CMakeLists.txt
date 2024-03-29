SET(modelSrc
  DirtyListener.h
  Simulation.h
  Simulation.cxx
  XMLHelper.h
  XMLHelper.cxx
  XMLStorable.h
)

SET(modelModelObjectsSrc
  ModelObjects/ModelObject.h
  ModelObjects/ModelObject.cxx
  ModelObjects/ModelObjectFactory.h
  ModelObjects/ModelObjectFactory.cxx
  ModelObjects/ModelObjectList.h
  ModelObjects/ModelObjectList.cxx
  ModelObjects/ModelObjectProperty.h
  ModelObjects/ModelObjectProperty.cxx
  ModelObjects/FluorophoreModelObjectProperty.h
  ModelObjects/FluorophoreModelObjectProperty.cxx
  ModelObjects/GeometryVerticesFluorophoreProperty.h
  ModelObjects/GeometryVerticesFluorophoreProperty.cxx
  ModelObjects/UniformFluorophoreProperty.h
  ModelObjects/UniformFluorophoreProperty.cxx
  ModelObjects/SurfaceUniformFluorophoreProperty.h
  ModelObjects/SurfaceUniformFluorophoreProperty.cxx
  ModelObjects/VolumeUniformFluorophoreProperty.h
  ModelObjects/VolumeUniformFluorophoreProperty.cxx
  ModelObjects/GridBasedFluorophoreProperty.h
  ModelObjects/GridBasedFluorophoreProperty.cxx
  ModelObjects/ModelObjectPropertyList.h
  ModelObjects/ModelObjectPropertyList.cxx
  ModelObjects/CylinderModelObject.h
  ModelObjects/CylinderModelObject.cxx
  ModelObjects/DiskModelObject.h
  ModelObjects/DiskModelObject.cxx
  ModelObjects/EllipsoidModelObject.h
  ModelObjects/EllipsoidModelObject.cxx
  ModelObjects/FlexibleTubeModelObject.h
  ModelObjects/FlexibleTubeModelObject.cxx
  ModelObjects/HollowCylinderModelObject.h
  ModelObjects/HollowCylinderModelObject.cxx
  ModelObjects/PlaneModelObject.h
  ModelObjects/PlaneModelObject.cxx
  ModelObjects/PointRingModelObject.h
  ModelObjects/PointRingModelObject.cxx
  ModelObjects/PointSetModelObject.h
  ModelObjects/PointSetModelObject.cxx
  ModelObjects/SphereModelObject.h
  ModelObjects/SphereModelObject.cxx
  ModelObjects/TorusModelObject.h
  ModelObjects/TorusModelObject.cxx
  ModelObjects/ImageModelObject.h
  ModelObjects/ImageModelObject.cxx
  ModelObjects/ImportedGeometryModelObject.h
  ModelObjects/ImportedGeometryModelObject.cxx
)

SET(modelAFMSimSrc
  AFMSim/AFMSimulation.h
  AFMSim/AFMSimulation.cxx
)

SET(modelFluoroSimSrc
  FluoroSim/FluorescenceSimulation.h
  FluoroSim/FluorescenceSimulation.cxx
  FluoroSim/FluorescenceOptimizer.h
  FluoroSim/FluorescenceOptimizer.cxx
  FluoroSim/ITKFluorescenceOptimizer.h
  FluoroSim/ITKFluorescenceOptimizer.cxx
  FluoroSim/GradientDescentFluorescenceOptimizer.h
  FluoroSim/GradientDescentFluorescenceOptimizer.cxx
  FluoroSim/NelderMeadFluorescenceOptimizer.h
  FluoroSim/NelderMeadFluorescenceOptimizer.cxx
  FluoroSim/PointsGradientFluorescenceOptimizer.h
  FluoroSim/PointsGradientFluorescenceOptimizer.cxx
  FluoroSim/PointSpreadFunction.h
  FluoroSim/PointSpreadFunction.cxx
  FluoroSim/GaussianPointSpreadFunction.h
  FluoroSim/GaussianPointSpreadFunction.cxx
  FluoroSim/ImportedPointSpreadFunction.h
  FluoroSim/ImportedPointSpreadFunction.cxx
  FluoroSim/GibsonLanniWidefieldPointSpreadFunction.h
  FluoroSim/GibsonLanniWidefieldPointSpreadFunction.cxx
  FluoroSim/ModifiedGibsonLanniWidefieldPointSpreadFunction.h
  FluoroSim/ModifiedGibsonLanniWidefieldPointSpreadFunction.cxx
  FluoroSim/HaeberleWidefieldPointSpreadFunction.h
  FluoroSim/HaeberleWidefieldPointSpreadFunction.cxx
  FluoroSim/PointSpreadFunctionList.h
  FluoroSim/PointSpreadFunctionList.cxx
)

ADD_LIBRARY(msimModel ${modelSrc} ${modelModelObjectsSrc} ${modelAFMSimSrc} ${modelFluoroSimSrc})

TARGET_LINK_LIBRARIES( msimModel
  msvtkGraphics
  msvtkFiltering
  msvtkImaging
  msimIO
  msimUtilities
  ${ITK_LIBRARIES}
  ${VTK_LIBRARIES}
  ${LIBXML2_LIBRARIES}
)
