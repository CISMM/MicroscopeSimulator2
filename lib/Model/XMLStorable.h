#ifndef _XML_STORABLE_H_
#define _XML_STORABLE_H_

#include <vtkStdString.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

class XMLStorable {

public:
  virtual ~XMLStorable() {};

  virtual void GetXMLConfiguration(xmlNodePtr node) = 0;

}; // class XMLStorable

#endif // _XML_STORABLE_H_
