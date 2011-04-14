/*
 * Copyright (C) Research In Motion, Limited 2009-2010. All rights reserved.
 */

#ifndef Olympia_Platform_AtomParser_h
#define Olympia_Platform_AtomParser_h

#include "FeedParser.h"

class AtomParser : public FeedParser {
public:
    struct Link {
        enum EType {
            TypeUnknown,
            TypeAlternate,
            TypeRelated,
            TypeSelf,
            TypeEnclosure,
            TypeVia,
            TypeUnsupported
        };

        Link()
            : m_enumType(TypeUnknown)
        {};

        EType relType();

        std::string rel;
        std::string href;
        std::string hreflang;
        std::string type;
        std::string title;
        std::string length;

    private:
        EType m_enumType;
    };

    AtomParser();

    bool parseBuffer(const char* buffer, int length, const char* url, const char* encoding);

private:
    bool parseXmlDoc(xmlDocPtr doc);
    FeedParser::Feed* parseFeed(xmlNode* rootNode);
    FeedParser::Item* parseItem(Feed* feed, xmlNode* node);
    Link* parseLink(xmlNode* node);
    Enclosure* enclosureFromLink(Link* link);
    std::string parseContent(const std::string& base, xmlNode* node);
    std::string parseAuthor(xmlNode* node);
    std::string parseCategory(xmlNode* node);
};

#endif // Olympia_Platform_AtomParser_h
