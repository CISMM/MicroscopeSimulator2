#ifndef _IMAGE_MODEL_OBJECT_H_
#define _IMAGE_MODEL_OBJECT_H_

#include <ImageIO.h>
#include <ModelObject.h>

#include <vtkImageChangeInformation.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>

class vtkContourFilter;


class ImageModelObject : public ModelObject {

 public:
  static const char* OBJECT_TYPE_NAME;

  static const char* FILE_NAME_PROP;
  static const char* X_SPACING_PROP;
  static const char* Y_SPACING_PROP;
  static const char* Z_SPACING_PROP;
  static const char* ISO_VALUE_PROP;

  ImageModelObject(DirtyListener* dirtyListener);
  virtual ~ImageModelObject();

  void LoadFile(const std::string& fileName);

  vtkImageData* GetImageData();

  void GetDimensions(int dim[3]);

  void GetSpacing(double spacing[3]);

  virtual void SetPosition(double position[3]);
  virtual void GetPosition(double position[3]);

  virtual void SetRotation(double rotation[4]);
  virtual void GetRotation(double rotation[4]);

  virtual void Update();

  virtual void RestoreFromXML(xmlNodePtr node);

 protected:
  ImageModelObject() {};

  ModelObjectPropertyList* CreateProperties();

  ImageIO*                                   m_ImageIO;
  vtkSmartPointer<vtkImageChangeInformation> m_InfoChanger;
  vtkSmartPointer<vtkContourFilter>          m_IsosurfaceSource;
};

typedef ImageModelObject* ImageModelObjectPtr;

#endif // _IMAGE_MODEL_OBJECT_H_
