#ifndef _POINT_SPREAD_FUNCTION_H_
#define _POINT_SPREAD_FUNCTION_H_

#include <string>

#include <XMLStorable.h>

class vtkAlgorithmOutput;
class vtkImageData;


class PointSpreadFunction : public XMLStorable {
 public:
  static const std::string NAME_ATTRIBUTE;
  static const std::string SIZE_ELEMENT;
  static const std::string SPACING_ELEMENT;
  static const std::string X_ATTRIBUTE;
  static const std::string Y_ATTRIBUTE;
  static const std::string Z_ATTRIBUTE;


  PointSpreadFunction();
  virtual ~PointSpreadFunction();

  void SetName(const std::string& name);
  std::string& GetName();

  virtual vtkImageData* GetOutput() = 0;
  virtual vtkAlgorithmOutput* GetOutputPort() = 0;

  virtual int GetNumberOfProperties() = 0;
  virtual std::string GetParameterName(int index) = 0;
  virtual double      GetParameterValue(int index) = 0;
  virtual void SetParameterValue(int index, double value) = 0;

  virtual void RestoreFromXML(xmlNodePtr node) = 0;

 protected:
  std::string m_Name;
};


#endif // _POINT_SPREAD_FUNCTION_H_
