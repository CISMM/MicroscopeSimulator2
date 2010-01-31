#ifndef _APPEND_GEOMETRY_SOURCE_H_
#define _APPEND_GEOMETRY_SOURCE_H_

#include <list>
#include <vtkSmartPointer.h>

#include <GeometrySource.h>



// Forward declarations
class vtkAlgorithmOutput;
class vtkAppendPolyData;

class AppendGeometrySource : public GeometrySource {

public:
  AppendGeometrySource(ModelObjectPtr modelObject);
  virtual ~AppendGeometrySource();

  virtual void AddGeometrySource(GeometrySource* source);

  virtual vtkAlgorithmOutput* GetPolyDataOutputPort();

  virtual void Update();

protected:
  AppendGeometrySource() {};

  std::list<GeometrySource*> m_Sources;

  vtkSmartPointer<vtkAppendPolyData> m_Appender;

};

#endif // _APPEND_GEOMETRY_SOURCE_H_
