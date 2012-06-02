#ifndef _POINT_SPREAD_FUNCTION_LIST_H_
#define _POINT_SPREAD_FUNCTION_LIST_H_

#include <list>

#include <XMLStorable.h>

class PointSpreadFunction;


class PointSpreadFunctionList : public XMLStorable {
 public:
  PointSpreadFunctionList();
  virtual ~PointSpreadFunctionList();

  PointSpreadFunction* AddGaussianPointSpreadFunction(const std::string& name);
  PointSpreadFunction* AddGibsonLanniWidefieldPointSpreadFunction(const std::string& name);
  PointSpreadFunction* AddModifiedGibsonLanniWidefieldPointSpreadFunction(const std::string& name);
  PointSpreadFunction* AddHaeberlieWidefieldPointSpreadFunction(const std::string& name);
  PointSpreadFunction* ImportPointSpreadFunction(const std::string& fileName);
  void DeletePointSpreadFunction(int index);

  int GetSize();

  PointSpreadFunction* GetPointSpreadFunctionAt(int index);
  void DeletePointSpreadFunctionAt(int index);

  void SetPointSpreadFunctionName(int index, const std::string& name);

  virtual void GetXMLConfiguration(xmlNodePtr node);
  virtual void RestoreFromXML(xmlNodePtr node);

 protected:
  std::list<PointSpreadFunction*> m_PSFList;

  bool IsNameUnique(int index, const std::string& name);
  std::string GetUniqueName(int index, const std::string& name);

  typedef std::list<PointSpreadFunction*>::iterator Iterator;
};

#endif // _POINT_SPREAD_FUNCTION_LIST_H_
