﻿/*********************************************************************
 * \file   IMultiTexture.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   June 2022
 *********************************************************************/
#ifndef MULTI_TEXTURE_INTERFACE_H
#define MULTI_TEXTURE_INTERFACE_H

#include "ITexture.h"
#include "GraphicErrors.h"

namespace Enigma::Graphics
{
    using error = std::error_code;

    class IMultiTexture : public ITexture
    {
    public:
        IMultiTexture(const std::string& name);
        IMultiTexture(const IMultiTexture&) = delete;
        IMultiTexture(IMultiTexture&&) = delete;
        virtual ~IMultiTexture() override;
        IMultiTexture& operator=(const IMultiTexture&) = delete;
        IMultiTexture& operator=(IMultiTexture&&) = delete;

        virtual void MultiLoad(const std::vector<byte_buffer>& img_buffs);
        virtual void MultiLoad(const std::vector<std::string>& filenames, const std::vector<std::string>& pathids);
        virtual void MultiSave(const std::vector<FileSystem::IFilePtr>& files);
        virtual void MultiSave(const std::vector<std::string>& filenames, const std::vector<std::string>& pathids);

        virtual void MultiCreate(const MathLib::Dimension<unsigned>& dimension, unsigned count, const std::vector<byte_buffer>& buffs);

        virtual bool IsMultiTexture() override { return true; }

    protected:
        virtual error CreateFromSystemMemories(const MathLib::Dimension<unsigned>& dimension, unsigned count, const std::vector<byte_buffer>& buffs) = 0;

        virtual error LoadTextureImages(const std::vector<byte_buffer>& img_buffs) = 0;
        virtual future_error AsyncLoadTextureImages(const std::vector<byte_buffer>& img_buffs);

        virtual error SaveTextureImages(const std::vector<FileSystem::IFilePtr>& files) = 0;
        virtual future_error AsyncSaveTextureImages(const std::vector<FileSystem::IFilePtr>& files);

        virtual error LoadTextureImages(const std::vector<std::string>& filenames,
            const std::vector<std::string>& pathids);
        virtual future_error AsyncLoadTextureImages(const std::vector<std::string>& filenames,
            const std::vector<std::string>& pathids);

        virtual error SaveTextureImages(const std::vector<std::string>& filenames,
            const std::vector<std::string>& pathids);
        virtual future_error AsyncSaveTextureImages(const std::vector<std::string>& filenames,
            const std::vector<std::string>& pathids);

    protected:
        //! not supported funcs
        //virtual er_code CreateAsCubeTexture(unsigned char* img_buffs[6], unsigned int buff_sizes[6])
          //  override final {
            //return ER_NOT_IMPLEMENT;
        //};
        virtual error CreateFromSystemMemory(const MathLib::Dimension<unsigned>&, const byte_buffer&) override final
        {
            return ErrorCode::notImplement;
        };
        virtual error LoadTextureImage(const byte_buffer&) override final
        {
            return ErrorCode::notImplement;
        };
        virtual error LoadTextureImage(const std::string& filename, const std::string& pathid) override final
        {
            return ErrorCode::notImplement;
        };
        virtual error UpdateTextureImage(const MathLib::Rect&, const byte_buffer&) override final
        {
            return ErrorCode::notImplement;
        };
        virtual error RetrieveTextureImage(const MathLib::Rect&) override final
        {
            return ErrorCode::notImplement;
        }
        virtual error SaveTextureImage(const FileSystem::IFilePtr&) override final
        {
            return ErrorCode::notImplement;
        };
        virtual error SaveTextureImage(const std::string& filename, const std::string& pathid) override final
        {
            return ErrorCode::notImplement;
        };
    };
    using IMultiTexturePtr = std::shared_ptr<IMultiTexture>;
    using IMultiTextureWeak = std::weak_ptr<IMultiTexture>;
}


#endif // MULTI_TEXTURE_INTERFACE_H
