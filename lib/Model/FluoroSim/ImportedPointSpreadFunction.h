#ifndef _IMPORTED_POINT_SPREAD_FUNCTION_H_
#define _IMPORTED_POINT_SPREAD_FUNCTION_H_

#include <StdLibGuard.h>
#include <string>

#include <PointSpreadFunction.h>


class ImportedPointSpreadFunction : public PointSpreadFunction {
 public:
  ImportedPointSpreadFunction();
  virtual ~ImportedPointSpreadFunction();

  virtual void SetFileName(const std::string& fileName);
  virtual std::string GetFileName();

  void SetSpacing(double  x, double  y, double  z);
  void GetSpacing(double& x, double& y, double& z);

  void SetOrigin(double  x, double  y, double  z);
  void GetOrigin(double& x, double& y, double& z);

  void GetSize(int& ix, int& iy, int& iz);

  virtual vtkImageData* GetOutput();
  virtual vtkAlgorithmOutput* GetOutputPort();

  virtual int GetNumberOfProperties();
  std::string GetParameterName(int index);
  double      GetParameterValue(int index);
  virtual void SetParameterValue(int index, double value);

  virtual void GetXMLConfiguration(xmlNodePtr node);
  virtual void RestoreFromXML(xmlNodePtr node);

 protected:
  std::string m_FileName;
};


#endif // _POINT_SPREAD_FUNCTION_H_
