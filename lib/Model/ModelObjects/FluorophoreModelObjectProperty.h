#ifndef _FLUOROHORE_MODEL_OBJECT_PROPERTY_H_
#define _FLUOROHORE_MODEL_OBJECT_PROPERTY_H_

#include <ModelObjectProperty.h>
#include <FluorophoreModelChannels.h>

#include <vtkSmartPointer.h>

class vtkAlgorithm;


class FluorophoreModelObjectProperty : public ModelObjectProperty {

 public:

  FluorophoreModelObjectProperty(const std::string& name,
                                 bool editable = false,
                                 bool optimizable = true);

  virtual ~FluorophoreModelObjectProperty();

  void SetEnabled(bool enabled);
  bool GetEnabled();

  void SetFluorophoreChannel(FluorophoreChannelType type);
  void SetFluorophoreChannelToRed();
  void SetFluorophoreChannelToGreen();
  void SetFluorophoreChannelToBlue();
  void SetFluorophoreChannelToAll();
  FluorophoreChannelType GetFluorophoreChannel();

  virtual int GetNumberOfFluorophores() = 0;

  vtkAlgorithm* GetFluorophoreOutput();

  virtual void RegenerateFluorophores() {};

  virtual void GetXMLConfiguration(xmlNodePtr root);
  virtual void RestoreFromXML(xmlNodePtr root);

 protected:
  FluorophoreModelObjectProperty();

  bool                   m_Enabled;
  FluorophoreChannelType m_Channel;

  vtkSmartPointer<vtkAlgorithm> m_FluorophoreOutput;

};

#endif // _FLUOROHORE_MODEL_OBJECT_PROPERTY_H_
