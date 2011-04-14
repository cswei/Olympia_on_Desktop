/*
 * Copyright (C) 2010 Research In Motion Limited. http://www.rim.com/
 */

#ifndef Olympia_Platform_StringHelper_h
#define Olympia_Platform_StringHelper_h

#include <string>
#include <SharedPointer.h>

std::string& trim(std::string& str);
std::string& trimLeft(std::string& str);
std::string& trimRight(std::string& str);
std::string& tolower(std::string& str);

std::wstring& trim(std::wstring& str);
std::wstring& trimLeft(std::wstring& str);
std::wstring& trimRight(std::wstring& str);
std::wstring& tolower(std::wstring& str);
bool startsWith(const std::string& str, const std::string& substr);
bool endsWith(const std::string& str, const std::string& substr);

struct TempCharModifier {
    TempCharModifier(char* p, char c)
        : m_dest(p)
        , m_old(*p)
    {
        *p = c;
    }
    ~TempCharModifier() { *m_dest = m_old; }

    char* m_dest;
    char m_old;
};

#endif // Olympia_Platform_StringHelper_h
