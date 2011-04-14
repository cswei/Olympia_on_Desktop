/*
 * Copyright (C) Research In Motion, Limited 2009-2010. All rights reserved.
 */

#ifndef Olympia_Platform_RSS20Parser_h
#define Olympia_Platform_RSS20Parser_h

#include "FeedParser.h"

class RSS20Parser : public FeedParser {
public:
    RSS20Parser();

    bool parseBuffer(const char* buffer, int length, const char* url, const char* encoding);

private:
    bool parseXmlDoc(xmlDocPtr doc);
    Feed* parseFeed(xmlNode* node);
    Item* parseItem(Feed* feed, xmlNode* node);
    Enclosure* parseEnclosure(xmlNode* node);
};

#endif // Olympia_Platform_RSS20Parser_h
