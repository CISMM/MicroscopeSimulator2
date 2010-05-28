#ifndef _FLUOROHORE_MODEL_OBJECT_PROPERTY_H_
#define _FLUOROHORE_MODEL_OBJECT_PROPERTY_H_

#include <ModelObjectProperty.h>
#include <FluorophoreModelTypes.h>

#include <vtkSmartPointer.h>

class vtkPolyDataAlgorithm;


class FluorophoreModelObjectProperty : public ModelObjectProperty {

 public:

  FluorophoreModelObjectProperty(const std::string& name,
                                 FluorophoreModelType type,
                                 vtkPolyDataAlgorithm* geometry,
                                 bool editable = false,
                                 bool optimizable = true);

  virtual ~FluorophoreModelObjectProperty();

  void SetFluorophoreModelType(FluorophoreModelType type);
  void SetFluorophoreModelTypeToGeometryVertices();
  void SetFluorophoreModelTypeToUniformRandomSurfaceSample();
  void SetFluorophoreModelTypeToUniformRandomVolumeSample();
  FluorophoreModelType GetFluorophoreModelType();

  void SetDensity(double density);
  double GetDensity();

  void SetEnabled(bool enabled);
  bool GetEnabled();

  void SetFluorophoreChannel(FluorophoreChannelType type);
  void SetFluorophoreChannelToRed();
  void SetFluorophoreChannelToGreen();
  void SetFluorophoreChannelToBlue();
  void SetFluorophoreChannelToAll();
  FluorophoreChannelType GetFluorophoreChannel();

  vtkPolyDataAlgorithm* GetGeometry();
  double GetGeometryArea();
  double GetGeometryVolume();

  vtkPolyDataAlgorithm* GetFluorophoreOutput();

  virtual void GetXMLConfiguration(xmlNodePtr root);
  virtual void RestoreFromXML(xmlNodePtr root);

 protected:
  FluorophoreModelObjectProperty();

  FluorophoreModelType   m_FluorophoreModelType;
  vtkPolyDataAlgorithm*  m_GeometrySource;
  double                 m_Density;
  bool                   m_Enabled;
  FluorophoreChannelType m_Channel;

  vtkSmartPointer<vtkPolyDataAlgorithm> m_FluorophoreOutput;

};

#endif // _FLUOROHORE_MODEL_OBJECT_PROPERTY_H_
