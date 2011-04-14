/*
 * Copyright (C) 2009 Research In Motion Limited. http://www.rim.com/
 */

#ifndef OlympiaPlatformFileSystem_h
#define OlympiaPlatformFileSystem_h

#include <string>
#include <vector>

namespace Olympia {
    namespace Platform {

        enum {
            FileNameMaxLength = 256,
        };

        typedef int FileHandle;

        bool fileExists(const char* filename);

        bool getFileSize(const char* filename, long long& size);
        bool getFileModificationTime(const char* filename, time_t& time);

        bool deleteFile(const char* filename);
        bool deleteEmptyDirectory(const char* path);

        bool createDirectory(const char* path);
        std::string openTemporaryFile(const char* prefix, FileHandle& handle);

        bool FileWrite(FileHandle handle, const char* data, size_t length);
        void FileClose(FileHandle handle);

        bool listDirectory(const char* path, const char* filter, std::vector<std::string>& lists);
    } // namespace Olympia
} // namespace Platform

#endif // OlympiaPlatformFileSystem_h
