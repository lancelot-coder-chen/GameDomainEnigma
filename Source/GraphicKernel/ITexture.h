﻿/*********************************************************************
 * \file   ITexture.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   June 2022
 *********************************************************************/
#ifndef TEXTURE_INTERFACE_H
#define TEXTURE_INTERFACE_H

#include "GraphicAPITypes.h"
#include "MathLib/AlgebraBasicTypes.h"
#include "Frameworks/ExtentTypesDefine.h"
#include "RenderTextureUsage.h"
#include <memory>
#include <system_error>

namespace Enigma::MathLib
{
    class Rect;
}
namespace Enigma::FileSystem
{
    class IFile;
    using IFilePtr = std::shared_ptr<IFile>;
}

namespace Enigma::Graphics
{
    class IBackSurface;
    using IBackSurfacePtr = std::shared_ptr<IBackSurface>;

    using error = std::error_code;

    class ITexture : public std::enable_shared_from_this<ITexture>
    {
    public:
        ITexture(const std::string& name);
        ITexture(const ITexture&) = delete;
        ITexture(ITexture&&) = delete;
        virtual ~ITexture();
        ITexture& operator=(const ITexture&) = delete;
        ITexture& operator=(ITexture&&) = delete;

        const std::string& getName() { return m_name; }

        virtual void load(const byte_buffer& img_buff);
        virtual void load(const std::string& filename, const std::string& pathid);
        virtual void save(const FileSystem::IFilePtr& file);
        virtual void save(const std::string& filename, const std::string& pathid);

        virtual void create(const MathLib::Dimension<unsigned>& dimension, const byte_buffer& buff);

        virtual void retrieve(const MathLib::Rect& rcSrc);
        virtual void update(const MathLib::Rect& rcDest, const byte_buffer& img_buff);

        virtual void asBackSurface(const IBackSurfacePtr& back_surf, const std::vector<RenderTextureUsage>&);

        virtual const GraphicFormat& format() { return m_format; };
        virtual const MathLib::Dimension<unsigned>& dimension() { return m_dimension; };
        const byte_buffer& getRetrievedBuffer() { return m_retrievedBuff; }
        virtual bool isCubeTexture() { return m_isCubeTexture; }

        virtual bool isMultiTexture() { return false; }

    protected:
        virtual error loadTextureImage(const byte_buffer& img_buff) = 0;
        virtual error loadTextureImage(const std::string& filename, const std::string& pathid);
        virtual error createFromSystemMemory(const MathLib::Dimension<unsigned>& dimension, const byte_buffer& buff) = 0;
        virtual error saveTextureImage(const FileSystem::IFilePtr& file) = 0;
        virtual error retrieveTextureImage(const MathLib::Rect& rcSrc) = 0;
        virtual error updateTextureImage(const MathLib::Rect& rcDest, const byte_buffer& img_buff) = 0;
        virtual error saveTextureImage(const std::string& filename, const std::string& pathid);
        virtual error useAsBackSurface(const IBackSurfacePtr& back_surf, const std::vector<RenderTextureUsage>& usages) = 0;

    protected:
        std::string m_name;
        bool m_isCubeTexture;
        MathLib::Dimension<unsigned> m_dimension;

        GraphicFormat m_format;

        byte_buffer m_retrievedBuff;
    };
    using ITexturePtr = std::shared_ptr<ITexture>;
    using ITextureWeak = std::weak_ptr<ITexture>;
}

#endif // TEXTURE_INTERFACE_H
