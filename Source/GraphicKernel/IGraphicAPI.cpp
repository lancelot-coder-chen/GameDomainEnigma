﻿#include "IGraphicAPI.h"
#include "GraphicThread.h"
#include "GraphicAssetRepository.h"
#include "IVertexShader.h"
#include "IVertexDeclaration.h"
#include "MathLib/ColorRGBA.h"
#include "Platforms/PlatformLayer.h"
#include <cassert>

using namespace Enigma::Graphics;

IGraphicAPI* IGraphicAPI::m_instance = nullptr;

IGraphicAPI::IGraphicAPI()
{
    assert(!m_instance);
    m_instance = this;
    m_workerThread = new GraphicThread{};
    m_repository = new AssetRepository{};
}

IGraphicAPI::~IGraphicAPI()
{
    m_instance = nullptr;
    if (m_workerThread) delete m_workerThread;
    if (m_repository) delete m_repository;
}

IGraphicAPI* IGraphicAPI::Instance()
{
    assert(m_instance);
    return m_instance;
}

future_error IGraphicAPI::AsyncCreateDevice(const DeviceRequiredBits& rqb, void* hwnd)
{
    return m_workerThread->PushTask(std::bind(&IGraphicAPI::CreateDevice, this, rqb, hwnd));
}

future_error IGraphicAPI::AsyncCleanupDevice()
{
    return m_workerThread->PushTask(std::bind(&IGraphicAPI::CleanupDevice, this));
}

future_error IGraphicAPI::AsyncFlip()
{
    return m_workerThread->PushTask([=]() -> error { return this->Flip(); });
}

future_error IGraphicAPI::AsyncCreatePrimaryBackSurface(const std::string& back_name, const std::string& depth_name)
{
    return m_workerThread->PushTask(std::bind(&IGraphicAPI::CreatePrimaryBackSurface, this,
        back_name, depth_name));
}

future_error IGraphicAPI::AsyncCreateBackSurface(const std::string& back_name, const MathLib::Dimension& dimension, 
    const GraphicFormat& fmt)
{
    return m_workerThread->PushTask([=]() -> error
        { return this->CreateBackSurface(back_name, dimension, fmt); });
}

future_error IGraphicAPI::AsyncCreateBackSurface(const std::string& back_name, const MathLib::Dimension& dimension, 
    unsigned buff_count, const std::vector<GraphicFormat>& fmts)
{
    return m_workerThread->PushTask([=]() -> error
        { return this->CreateBackSurface(back_name, dimension, buff_count, fmts); });
}

future_error IGraphicAPI::AsyncCreateDepthStencilSurface(const std::string& depth_name, const MathLib::Dimension& dimension,
    const GraphicFormat& fmt)
{
    return m_workerThread->PushTask([=]() -> error
        { return this->CreateDepthStencilSurface(depth_name, dimension, fmt); });
}

future_error IGraphicAPI::AsyncShareDepthStencilSurface(const std::string& depth_name, 
    const IDepthStencilSurfacePtr& from_depth)
{
    return m_workerThread->PushTask([=]() -> error
        { return this->ShareDepthStencilSurface(depth_name, from_depth); });
}

future_error IGraphicAPI::AsyncClearSurface(const IBackSurfacePtr& back_surface,
    const IDepthStencilSurfacePtr& depth_surface, const MathLib::ColorRGBA& color, float depth_value,
    unsigned stencil_value)
{
    return m_workerThread->PushTask([=]() -> error
        { return this->ClearSurface(back_surface, depth_surface, color, depth_value, stencil_value); });
}

future_error IGraphicAPI::AsyncBindBackSurface(const IBackSurfacePtr& back_surface,
    const IDepthStencilSurfacePtr& depth_surface)
{
    return m_workerThread->PushTask([=]() -> error
        { return this->BindBackSurface(back_surface, depth_surface); });
}

future_error IGraphicAPI::AsyncBindViewPort(const TargetViewPort& vp)
{
    return m_workerThread->PushTask([=]() -> error
        { return this->BindViewPort(vp); });
}

future_error IGraphicAPI::AsyncCreateVertexShader(const std::string& name)
{
    return m_workerThread->PushTask([=]() -> error { return this->CreateVertexShader(name); });
}

future_error IGraphicAPI::AsyncCreatePixelShader(const std::string& name)
{
    return m_workerThread->PushTask([=]() -> error { return this->CreatePixelShader(name); });
}

future_error IGraphicAPI::AsyncCreateShaderProgram(const std::string& name,
    const IVertexShaderPtr& vtx_shader, const IPixelShaderPtr& pix_shader)
{
    return m_workerThread->PushTask([=]() -> error { return this->CreateShaderProgram(name, vtx_shader, pix_shader); });
}

future_error IGraphicAPI::AsyncCreateVertexDeclaration(const std::string& name,
    const std::string& data_vertex_format, const IVertexShaderPtr& shader)
{
    return m_workerThread->PushTask([=]() -> error { return this->CreateVertexDeclaration(name, data_vertex_format, shader); });
}

std::string IGraphicAPI::QueryVertexDeclarationName(const std::string& data_vertex_format, const IVertexShaderPtr& shader)
{
    assert(shader);

    Platforms::Debug::Printf("query vertex declaration name in thread %d\n", std::this_thread::get_id());

    std::lock_guard locker{ m_declMapLock };
    auto iter = m_vertexDeclMap.find({ data_vertex_format, shader->GetName() });
    if (iter == m_vertexDeclMap.end()) return std::string();
    return iter->second;
}

future_error IGraphicAPI::AsyncCreateVertexBuffer(const std::string& buff_name)
{
    return m_workerThread->PushTask([=]() -> error { return this->CreateVertexBuffer(buff_name); });
}

future_error IGraphicAPI::AsyncCreateIndexBuffer(const std::string& buff_name)
{
    return m_workerThread->PushTask([=]() -> error { return this->CreateIndexBuffer(buff_name); });
}

future_error IGraphicAPI::AsyncCreateSamplerState(const std::string& name)
{
    return m_workerThread->PushTask([=]() -> error { return this->CreateSamplerState(name); });
}

future_error IGraphicAPI::AsyncCreateRasterizerState(const std::string& name)
{
    return m_workerThread->PushTask([=]() -> error { return this->CreateRasterizerState(name); });
}

future_error IGraphicAPI::AsyncCreateAlphaBlendState(const std::string& name)
{
    return m_workerThread->PushTask([=]() -> error { return this->CreateDepthStencilState(name); });
}

future_error IGraphicAPI::AsyncCreateDepthStencilState(const std::string& name)
{
    return m_workerThread->PushTask([=]() -> error { return this->CreateDepthStencilState(name); });
}

future_error IGraphicAPI::AsyncCreateTexture(const std::string& tex_name)
{
    return m_workerThread->PushTask([=]() -> error { return this->CreateTexture(tex_name); });
}

future_error IGraphicAPI::AsyncCreateMultiTexture(const std::string& tex_name)
{
    return m_workerThread->PushTask([=]() -> error { return this->CreateMultiTexture(tex_name); });
}

void IGraphicAPI::TerminateGraphicThread()
{
    if (m_workerThread)
    {
        m_workerThread->Terminate();
    }
}

GraphicThread* IGraphicAPI::GetGraphicThread()
{
    assert(m_workerThread);
    return m_workerThread;
}

