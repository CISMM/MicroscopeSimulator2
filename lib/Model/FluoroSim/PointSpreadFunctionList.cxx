#pragma warning(disable : 4996)

#include <GaussianPointSpreadFunction.h>
#include <ImportedPointSpreadFunction.h>
#include <GibsonLanniWidefieldPointSpreadFunction.h>
#include <ModifiedGibsonLanniWidefieldPointSpreadFunction.h>
#include <HaeberleWidefieldPointSpreadFunction.h>
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
::AddGibsonLanniWidefieldPointSpreadFunction(const std::string& name) {
  GibsonLanniWidefieldPointSpreadFunction* psf =
    new GibsonLanniWidefieldPointSpreadFunction();
  psf->SetName(GetUniqueName(-1, name));
  m_PSFList.push_back(psf);

  return psf;
}


PointSpreadFunction*
PointSpreadFunctionList
::AddModifiedGibsonLanniWidefieldPointSpreadFunction(const std::string& name) {
  ModifiedGibsonLanniWidefieldPointSpreadFunction* psf =
    new ModifiedGibsonLanniWidefieldPointSpreadFunction();
  psf->SetName(GetUniqueName(-1, name));
  m_PSFList.push_back(psf);

  return psf;
}


PointSpreadFunction*
PointSpreadFunctionList
::AddHaeberlieWidefieldPointSpreadFunction(const std::string& name) {
  HaeberleWidefieldPointSpreadFunction* psf =
    new HaeberleWidefieldPointSpreadFunction();
  psf->SetName(GetUniqueName(-1, name));
  m_PSFList.push_back(psf);

  return psf;
}


PointSpreadFunction*
PointSpreadFunctionList
::ImportPointSpreadFunction(const std::string& name) {
  ImportedPointSpreadFunction* psf = new ImportedPointSpreadFunction();
  psf->SetName(GetUniqueName(-1, name));
  psf->SetFileName(name);
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
    // The XML from each PSF will be added to the node data structure
    // in the call below.
    (*iter)->GetXMLConfiguration(node);
  }
}


void
PointSpreadFunctionList
::RestoreFromXML(xmlNodePtr node) {
  char errName[] = {"No name given"};

  // Iterate over all children and import
  xmlNodePtr psfNode = node->children;
  while (psfNode != NULL) {
    if (psfNode->type == XML_ELEMENT_NODE) {
      std::string nodeName((const char*) psfNode->name);
      const char* name = (const char*) xmlGetProp(psfNode, BAD_CAST PointSpreadFunction::NAME_ATTRIBUTE.c_str());
      if (!name)
        name = errName;

      if (nodeName == GaussianPointSpreadFunction::PSF_ELEMENT) {
        PointSpreadFunction* psf = new GaussianPointSpreadFunction();
        psf->RestoreFromXML(psfNode);
        m_PSFList.push_back(psf);
      } else if (nodeName == "ImportedPointSpreadFunction") {
        ImportedPointSpreadFunction* psf = new ImportedPointSpreadFunction();
        psf->RestoreFromXML(psfNode);
        if (psf->IsFileValid()) {
          m_PSFList.push_back(psf);
        }
      } else if (nodeName == "WidefieldPointSpreadFunction" || nodeName == GibsonLanniWidefieldPointSpreadFunction::PSF_ELEMENT) {
        PointSpreadFunction* psf = new GibsonLanniWidefieldPointSpreadFunction();
        psf->RestoreFromXML(psfNode);
        m_PSFList.push_back(psf);
      } else if (nodeName == ModifiedGibsonLanniWidefieldPointSpreadFunction::PSF_ELEMENT) {
        PointSpreadFunction* psf = new ModifiedGibsonLanniWidefieldPointSpreadFunction();
        psf->RestoreFromXML(psfNode);
        m_PSFList.push_back(psf);
      } else if (nodeName == HaeberleWidefieldPointSpreadFunction::PSF_ELEMENT) {
        PointSpreadFunction* psf = new HaeberleWidefieldPointSpreadFunction();
        psf->RestoreFromXML(psfNode);
        m_PSFList.push_back(psf);
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
