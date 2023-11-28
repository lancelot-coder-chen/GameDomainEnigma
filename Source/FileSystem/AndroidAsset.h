﻿/*********************************************************************
 * \file   AndroidAsset.h
 * \brief  
 * 
 * \author Lancelot 'Robin' Chen
 * \date   May 2022
 *********************************************************************/
#ifndef _ANDROID_ASSET_H
#define _ANDROID_ASSET_H

#include "Platforms/PlatformLayerUtilities.h"

#if TARGET_PLATFORM == PLATFORM_ANDROID
#include "ReadWriteOption.h"
#include "IFile.h"
#include <android/asset_manager.h>
namespace Enigma::FileSystem
{
    class AndroidAsset : public IFile
    {
    public:
        AndroidAsset();
        AndroidAsset(const AndroidAsset&) = delete;
        AndroidAsset(const std::string& filename, const ReadWriteOption& rw_option);
        virtual ~AndroidAsset();
        AndroidAsset& operator=(const AndroidAsset&) = delete;

        virtual std::string GetFullPath() override { return m_fullPath; };

        virtual std::optional<std::vector<unsigned char>> Read(size_t offset, size_t size_request) override;
        virtual size_t Write(size_t offset, const std::vector<unsigned char>& in_buff) override;

        virtual size_t Size() override;

        virtual time_t FileTime() override { return 0; };

        virtual bool IsExisted() override;
        virtual bool IsWritable() override { return false; };

    protected:
        virtual error open() override;
        virtual error close() override;

    private:
        AAsset* m_aasset;
        std::string m_filename;
        std::string m_fullPath;
        ReadWriteOption m_rwOption;
        static std::mutex m_allAssetLocker;
    };
}


#endif // TARGET_PALTFORM == PLATFORM_ANDROID

#endif