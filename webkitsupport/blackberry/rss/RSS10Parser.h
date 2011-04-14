/*
 * Copyright (C) Research In Motion, Limited 2009-2010. All rights reserved.
 */

#ifndef Olympia_Platform_RSS10Parser_h
#define Olympia_Platform_RSS10Parser_h

#include "FeedParser.h"

class RSS10Parser : public FeedParser {
public:
    RSS10Parser();

    bool parseBuffer(const char* buffer, int length, const char* url, const char* encoding);

private:
    bool parseXmlDoc(xmlDocPtr doc);
    void parseFeed(Feed* feed, xmlNode* node);
    Item* parseItem(Feed* feed, xmlNode* node);
};

#endif // Olympia_Platform_RSS10Parser_h
