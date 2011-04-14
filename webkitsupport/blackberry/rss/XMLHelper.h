/*
 * Copyright (C) 2010 Research In Motion Limited. http://www.rim.com/
 */

#ifndef Olympia_Platform_XmlHelper_h
#define Olympia_Platform_XmlHelper_h

#include <string>
#include "libxml/tree.h"

std::string encodeAttrText(const std::string& input, xmlCharEncodingHandlerPtr handler);
std::string decodeAttrText(const std::string& src, xmlCharEncodingHandlerPtr handler);
std::string attrText(xmlAttr *a, xmlCharEncodingHandlerPtr handler);
std::string allText(xmlNode *cur, xmlCharEncodingHandlerPtr handler);
bool isRelativePath(const std::string& path);

#endif // Olympia_Platform_XmlHelper_h

