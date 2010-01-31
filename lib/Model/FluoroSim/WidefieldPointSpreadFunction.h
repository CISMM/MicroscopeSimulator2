#ifndef _WIDEFIELD_POINT_SPREAD_FUNCTION_H_
#define _WIDEFIELD_POINT_SPREAD_FUNCTION_H_

#include <PointSpreadFunction.h>


class WidefieldPointSpreadFunction : public PointSpreadFunction {

 public:
  WidefieldPointSpreadFunction();
  virtual ~WidefieldPointSpreadFunction();

  virtual vtkImageData* GetOutput();
  virtual vtkAlgorithmOutput* GetOutputPort();

  virtual int GetNumberOfProperties();
  virtual std::string GetParameterName(int index);
  virtual double      GetParameterValue(int index);
  virtual void SetParameterValue(int index, double value);

  virtual void GetXMLConfiguration(xmlNodePtr node);
  virtual void RestoreFromXML(xmlNodePtr node);

 protected:

};

#endif // _WIDEFIELD_POINT_SPREAD_FUNCTION_H_
