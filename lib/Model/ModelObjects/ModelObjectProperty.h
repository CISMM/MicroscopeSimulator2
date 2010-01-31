#ifndef _MODEL_OBJECT_PROPERTY_H_
#define _MODEL_OBJECT_PROPERTY_H_

#include <string>


class ModelObjectProperty {
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

  virtual ModelObjectPropertyType GetType();

  bool IsEditable();

  bool IsOptimizable();

  void SetBoolValue(bool value);
  bool GetBoolValue();

  void SetIntValue(int value);
  int GetIntValue();

  void SetDoubleValue(double value);
  double GetDoubleValue();

  void SetStringValue(const std::string& value);
  std::string& GetStringValue();

  std::string& GetUnits();

 protected:
  std::string m_Name;
  
  bool m_Editable;

  bool m_Optimizable;

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
