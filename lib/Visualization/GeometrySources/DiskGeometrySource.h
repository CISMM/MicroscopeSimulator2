#ifndef _DISK_GEOMETRY_SOURCE_H_
#define _DISK_GEOMETRY_SOURCE_H_

#include <vtkSmartPointer.h>

#include "GeometrySource.h"

// Forward declarations
class vtkDiskSource;

class DiskGeometrySource : public GeometrySource {

 public:
  DiskGeometrySource(ModelObjectPtr modelObject);
  virtual ~DiskGeometrySource();

  virtual vtkAlgorithmOutput* GetPolyDataOutputPort();

  virtual void Update();

 protected:
  DiskGeometrySource() {};

  vtkSmartPointer<vtkDiskSource> m_DiskSource;
  
};

typedef DiskGeometrySource* DiskGeometrySourcePtr;


#endif // _DISK_GEOMETRY_SOURCE_H_
