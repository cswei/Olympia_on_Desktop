/*
 * Copyright (C) 2007 Staikos Computing Services Inc.
 * Copyright (C) 2007 Holger Hans Peter Freyther
 * Copyright (C) 2008 Apple, Inc. All rights reserved.
 * Copyright (C) 2008 Collabora, Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "FileSystem.h"

#include "OlympiaPlatformSettings.h"
#include "PlatformString.h"
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFSFileEngine>
#include <wtf/text/CString.h>

namespace Olympia {
namespace Platform {

using namespace WebCore;

bool fileExists(const char* path)
{
    return QFile::exists(QString::fromLatin1(path));
}

bool deleteFile(const char* path)
{
    return QFile::remove(QString::fromLatin1(path));
}

bool deleteEmptyDirectory(const char* path)
{
    return QDir::root().rmdir(QString::fromLatin1(path));
}

bool getFileSize(const char* filename, long long& size)
{
    QFileInfo info(QString::fromLatin1(filename));
    size = info.size();
    return info.exists();
}

bool getFileModificationTime(const char* filename, time_t& time)
{
    QFileInfo info(QString::fromLatin1(filename));
    time = info.lastModified().toTime_t();
    return info.exists();
}


bool listDirectory(const char* path, const char* filter, std::vector<std::string>& lists)
{
    if (!QDir(QString::fromLatin1(path)).exists())
        return false;

    QStringList nameFilters;
    if (filter != NULL)
        nameFilters.append(QString::fromLatin1(filter));
    QFileInfoList fileInfoList = QDir(QString::fromLatin1(path)).entryInfoList(nameFilters, QDir::AllEntries | QDir::NoDotAndDotDot);
    foreach (const QFileInfo fileInfo, fileInfoList) {
        std::string entry = fileInfo.canonicalFilePath().toStdString();
        lists.push_back(entry);
    }

    return true;
}

std::string openTemporaryFile(const char* prefix, FileHandle& handle)
{
    std::string path = Settings::get()->localUserSpecificStorageDirectory();
    path.append(prefix);
    QFile file(QString::fromStdString(path));
    if (file.open(QIODevice::ReadWrite)) {
        handle = file.handle();
        return path;
    } else
        return std::string();
}

void closeFile(FileHandle& handle)
{
    QFSFileEngine engine;
    if (!engine.open(QIODevice::ReadOnly, handle))
        return;

    if (engine.close())
        handle = invalidPlatformFileHandle;
}

int writeToFile(FileHandle handle, const char* data, int length)
{
    QFSFileEngine engine;
    if (!engine.open(QIODevice::WriteOnly, handle))
        return 0;
    return engine.write(data, length);
}

bool createDirectory(const char* path)
{
    if (QDir(QString::fromLatin1(path)).exists())
        return false;

    QDir dir(QString::fromLatin1(path));
    return dir.mkpath(QString::fromLatin1(path));
}

bool FileWrite(FileHandle handle, const char* data, size_t length)
{
    QFSFileEngine engine;
    if (engine.open(QIODevice::WriteOnly, handle))
        return false;
    if (engine.write(data, length) == length)
        return true;
    else
        return false;
}

void FileClose(FileHandle handle)
{
    QFSFileEngine engine;
    if (engine.open(QIODevice::ReadOnly, handle))
        return;
    engine.close();
}

}  // Platform
}  //Olympia

// vim: ts=4 sw=4 et
