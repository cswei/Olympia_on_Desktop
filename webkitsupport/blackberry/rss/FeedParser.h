/*
 * Copyright (C) Research In Motion, Limited 2009-2010. All rights reserved.
 */

#ifndef Olympia_Platform_FeedParser_h
#define Olympia_Platform_FeedParser_h

#include "SharedPointer.h"
#include "libxml/tree.h"
#include <memory>
#include <string>
#include <vector>

class FeedParser {
public:
    FeedParser();
    virtual ~FeedParser();

    virtual bool parseBuffer(const char* buffer, int length, const char* url, const char* encoding) = 0;

    struct Item;
    struct Enclosure;

    struct Feed {
        Feed() {}
        void clear();
        std::string title;
        std::string link;
        std::string description;
        std::string updated;
        std::string author;
        std::string pubDate;
        std::string id;
        std::string language;
        std::string ttl;
        std::vector<Item*> items;
    };

    struct Item {
        Item(Feed* parent = 0) : feed(parent) { if (feed) feed->items.push_back(this); }
        void clear();
        Feed* feed;
        std::string title;
        std::string link;
        std::string description;
        std::string updated;
        std::string author;
        std::string pubDate;
        std::string id;
        std::string comments;
        std::vector< std::string > categories;
        std::auto_ptr<Enclosure> enclosure;
    };

    struct Enclosure {
        enum EType {
            TypeUnknown,
            TypeImage,
            TypeAudio,
            TypeVideo,
            TypeApplication,
            TypeUnsupported
        };

        Enclosure()
            : m_enumType(TypeUnknown)
        {};

        EType baseType();
        std::string suggestedName();

        std::string url;
        std::string type;
        std::string length;

    private:
        std::string baseTypeString();

        EType m_enumType;
    };

    Feed* m_root;

};

#endif // Olympia_Platform_FeedParser_h
