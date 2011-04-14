/*
 * Copyright (C) 2010 Research In Motion Limited. http://www.rim.com/
 */

#ifndef Olympia_Platform_RSSGenerator_h
#define Olympia_Platform_RSSGenerator_h

#include "FeedParser.h"

class RSSGenerator {
public:
    RSSGenerator();
    ~RSSGenerator();

    std::string generateHtml(FeedParser::Feed* feed);

    // TODO: add configuration for the styling, etc.
private:
    std::string stylesheet;
    std::string script;
};

#endif // Olympia_Platform_RSSGenerator_h
