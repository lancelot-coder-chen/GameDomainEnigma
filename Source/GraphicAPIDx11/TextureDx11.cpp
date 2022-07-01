﻿#include "TextureDx11.h"
#include "BackSurfaceDx11.h"
#include "GraphicAPIDx11.h"
#include "Platforms/MemoryMacro.h"
#include "Platforms/PlatformLayer.h"
#include "GraphicKernel/GraphicErrors.h"
#include "GraphicKernel/GraphicEvents.h"
#include "Frameworks/EventPublisher.h"
#include "MathLib/Rect.h"
#include "FileSystem/IFile.h"

using namespace Enigma::Devices;
using ErrorCode = Enigma::Graphics::ErrorCode;

extern DXGI_FORMAT ConvertGraphicFormatToDXGI(const Enigma::Graphics::GraphicFormat& format);
extern unsigned int ConvertDXGIFormatToGraphicFormat(DXGI_FORMAT fmt);

TextureDx11::TextureDx11(const std::string& name) :ITexture(name)
{
    m_d3dTextureResource = nullptr;
}

TextureDx11::~TextureDx11()
{
    SAFE_RELEASE(m_d3dTextureResource);
}

error TextureDx11::CreateFromSystemMemory(const MathLib::Dimension& dimension, const byte_buffer& buff)
{
    GraphicAPIDx11* api_dx11 = dynamic_cast<GraphicAPIDx11*>(Graphics::IGraphicAPI::Instance());
    assert(api_dx11);
    ID3D11Device* device = api_dx11->GetD3DDevice();
    if (FATAL_LOG_EXPR(!device)) return ErrorCode::d3dDeviceNullPointer;

    D3D11_TEXTURE2D_DESC tex_desc;
    ZeroMemory(&tex_desc, sizeof(tex_desc));
    tex_desc.Width = dimension.m_width;
    tex_desc.Height = dimension.m_height;
    tex_desc.ArraySize = 1;
    tex_desc.SampleDesc.Count = 1;
    tex_desc.SampleDesc.Quality = 0;
    tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    tex_desc.Usage = D3D11_USAGE_DEFAULT;
    tex_desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
    tex_desc.CPUAccessFlags = 0;
    tex_desc.MiscFlags = 0;
    tex_desc.MipLevels = 1;
    if (m_isCubeTexture)
    {
        tex_desc.ArraySize = 6;
        tex_desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    }

    ID3D11Texture2D* texture2D = 0;

    if (!buff.empty())
    {
        D3D11_SUBRESOURCE_DATA data;
        ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
        data.pSysMem = &buff[0];
        data.SysMemPitch = 4 * tex_desc.Width;

        HRESULT hr = device->CreateTexture2D(&tex_desc, &data, &texture2D);
        if (FAILED(hr)) return ErrorCode::deviceCreateTexture;
    }
    else
    {
        HRESULT hr = device->CreateTexture2D(&tex_desc, 0, &texture2D);
        if (FAILED(hr)) return ErrorCode::deviceCreateTexture;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC SRDesc;
    ID3D11ShaderResourceView* d3dResource = 0;

    SRDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SRDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    SRDesc.Texture2D.MostDetailedMip = 0;
    SRDesc.Texture2D.MipLevels = 1;
    HRESULT hr = device->CreateShaderResourceView(texture2D, &SRDesc, &d3dResource);
    if (FATAL_LOG_EXPR(S_OK != hr)) return ErrorCode::deviceCreateTexture;
    m_d3dTextureResource = d3dResource;
    texture2D->Release();

    m_dimension = dimension;
    m_format = ConvertDXGIFormatToGraphicFormat(SRDesc.Format);

    Frameworks::EventPublisher::Post(Frameworks::IEventPtr{ menew Graphics::TextureResourceFromMemoryCreated(m_name) });
    return ErrorCode::ok;
}

error TextureDx11::LoadTextureImage(const byte_buffer& img_buff)
{
    GraphicAPIDx11* api_dx11 = dynamic_cast<GraphicAPIDx11*>(Graphics::IGraphicAPI::Instance());
    assert(api_dx11);
    ID3D11Device* device = api_dx11->GetD3DDevice();
    if (FATAL_LOG_EXPR(!device)) return ErrorCode::d3dDeviceNullPointer;

    SAFE_RELEASE(m_d3dTextureResource);

    DirectX::TexMetadata metaData;
    DirectX::ScratchImage scratchImage;
    if ((img_buff[0] == 'D') && (img_buff[1] == 'D') && (img_buff[2] == 'S') && (img_buff[3] == ' '))
    {
        HRESULT hr = DirectX::LoadFromDDSMemory(&img_buff[0], img_buff.size(), 0, &metaData, scratchImage);
        if (FAILED(hr))
        {
            return ErrorCode::dxLoadTexture;
        }
    }
    else
    {
        HRESULT hr = DirectX::LoadFromWICMemory(&img_buff[0], img_buff.size(), DirectX::WIC_FLAGS_FORCE_RGB, &metaData, scratchImage);
        if (FAILED(hr))
        {
            return ErrorCode::dxLoadTexture;
        }
    }
    m_isCubeTexture = metaData.arraySize == 6;
    error er = CreateFromScratchImage(scratchImage);
    if (er) return er;

    Frameworks::EventPublisher::Post(Frameworks::IEventPtr{ menew Graphics::TextureResourceImageLoaded(m_name) });
    return ErrorCode::ok;
}

error TextureDx11::RetrieveTextureImage(const MathLib::Rect& rcSrc)
{
    m_retrievedBuff.clear();

    if (FATAL_LOG_EXPR(!m_d3dTextureResource)) return ErrorCode::nullDxTexture;

    GraphicAPIDx11* api_dx11 = dynamic_cast<GraphicAPIDx11*>(Graphics::IGraphicAPI::Instance());
    assert(api_dx11);
    ID3D11Device* device = api_dx11->GetD3DDevice();
    if (FATAL_LOG_EXPR(!device)) return ErrorCode::d3dDeviceNullPointer;
    ID3D11DeviceContext* deviceContext = api_dx11->GetD3DDeviceContext();
    if (FATAL_LOG_EXPR(!deviceContext)) return ErrorCode::d3dDeviceNullPointer;

    D3D11_TEXTURE2D_DESC tex_desc;
    ZeroMemory(&tex_desc, sizeof(tex_desc));
    tex_desc.Width = rcSrc.Width();
    tex_desc.Height = rcSrc.Height();
    tex_desc.ArraySize = 1;
    tex_desc.SampleDesc.Count = 1;
    tex_desc.SampleDesc.Quality = 0;
    tex_desc.Format = ConvertGraphicFormatToDXGI(m_format);
    tex_desc.Usage = D3D11_USAGE_STAGING;
    tex_desc.BindFlags = 0; //D3D10_BIND_SHADER_RESOURCE;
    tex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    tex_desc.MiscFlags = 0;
    tex_desc.MipLevels = 1;

    ID3D11Texture2D* targetTex = 0;
    HRESULT hr = device->CreateTexture2D(&tex_desc, 0, &targetTex);
    if (FAILED(hr)) return ErrorCode::dxCreateTexture;
    if (FATAL_LOG_EXPR(!targetTex)) return ErrorCode::nullDxTexture;

    ID3D11Resource* d3dSrcResource;
    m_d3dTextureResource->GetResource(&d3dSrcResource);

    D3D10_BOX boxSrc;
    boxSrc.left = rcSrc.Left();
    boxSrc.top = rcSrc.Top();
    boxSrc.right = rcSrc.Right();
    boxSrc.bottom = rcSrc.Bottom();
    boxSrc.front = 0;
    boxSrc.back = 1;
    deviceContext->CopySubresourceRegion(targetTex, 0, 0, 0, 0, d3dSrcResource, 0, (const D3D11_BOX*)&boxSrc);
    d3dSrcResource->Release();

    D3D11_MAPPED_SUBRESOURCE mapped_tex_data;
    hr = deviceContext->Map(targetTex, 0, D3D11_MAP_READ, 0, &mapped_tex_data);
    if (FAILED(hr))
    {
        targetTex->Release();
        return ErrorCode::dxTextureMapping;
    }
    m_retrievedBuff.resize(rcSrc.Height() * rcSrc.Width() * 4);
    if (mapped_tex_data.RowPitch == (unsigned int)rcSrc.Width() * 4)
    {
        size_t buff_size = m_retrievedBuff.size();
        memcpy(&m_retrievedBuff[0], mapped_tex_data.pData, buff_size);
    }
    else
    {
        int height = rcSrc.Height();
        int width = rcSrc.Width();
        int dest_pitch = width << 2;
        void* src_buf = mapped_tex_data.pData;
        void* dest_buf = (void*)(&m_retrievedBuff[0]);
        for (int i = 0; i < height; i++)
        {
            memcpy(dest_buf, src_buf, dest_pitch);
            src_buf = (void*)((size_t)src_buf + mapped_tex_data.RowPitch);
            dest_buf = (void*)((size_t)dest_buf + dest_pitch);
        }
    }
    deviceContext->Unmap(targetTex, 0);
    targetTex->Release();

    Frameworks::EventPublisher::Post(Frameworks::IEventPtr{ menew Graphics::TextureResourceImageRetrieved(m_name, rcSrc) });
    return ErrorCode::ok;
}

error TextureDx11::UpdateTextureImage(const MathLib::Rect& rcDest, const byte_buffer& img_buff)
{
    if (FATAL_LOG_EXPR(img_buff.empty())) return ErrorCode::nullMemoryBuffer;
    if (FATAL_LOG_EXPR((rcDest.Width() <= 0) || (rcDest.Height() <= 0))) return ErrorCode::invalidParameter;
    GraphicAPIDx11* api_dx11 = dynamic_cast<GraphicAPIDx11*>(Graphics::IGraphicAPI::Instance());
    assert(api_dx11);
    ID3D11Device* device = api_dx11->GetD3DDevice();
    if (FATAL_LOG_EXPR(!device)) return ErrorCode::d3dDeviceNullPointer;
    ID3D11DeviceContext* deviceContext = api_dx11->GetD3DDeviceContext();
    if (FATAL_LOG_EXPR(!deviceContext)) return ErrorCode::d3dDeviceNullPointer;
    if (FATAL_LOG_EXPR(!m_d3dTextureResource)) return ErrorCode::nullDxTexture;

    D3D11_TEXTURE2D_DESC tex_desc;
    ZeroMemory(&tex_desc, sizeof(tex_desc));
    tex_desc.Width = rcDest.Width();
    tex_desc.Height = rcDest.Height();
    tex_desc.ArraySize = 1;
    tex_desc.SampleDesc.Count = 1;
    tex_desc.SampleDesc.Quality = 0;
    tex_desc.Format = ConvertGraphicFormatToDXGI(m_format);
    tex_desc.Usage = D3D11_USAGE_DYNAMIC;
    tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    tex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    tex_desc.MiscFlags = 0;
    tex_desc.MipLevels = 1;

    ID3D11Texture2D* mappingTex = 0;
    HRESULT hr = device->CreateTexture2D(&tex_desc, 0, &mappingTex);
    if (FAILED(hr)) return ErrorCode::dxCreateTexture;
    if (FATAL_LOG_EXPR(!mappingTex)) return ErrorCode::nullDxTexture;

    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = deviceContext->Map(mappingTex, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr))
    {
        mappingTex->Release();
        return ErrorCode::dxTextureMapping;
    }
    if (mapped.RowPitch == (unsigned int)rcDest.Width() * 4)
    {
        memcpy(mapped.pData, &img_buff[0], img_buff.size());
    }
    else
    {
        int height = rcDest.Height();
        int width = rcDest.Width();
        int src_pitch = width << 2;
        void* dest_buf = mapped.pData;
        void* src_buf = (void*)&img_buff[0];
        for (int i = 0; i < height; i++)
        {
            memcpy(dest_buf, src_buf, src_pitch);
            dest_buf = (void*)((size_t)dest_buf + mapped.RowPitch);
            src_buf = (void*)((size_t)src_buf + src_pitch);
        }
    }
    deviceContext->Unmap(mappingTex, 0);

    ID3D11Resource* d3dResource;
    m_d3dTextureResource->GetResource(&d3dResource);
    D3D11_BOX boxSrc;
    boxSrc.left = 0;
    boxSrc.top = 0;
    boxSrc.right = rcDest.Width();
    boxSrc.bottom = rcDest.Height();
    boxSrc.front = 0;
    boxSrc.back = 1;
    deviceContext->CopySubresourceRegion(d3dResource, 0, rcDest.Left(), rcDest.Top(), 0, mappingTex, 0, &boxSrc);

    mappingTex->Release();
    d3dResource->Release();

    Frameworks::EventPublisher::Post(Frameworks::IEventPtr{ menew Graphics::TextureResourceImageUpdated(m_name, rcDest) });
    return ErrorCode::ok;
}

error TextureDx11::SaveTextureImage(const FileSystem::IFilePtr& file)
{
    assert(file);
    GraphicAPIDx11* api_dx11 = dynamic_cast<GraphicAPIDx11*>(Graphics::IGraphicAPI::Instance());
    assert(api_dx11);
    ID3D11Device* device = api_dx11->GetD3DDevice();
    if (FATAL_LOG_EXPR(!device)) return ErrorCode::d3dDeviceNullPointer;
    ID3D11DeviceContext* deviceContext = api_dx11->GetD3DDeviceContext();
    if (FATAL_LOG_EXPR(!deviceContext)) return ErrorCode::d3dDeviceNullPointer;

    if (!m_d3dTextureResource) return ErrorCode::nullDxTexture;
    ID3D11Resource* d3dResource;
    m_d3dTextureResource->GetResource(&d3dResource);
    if (!d3dResource) return ErrorCode::nullDxTexture;

    DirectX::ScratchImage resultImage;
    HRESULT hr = DirectX::CaptureTexture(device, deviceContext, d3dResource, resultImage);
    if (FATAL_LOG_EXPR(FAILED(hr))) return ErrorCode::dxSaveTexture;

    DirectX::Blob blob;
    DirectX::SaveToDDSMemory(resultImage.GetImages(), resultImage.GetImageCount(), resultImage.GetMetadata(), 0, blob);
    byte_buffer write_buff = make_data_buffer((unsigned char*)blob.GetBufferPointer(), blob.GetBufferSize());
    size_t write_bytes = file->Write(0, write_buff);
    if (write_bytes != write_buff.size()) return ErrorCode::saveTextureFile;

    Frameworks::EventPublisher::Post(Frameworks::IEventPtr{ menew Graphics::TextureResourceImageSaved(m_name, file->GetFullPath()) });
    return ErrorCode::ok;
}

error TextureDx11::UseAsBackSurface(const std::shared_ptr<Graphics::IBackSurface>& back_surf)
{
    GraphicAPIDx11* api_dx11 = dynamic_cast<GraphicAPIDx11*>(Graphics::IGraphicAPI::Instance());
    assert(api_dx11);
    ID3D11Device* device = api_dx11->GetD3DDevice();
    if (FATAL_LOG_EXPR(!device)) return ErrorCode::d3dDeviceNullPointer;

    if (FATAL_LOG_EXPR(!back_surf)) return ErrorCode::nullBackSurface;
    BackSurfaceDx11* bbDx11 = dynamic_cast<BackSurfaceDx11*>(back_surf.get());
    assert(bbDx11);

    SAFE_RELEASE(m_d3dTextureResource);

    m_dimension = bbDx11->GetDimension();
    m_format = back_surf->GetFormat();

    // backbuffer surface不能直接拿來用，要建立一個shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC SRDesc;
    ID3D11ShaderResourceView* d3dResource = NULL;

    SRDesc.Format = ConvertGraphicFormatToDXGI(back_surf->GetFormat());
    SRDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
    SRDesc.Texture2D.MostDetailedMip = 0;
    SRDesc.Texture2D.MipLevels = 1;
    HRESULT hr = device->CreateShaderResourceView(bbDx11->GetD3DSurface(), &SRDesc, &d3dResource);
    if (FATAL_LOG_EXPR(S_OK != hr)) return ErrorCode::dxCreateShaderResource;
    m_d3dTextureResource = d3dResource;

    Frameworks::EventPublisher::Post(Frameworks::IEventPtr{ menew Graphics::TextureResourceAsBackSurfaceUsed(m_name, back_surf->GetName()) });
    return ErrorCode::ok;
}

error TextureDx11::CreateFromScratchImage(DirectX::ScratchImage& scratchImage)
{
    if (!m_isCubeTexture)
    {
        byte_buffer img_buff;
        img_buff.resize(scratchImage.GetPixelsSize());
        memcpy(&img_buff[0], scratchImage.GetPixels(), scratchImage.GetPixelsSize());
        return CreateFromSystemMemory(
            MathLib::Dimension{ (unsigned int)scratchImage.GetMetadata().width, (unsigned int)scratchImage.GetMetadata().height },
            img_buff);
    }
    else
    {
        //CreateCubeTextureFromScratchImage(scratchImage);
        return ErrorCode::notImplement;
    }
}