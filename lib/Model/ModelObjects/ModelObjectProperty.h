#ifndef _MODEL_OBJECT_PROPERTY_H_
#define _MODEL_OBJECT_PROPERTY_H_

#include <string>

#include <XMLStorable.h>


class ModelObjectProperty : public XMLStorable {
 public:

  typedef enum {
    BOOL_TYPE,
    INT_TYPE,
    DOUBLE_TYPE,
    STRING_TYPE,
    FLUOROPHORE_MODEL_TYPE
  } ModelObjectPropertyType;

  ModelObjectProperty(const std::string& name, ModelObjectPropertyType type,
                      const std::string& units = "-", bool editable = true,
                      bool optimizable = true);
  ModelObjectProperty(const std::string& name, bool value,
                      const std::string& units = "-", bool editable = true,
                      bool optimizable = true);
  ModelObjectProperty(const std::string& name, int value,
                      const std::string& units = "-", bool editable = true,
                      bool optimizable = true);
  ModelObjectProperty(const std::string& name, double value,
                      const std::string& units = "-", bool editable = true,
                      bool optimizable = true);
  ModelObjectProperty(const std::string& name, const std::string& value,
                      const std::string& units = "-", bool editable = true,
                      bool optimizable = true);
  virtual ~ModelObjectProperty();

  std::string& GetName();
  std::string  GetXMLElementName();

  virtual ModelObjectPropertyType GetType();

  bool IsEditable();
  bool IsOptimizable();

  void SetOptimize(bool optimize);
  bool GetOptimize();

  virtual void SetBoolValue(bool value);
  virtual bool GetBoolValue();

  virtual void SetIntValue(int value);
  virtual int  GetIntValue();

  virtual void   SetDoubleValue(double value);
  virtual double GetDoubleValue();

  void SetStringValue(const std::string& value);
  std::string& GetStringValue();

  std::string& GetUnits();

  virtual void Update() {}; // Default implementation is a noop.

  virtual void GetXMLConfiguration(xmlNodePtr root);
  virtual void RestoreFromXML(xmlNodePtr root);

 protected:
  std::string m_Name;
  
  bool m_Editable;

  bool m_Optimizable;

  bool m_Optimize;

  ModelObjectPropertyType m_Type;

  bool m_BoolValue;

  int m_IntValue;

  double m_DoubleValue;

  std::string m_StringValue;

  std::string m_Units;

 private:
  ModelObjectProperty();

  void Initialize(const std::string& name, ModelObjectPropertyType type,
                  const std::string& units, bool editable, bool optimizable);
};

typedef ModelObjectProperty* ModelObjectPropertyPtr;

#endif // _MODEL_OBJECT_PROPERTY_H_
