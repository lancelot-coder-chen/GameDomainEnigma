﻿#include "AndroidMountPath.h"
#include "AndroidAsset.h"
#include "MemoryMacro.h"
#include <cassert>

Enigma::AndroidMountPath::AndroidMountPath(const std::string& sub_path, const std::string& path_id)
    : IMountPath(path_id)
{
    m_subPath = sub_path;
    if (m_subPath.length() > 0)
    {
        if (m_subPath[m_subPath.length() - 1] != '/')
        {
            m_subPath += "/";
        }
    }
}

Enigma::AndroidMountPath::~AndroidMountPath()
{
}

#pragma warning(disable:4100)  // for unused parameter rw_option in VC++
Enigma::IFile* Enigma::AndroidMountPath::CreateFile(const std::string& filename, const std::string& rw_option)
{
    if (filename.length() == 0) return nullptr;
    IFile* file = nullptr;
#if TARGET_PLATFORM == PLATFORM_ANDROID
    file = menew AndroidAsset(m_subPath + filename, rw_option);
#endif
    return file;
}
#pragma warning(default:4100)

bool Enigma::AndroidMountPath::EqualMountPath(IMountPath* path)
{
    assert(path);
    if (!EqualPathID(path->GetPathID())) return false;
    AndroidMountPath* pkg_path = dynamic_cast<AndroidMountPath*>(path);
    if (!pkg_path) return false;
    if (pkg_path->m_subPath != m_subPath) return false;
    return true;
}

bool Enigma::AndroidMountPath::EqualMountPath(const std::filesystem::path& path)
{
    return EqualMountPath(path.string());
}

bool Enigma::AndroidMountPath::EqualMouthPath(const std::string& path)
{
    return m_subPath == path;
}