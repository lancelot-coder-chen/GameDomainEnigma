﻿#include "DrawPrimitiveTestApp.h"
#include "MathLib/Vector3.h"
#include "Controllers/InstallingPolicies.h"
#include <cassert>
#include <GraphicKernel/GraphicEvents.h>
#include <Platforms/MemoryAllocMacro.h>
#include "Frameworks/EventPublisher.h"
#include "Frameworks/ServiceManager.h"
#include "GameEngine/RendererEvents.h"
#include "Frameworks/CommandBus.h"
#include "ShaderBuilder.h"
#include "BufferBuilder.h"
#include "GameEngine/RendererCommands.h"

using namespace Enigma::Controllers;
using namespace Enigma::Graphics;
using namespace Enigma::Application;
using namespace Enigma::Frameworks;
using namespace Enigma::Engine;
using namespace Enigma::MathLib;

std::string PrimaryTargetName = "primary_target";
std::string DefaultRendererName = "default_renderer";
std::string ShaderProgramName = "shader_program";
std::string VertexDeclName = "vtx_layout";
std::string VertexBufferName = "vtx_buffer";
std::string IndexBufferName = "idx_buffer";

struct VtxData
{
    Vector3 pos;
    unsigned int idx;
};
static VtxData vtx_pos[] =
{
    { Vector3(-0.25f, 0.25f, 0.5f), 0 },
    { Vector3(0.25f, 0.25f, 0.5f), 1 },
    { Vector3(-0.25f, -0.25f, 0.5f), 2 },
    { Vector3(0.25f, -0.25f, 0.5f), 3 },
};
static unsigned int vtx_idx[] =
{
    0, 1, 2, 1, 3, 2,
};

static std::string vs_code_11 = " \
float4 offset[4]; \
struct VS_INPUT \
{\
    float3 Pos : POSITION;\
    uint Index : BLENDINDICES; \
};\
struct VS_OUTPUT\
{\
    float4 Pos : SV_POSITION;\
    float4 DiffCol : COLOR0;\
};\
VS_OUTPUT vs_main(const VS_INPUT v)\
{\
    VS_OUTPUT o;\
    o.Pos.xyz = v.Pos + offset[v.Index];\
    o.Pos.w = 1.0f; \
    o.DiffCol = float4(0.5f + v.Pos.x + v.Pos.y, 0.5f + v.Pos.x - v.Pos.y, 0.5f + v.Pos.y - v.Pos.x, 1.0f);\
    return o;\
}\
";

static std::string ps_code_11 = ""
"//semantic anim_time ANIM_TIMER\n\r"
"//semantic anim_scale ANIM_SCALE\n\r"
"float anim_time : ANIM_TIMER; \
float anim_scale : ANIM_SCALE; \
struct PS_INPUT \
{ \
    float4 Pos : SV_POSITION; \
    float4 DiffCol : COLOR0; \
}; \
struct PS_OUTPUT \
{ \
    float4 Color : SV_TARGET; \
}; \
PS_OUTPUT ps_main(const PS_INPUT p) \
{ \
    PS_OUTPUT o; \
    o.Color = p.DiffCol; \
    return o; \
} \
";

DrawPrimitiveTestApp::DrawPrimitiveTestApp(const std::string app_name) : AppDelegate(app_name)
{
}

DrawPrimitiveTestApp::~DrawPrimitiveTestApp()
{
}

void DrawPrimitiveTestApp::InstallEngine()
{
    m_onRenderTargetCreated = std::make_shared<EventSubscriber>([=](auto e) { this->OnRenderTargetCreated(e); });
    EventPublisher::Subscribe(typeid(PrimaryRenderTargetCreated), m_onRenderTargetCreated);
    m_onShaderProgramCreated = std::make_shared<EventSubscriber>([=](auto e) { this->OnShaderProgramCreated(e); });
    EventPublisher::Subscribe(typeid(DeviceShaderProgramCreated), m_onShaderProgramCreated);
    m_onVertexBufferBuilt = std::make_shared<EventSubscriber>([=](auto e) { this->OnVertexBufferBuilt(e); });
    EventPublisher::Subscribe(typeid(VertexBufferBuilt), m_onVertexBufferBuilt);
    m_onIndexBufferBuilt = std::make_shared<EventSubscriber>([=](auto e) { this->OnIndexBufferBuilt(e); });
    EventPublisher::Subscribe(typeid(IndexBufferBuilt), m_onIndexBufferBuilt);

    assert(m_graphicMain);

    auto creating_policy = std::make_unique<DeviceCreatingPolicy>(IGraphicAPI::Instance(), DeviceRequiredBits(), m_asyncType, m_hwnd);
    auto policy = std::make_unique<InstallingDefaultRendererPolicy>(std::move(creating_policy), DefaultRendererName, PrimaryTargetName);
    m_graphicMain->InstallRenderEngine(std::move(policy));
    m_rendererManager = ServiceManager::GetSystemServiceAs<RendererManager>();

    m_shaderBuilder = menew ShaderBuilder(m_asyncType);
    m_shaderBuilder->BuildShaderProgram(ShaderBuilder::ShaderProgramBuildParameter{ ShaderProgramName, "vtx_shader", "xyzb1_betabyte", vs_code_11, VertexDeclName, "pix_shader", ps_code_11 });

    byte_buffer points = make_data_buffer((unsigned char*)vtx_pos, sizeof(vtx_pos));
    uint_buffer indices = make_data_buffer(vtx_idx, 6);
    m_bufferBuilder = menew BufferBuilder(m_asyncType);
    m_bufferBuilder->BuildVertexBuffer(VertexBufferName, sizeof(VtxData), points);
    m_bufferBuilder->BuildIndexBuffer(IndexBufferName, indices);
}

void DrawPrimitiveTestApp::ShutdownEngine()
{
    delete m_shaderBuilder;
    m_shaderBuilder = nullptr;
    delete m_bufferBuilder;
    m_bufferBuilder = nullptr;

    EventPublisher::Unsubscribe(typeid(PrimaryRenderTargetCreated), m_onRenderTargetCreated);
    m_onRenderTargetCreated = nullptr;
    EventPublisher::Unsubscribe(typeid(DeviceShaderProgramCreated), m_onShaderProgramCreated);
    m_onShaderProgramCreated = nullptr;
    EventPublisher::Unsubscribe(typeid(VertexBufferBuilt), m_onVertexBufferBuilt);
    m_onVertexBufferBuilt = nullptr;
    EventPublisher::Unsubscribe(typeid(IndexBufferBuilt), m_onIndexBufferBuilt);
    m_onIndexBufferBuilt = nullptr;

    m_renderTarget = nullptr;
    m_program = nullptr;
    m_vtxDecl = nullptr;
    m_vtxBuffer = nullptr;
    m_idxBuffer = nullptr;

    m_graphicMain->ShutdownRenderEngine();
}

void DrawPrimitiveTestApp::FrameUpdate()
{
    AppDelegate::FrameUpdate();
}

void DrawPrimitiveTestApp::RenderFrame()
{
    if ((!m_vtxDecl) || (!m_program) || (!m_vtxBuffer) || (!m_idxBuffer) || (!m_renderTarget)) return;
    m_renderTarget->AsyncBind();
    m_renderTarget->AsyncBindViewPort();
    IGraphicAPI::Instance()->AsyncBindVertexDeclaration(m_vtxDecl);
    IGraphicAPI::Instance()->AsyncBindShaderProgram(m_program);
    IGraphicAPI::Instance()->AsyncBindVertexBuffer(m_vtxBuffer, PrimitiveTopology::Topology_TriangleList);
    IGraphicAPI::Instance()->AsyncBindIndexBuffer(m_idxBuffer);
    m_renderTarget->AsyncClear();
    IGraphicAPI::Instance()->AsyncBeginScene();
    IGraphicAPI::Instance()->AsyncDrawIndexedPrimitive(6, 4, 0, 0);
    IGraphicAPI::Instance()->AsyncEndScene();
    m_renderTarget->AsyncFlip();
}

void DrawPrimitiveTestApp::OnRenderTargetCreated(const IEventPtr& e)
{
    if (!e) return;
    std::shared_ptr<PrimaryRenderTargetCreated> ev = std::dynamic_pointer_cast<PrimaryRenderTargetCreated, IEvent>(e);
    if (!ev) return;
    m_renderTarget = m_rendererManager->GetRenderTarget(ev->GetRenderTargetName());
}

void DrawPrimitiveTestApp::OnShaderProgramCreated(const IEventPtr& e)
{
    if (!e) return;
    std::shared_ptr<DeviceShaderProgramCreated> ev = std::dynamic_pointer_cast<DeviceShaderProgramCreated, IEvent>(e);
    if (!ev) return;
    if (ev->GetName() != ShaderProgramName) return;
    m_program = IGraphicAPI::Instance()->GetGraphicAsset<IShaderProgramPtr>(ev->GetName());
    m_vtxDecl = IGraphicAPI::Instance()->GetGraphicAsset<IVertexDeclarationPtr>(VertexDeclName);
}

void DrawPrimitiveTestApp::OnVertexBufferBuilt(const IEventPtr& e)
{
    if (!e) return;
    std::shared_ptr<VertexBufferBuilt> ev = std::dynamic_pointer_cast<VertexBufferBuilt, IEvent>(e);
    if (!ev) return;
    if (ev->GetBufferName() != VertexBufferName) return;
    m_vtxBuffer = IGraphicAPI::Instance()->GetGraphicAsset<IVertexBufferPtr>(ev->GetBufferName());
}

void DrawPrimitiveTestApp::OnIndexBufferBuilt(const IEventPtr& e)
{
    if (!e) return;
    std::shared_ptr<IndexBufferBuilt> ev = std::dynamic_pointer_cast<IndexBufferBuilt, IEvent>(e);
    if (!ev) return;
    if (ev->GetBufferName() != IndexBufferName) return;
    m_idxBuffer = IGraphicAPI::Instance()->GetGraphicAsset<IIndexBufferPtr>(ev->GetBufferName());
}