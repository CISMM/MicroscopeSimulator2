#ifndef _XML_HELPER_H_
#define _XML_HELPER_H_

#include <vtkStdString.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xpointer.h>

xmlNodePtr
xmlGetFirstElementChildWithName(xmlNodePtr parent, xmlChar* name);

char *
xmlGetElementText(xmlNodePtr node);

#endif // _XML_HELPER_H_