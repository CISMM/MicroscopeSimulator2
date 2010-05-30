#ifndef _FLUOROHORE_MODEL_OBJECT_PROPERTY_H_
#define _FLUOROHORE_MODEL_OBJECT_PROPERTY_H_

#include <ModelObjectProperty.h>
#include <FluorophoreModelTypes.h>

#include <vtkSmartPointer.h>

class vtkPolyDataAlgorithm;


class FluorophoreModelObjectProperty : public ModelObjectProperty {

 public:

  FluorophoreModelObjectProperty(const std::string& name,
                                 vtkPolyDataAlgorithm* geometry,
                                 bool editable = false,
                                 bool optimizable = true);

  virtual ~FluorophoreModelObjectProperty();

  virtual void SetDensity(double density);
  virtual double GetDensity();

  void SetEnabled(bool enabled);
  bool GetEnabled();

  void SetFluorophoreChannel(FluorophoreChannelType type);
  void SetFluorophoreChannelToRed();
  void SetFluorophoreChannelToGreen();
  void SetFluorophoreChannelToBlue();
  void SetFluorophoreChannelToAll();
  FluorophoreChannelType GetFluorophoreChannel();

  vtkPolyDataAlgorithm* GetGeometry();

  vtkPolyDataAlgorithm* GetFluorophoreOutput();

  virtual void GetXMLConfiguration(xmlNodePtr root);
  virtual void RestoreFromXML(xmlNodePtr root);

 protected:
  FluorophoreModelObjectProperty();

  vtkPolyDataAlgorithm*  m_GeometrySource;
  double                 m_Density;
  bool                   m_Enabled;
  FluorophoreChannelType m_Channel;

  vtkSmartPointer<vtkPolyDataAlgorithm> m_FluorophoreOutput;

};

#endif // _FLUOROHORE_MODEL_OBJECT_PROPERTY_H_
