﻿/*********************************************************************
 * \file   TextureDx11.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   June 2022
 *********************************************************************/
#ifndef TEXTURE_DX11_H
#define TEXTURE_DX11_H

#include "GraphicKernel/ITexture.h"
#include <D3D11.h>
#include "DirectXTex.h"

namespace Enigma::Devices
{
    using error = std::error_code;

    class TextureDx11 : public Graphics::ITexture
    {
    public:
        TextureDx11(const std::string& name);
        TextureDx11(const TextureDx11&) = delete;
        TextureDx11(TextureDx11&&) = delete;
        virtual ~TextureDx11() override;
        TextureDx11& operator=(const TextureDx11&) = delete;
        TextureDx11& operator=(TextureDx11&&) = delete;

        ID3D11ShaderResourceView* getD3DResourceView() const { return m_d3dTextureResource; }

    protected:
        virtual error createFromSystemMemory(const MathLib::Dimension<unsigned>& dimension, const byte_buffer& buff) override;
        virtual error loadTextureImage(const byte_buffer& img_buff) override;
        virtual error RetrieveTextureImage(const MathLib::Rect& rcSrc) override;
        virtual error UpdateTextureImage(const MathLib::Rect& rcDest, const byte_buffer& img_buff) override;
        virtual error SaveTextureImage(const FileSystem::IFilePtr& file) override;
        virtual error UseAsBackSurface(const std::shared_ptr<Graphics::IBackSurface>& back_surf, const std::vector<Graphics::RenderTextureUsage>& usages) override;

        error createFromScratchImage(DirectX::ScratchImage& scratchImage);
        //void CreateCubeTextureFromScratchImage(DirectX::ScratchImage& scratchImage);

    protected:
        ID3D11ShaderResourceView* m_d3dTextureResource;
    };
}


#endif // TEXTURE_DX11_H
