#include "XMLHelper.h"

xmlNodePtr
xmlGetFirstElementChildWithName(xmlNodePtr parent, xmlChar* name) {
  if (parent->children == NULL)
    return parent->children;

  xmlNodePtr child = parent->children;
  if (!xmlStrcmp(child->name, name)) {
    return child;
  }

  while (child->next) {
    if (!xmlStrcmp(child->next->name, name)) {
      return child->next;
    } else {
      child = child->next;
    }
  }

  return NULL;
}


char*
xmlGetElementText(xmlNodePtr node) {
  if (node && node->children && node->children->type == XML_TEXT_NODE) {
    return (char *) node->children->content;
  }

  return NULL;
}