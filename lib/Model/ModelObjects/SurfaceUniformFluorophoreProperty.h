#ifndef _SURFACE_UNIFORM_FLUOROPHORE_PROPERTY_H_
#define _SURFACE_UNIFORM_FLUOROPHORE_PROPERTY_H_

#include <UniformFluorophoreProperty.h>

class vtkPolyDataAlgorithm;
class vtkSurfaceUniformPointSampler;


class SurfaceUniformFluorophoreProperty : public UniformFluorophoreProperty {

 public:
  SurfaceUniformFluorophoreProperty(const std::string& name,
                                    vtkPolyDataAlgorithm* surfaceSource,
                                    bool editable = false,
                                    bool optimizable = true);
  virtual ~SurfaceUniformFluorophoreProperty();

  double GetGeometryArea();

 protected:
  SurfaceUniformFluorophoreProperty() {};

  virtual double GetDensityScale();

  vtkSmartPointer<vtkPolyDataAlgorithm>          m_SurfaceSource;
  vtkSmartPointer<vtkSurfaceUniformPointSampler> m_SurfaceSampler;
};


#endif // _SURFACE_UNIFORM_FLUOROPHORE_PROPERTY_H_
