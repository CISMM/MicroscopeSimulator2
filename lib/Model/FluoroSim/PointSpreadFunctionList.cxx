#include <GaussianPointSpreadFunction.h>
#include <ImportedPointSpreadFunction.h>
#include <WidefieldPointSpreadFunction.h>
#include <PointSpreadFunctionList.h>

#include <libxml/tree.h>

PointSpreadFunctionList
::PointSpreadFunctionList() {
}


PointSpreadFunctionList
::~PointSpreadFunctionList() {
  // Delete all PSFs
  Iterator iter;
  for (iter = m_PSFList.begin(); iter != m_PSFList.end(); iter++) {
    delete *iter;
  }
}


PointSpreadFunction*
PointSpreadFunctionList
::AddGaussianPointSpreadFunction(const std::string& name) {
  GaussianPointSpreadFunction* psf = new GaussianPointSpreadFunction();
  psf->SetName(GetUniqueName(-1, name));
  m_PSFList.push_back(psf);
  
  return psf;
}


PointSpreadFunction*
PointSpreadFunctionList
::AddWidefieldPointSpreadFunction(const std::string& name) {
  WidefieldPointSpreadFunction* psf = new WidefieldPointSpreadFunction();
  psf->SetName(GetUniqueName(-1, name));
  m_PSFList.push_back(psf);

  return psf;
}


PointSpreadFunction*
PointSpreadFunctionList
::ImportPointSpreadFunction(const std::string& fileName) {
  ImportedPointSpreadFunction* psf = new ImportedPointSpreadFunction();
  psf->SetName(GetUniqueName(-1, fileName));
  psf->SetFileName(fileName);
  m_PSFList.push_back(psf);

  return psf;
}


void
PointSpreadFunctionList
::DeletePointSpreadFunction(int index) {
  std::list<PointSpreadFunction*>::iterator iter = m_PSFList.begin();
  for (int i = 0; i < index; i++) {
    iter++;
  }
  delete *iter;;
  m_PSFList.erase(iter);
}


void
PointSpreadFunctionList
::SetPointSpreadFunctionName(int index, const std::string& name) {
  GetPointSpreadFunctionAt(index)->SetName(GetUniqueName(index, name));
}


int
PointSpreadFunctionList
::GetSize() {
  return static_cast<int>(m_PSFList.size());
}


PointSpreadFunction*
PointSpreadFunctionList
::GetPointSpreadFunctionAt(int index) {
  PointSpreadFunction* psf = NULL;
  int i = 0;
  Iterator iter;
  for (iter = m_PSFList.begin(); iter != m_PSFList.end(); iter++, i++) {
    if (i == index) {
      return *iter;
    } 
  }


  return psf;
}


void
PointSpreadFunctionList
::GetXMLConfiguration(xmlNodePtr node) {
  Iterator iter;
  for (iter = m_PSFList.begin(); iter != m_PSFList.end(); iter++) {
    (*iter)->GetXMLConfiguration(node);
  }
}


void
PointSpreadFunctionList
::RestoreFromXML(xmlNodePtr node) {
  // Iterate over all children and import
  xmlNodePtr psfNode = node->children;
  while (psfNode != NULL) {
    if (psfNode->type == XML_ELEMENT_NODE) {
      std::string nodeName(reinterpret_cast<const char*>(psfNode->name));
      if (nodeName == "GaussianPointSpreadFunction") {
        const char* name = reinterpret_cast<const char*>(xmlGetProp(psfNode, BAD_CAST "name"));
        PointSpreadFunction* psf = AddGaussianPointSpreadFunction(name);
        psf->RestoreFromXML(psfNode);
      } else if (nodeName == "ImportedPointSpreadFunction") {
        // TODO - fix this up
        //ImportedPointSpreadFunction("tmp");
      } else if (nodeName == "WidefieldPointSpreadFunction") {
        const char* name = reinterpret_cast<const char*>(xmlGetProp(psfNode, BAD_CAST "name"));
        PointSpreadFunction* psf = AddWidefieldPointSpreadFunction(name);
        psf->RestoreFromXML(psfNode);
      }
    }
    psfNode = psfNode->next;
  }

}


bool
PointSpreadFunctionList
::IsNameUnique(int index, const std::string& name) {
  int i = 0;
  Iterator iter;
  for (iter = m_PSFList.begin(); iter != m_PSFList.end(); iter++, i++) {
    if (index != i) {
      if (name == (*iter)->GetName())
        return false;
    }
  }
  return true;
}


std::string
PointSpreadFunctionList
::GetUniqueName(int index, const std::string& name) {
  if (IsNameUnique(index, name))
    return name;

  int i = 1;
  while (true) {
    char buf[1024];
    sprintf(buf, "%s - %d", name.c_str(), i);
    std::string testName(buf);
    if (IsNameUnique(index, testName)) {
      return testName;
    }
    i++;
  }

  return name;
}
