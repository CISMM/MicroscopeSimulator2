SET(visualizationSrc
  ImagePlaneVisualizationPipeline.cxx
  IsosurfaceVisualizationPipeline.cxx
  OutlineVisualizationPipeline.cxx
  VisualizationPipeline.cxx
  Visualization.h
  Visualization.cxx
  VisualizationFluorescenceImageSource.h
  VisualizationFluorescenceImageSource.cxx
)

SET(visualizationViewsSrc
  vtkModelObjectRepresentation.h
  vtkModelObjectRepresentation.cxx
  GeometryView/GeometryRepresentation.h
  GeometryView/GeometryRepresentation.cxx
  GeometryView/vtkGeometryRenderView.h
  GeometryView/vtkGeometryRenderView.cxx
  GeometryView/vtkModelObjectGeometryRepresentation.h
  GeometryView/vtkModelObjectGeometryRepresentation.cxx
  GeometryView/vtkModelObjectActorPickObserver.h
  GeometryView/vtkModelObjectActorPickObserver.cxx
  GeometryView/vtkVisualizationInteractionObserver.h
  GeometryView/vtkVisualizationInteractionObserver.cxx
  GeometryView/vtkFluorescenceWidgetsRepresentation.h
  GeometryView/vtkFluorescenceWidgetsRepresentation.cxx
  GeometryView/vtkImagePlaneWidgetRepresentation.h
  GeometryView/vtkImagePlaneWidgetRepresentation.cxx
  GeometryView/vtkReferenceGridSource.h
  GeometryView/vtkReferenceGridSource.cxx
  FluorescenceView/FluorescenceRepresentation.h
  FluorescenceView/FluorescenceRepresentation.cxx
  FluorescenceView/vtkModelObjectFluorescenceRepresentation.h
  FluorescenceView/vtkModelObjectFluorescenceRepresentation.cxx
  FluorescenceView/vtkFluorescenceRenderView.h
  FluorescenceView/vtkFluorescenceRenderView.cxx
)

ADD_LIBRARY( msimVisualization 
  ${visualizationSrc}
  ${visualizationViewsSrc}
)

TARGET_LINK_LIBRARIES( msimVisualization
  msvtkFiltering
  msvtkImaging
  msvtkRendering
  msimModel
)