﻿#include "RenderBufferManagerTest.h"

#include <Frameworks/CommandBus.h>
#include <Platforms/MemoryAllocMacro.h>
#include "TextureSamplerBuilder.h"
#include "Controllers/InstallingPolicies.h"
#include "Gateways/EffectPolicyJsonGateway.h"
#include "Frameworks/ServiceManager.h"
#include "GameEngine/RendererManager.h"
#include "Frameworks/EventSubscriber.h"
#include "Frameworks/EventPublisher.h"
#include "GameEngine/RendererEvents.h"
#include "GameEngine/RenderTarget.h"
#include "GameEngine/RendererCommands.h"
#include "GameEngine/EffectEvents.h"
#include "GameEngine/RenderBufferEvents.h"
#include "GameEngine/RenderBufferCommands.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/IFile.h"
#include "MathLib/Vector2.h"
#include "FileSystem/StdMountPath.h"
#include "FileSystem/AndroidMountPath.h"

using namespace Enigma::Application;
using namespace Enigma::Controllers;
using namespace Enigma::Gateways;
using namespace Enigma::FileSystem;
using namespace Enigma::Frameworks;
using namespace Enigma::Engine;
using namespace Enigma::MathLib;
using namespace Enigma::Graphics;

std::string PrimaryTargetName = "primary_target";
std::string DefaultRendererName = "default_renderer";
std::string MaterialName = "test_effect";
std::string VertexBufferName = "vtx_buffer";
std::string IndexBufferName = "idx_buffer";
std::string RenderBufferName = "render_buffer";
std::string BufferSignatureName = "signature";
std::string TextureName = "earth";
std::string PassTopLeftName = "pass_top_left";
std::string PassTopRightName = "pass_top_right";
std::string PassBottomLeftName = "pass_bottom_left";
std::string PassBottomRightName = "pass_bottom_right";

struct VtxData
{
	Vector3 pos;
	Vector2 coord;
};
static VtxData vtx_pos[] =
{
	{ Vector3(-0.25f, 0.25f, 0.5f), Vector2(0.0f, 0.0f)},
	{ Vector3(0.25f, 0.25f, 0.5f), Vector2(1.0f, 0.0f) },
	{ Vector3(-0.25f, -0.25f, 0.5f), Vector2(0.0f, 1.0f) },
	{ Vector3(0.25f, -0.25f, 0.5f), Vector2(1.0f, 1.0f) },
};
static unsigned int vtx_idx[] =
{
	0, 1, 2, 1, 3, 2,
};

RenderBufferManagerTest::RenderBufferManagerTest(const std::string app_name) : AppDelegate(app_name)
{
	m_material = nullptr;
}

RenderBufferManagerTest::~RenderBufferManagerTest()
{
	
}

void RenderBufferManagerTest::InitializeMountPaths()
{
#if TARGET_PLATFORM == PLATFORM_WIN32
	if (FileSystem::Instance())
	{
		auto path = std::filesystem::current_path();
		auto mediaPath = path / "../../../../Media/";
		FileSystem::Instance()->AddMountPath(std::make_shared<StdMountPath>(mediaPath.string(), MediaPathName));
	}
#elif TARGET_PLATFORM == PLATFORM_ANDROID
	if (FileSystem::Instance())
	{
		FileSystem::Instance()->AddMountPath(std::make_shared<AndroidMountPath>("", MediaPathName));
	}
#endif
}

void RenderBufferManagerTest::InstallEngine()
{
	m_onRenderTargetCreated = std::make_shared<EventSubscriber>([=](auto e) { this->OnRenderTargetCreated(e); });
	EventPublisher::Subscribe(typeid(PrimaryRenderTargetCreated), m_onRenderTargetCreated);

	m_onEffectMaterialCompiled = std::make_shared<EventSubscriber>([=](auto e) { this->OnEffectMaterialCompiled(e); });
	EventPublisher::Subscribe(typeid(EffectMaterialCompiled), m_onEffectMaterialCompiled);
	m_onEffectCompileFailed = std::make_shared<EventSubscriber>([=](auto e) { this->OnEffectCompileFailed(e); });
	EventPublisher::Subscribe(typeid(EffectMaterialCompileFailed), m_onEffectCompileFailed);

	m_onRenderBufferBuilt = std::make_shared<EventSubscriber>([=](auto e) { this->OnRenderBufferBuilt(e); });
	EventPublisher::Subscribe(typeid(RenderBufferBuilt), m_onRenderBufferBuilt);
	m_onRenderBufferBuildFailed = std::make_shared<EventSubscriber>([=](auto e) { this->OnRenderBufferBuildFailed(e); });
	EventPublisher::Subscribe(typeid(RenderBufferBuildFailed), m_onRenderBufferBuildFailed);

	m_onTextureLoaded = std::make_shared<EventSubscriber>([=](auto e) { this->OnTextureLoaded(e); });
	EventPublisher::Subscribe(typeid(TextureLoaded), m_onTextureLoaded);

	assert(m_graphicMain);

	auto creating_policy = std::make_unique<DeviceCreatingPolicy>(Enigma::Graphics::DeviceRequiredBits(), m_hwnd);
	auto policy = std::make_unique<InstallingDefaultRendererPolicy>(std::move(creating_policy), DefaultRendererName, PrimaryTargetName);
	m_graphicMain->InstallRenderEngine(std::move(policy));
	m_rendererManager = ServiceManager::GetSystemServiceAs<RendererManager>();
	m_materialManager = ServiceManager::GetSystemServiceAs<EffectMaterialManager>();

	m_gateway = menew EffectPolicyJsonGateway();

	IFilePtr iFile = FileSystem::Instance()->OpenFile(Filename("TestEffect.efx@APK_PATH"), "rb");
	if (FATAL_LOG_EXPR(!iFile)) return;
	size_t file_size = iFile->Size();
	if (FATAL_LOG_EXPR(file_size <= 0))
	{
		FileSystem::Instance()->CloseFile(iFile);
		return;
	}

	auto code_buff = iFile->Read(0, file_size);
	if (!code_buff) return;
	FileSystem::Instance()->CloseFile(iFile);
	auto compiling_policy = m_gateway->Deserialize(convert_to_string(code_buff.value(), file_size));
	if (compiling_policy)
	{
		m_materialManager->CompileEffectMaterial(compiling_policy.value());
	}
	byte_buffer points = make_data_buffer((unsigned char*)vtx_pos, sizeof(vtx_pos));
	uint_buffer indices = make_data_buffer(vtx_idx, 6);
	RenderBufferSignature signature{ BufferSignatureName, PrimitiveTopology::Topology_TriangleList, 4, 6 };
	RenderBufferPolicy buffer_policy {RenderBufferName, signature, VertexBufferName, IndexBufferName,
		sizeof(VtxData), sizeof(vtx_pos), sizeof(vtx_idx), points, indices};
	CommandBus::Post(std::make_shared<BuildRenderBuffer>(buffer_policy));

	m_textureBuilder = menew TextureSamplerBuilder();
	m_textureBuilder->BuildTexture(TextureName, "earth.png", MediaPathName);

	m_timer = menew Timer;
	m_timer->Reset();
	m_tick = 0.0f;
}

void RenderBufferManagerTest::ShutdownEngine()
{
	delete m_gateway;
	m_gateway = nullptr;
	delete m_textureBuilder;
	m_textureBuilder = nullptr;
	delete m_timer;
	m_timer = nullptr;

	EventPublisher::Unsubscribe(typeid(PrimaryRenderTargetCreated), m_onRenderTargetCreated);
	m_onRenderTargetCreated = nullptr;
	EventPublisher::Unsubscribe(typeid(EffectMaterialCompiled), m_onEffectMaterialCompiled);
	m_onEffectMaterialCompiled = nullptr;
	EventPublisher::Unsubscribe(typeid(EffectMaterialCompileFailed), m_onEffectCompileFailed);
	m_onEffectCompileFailed = nullptr;

	EventPublisher::Unsubscribe(typeid(RenderBufferBuilt), m_onRenderBufferBuilt);
	m_onRenderBufferBuilt = nullptr;
	EventPublisher::Unsubscribe(typeid(RenderBufferBuildFailed), m_onRenderBufferBuildFailed);
	m_onRenderBufferBuildFailed = nullptr;

	EventPublisher::Unsubscribe(typeid(TextureLoaded), m_onTextureLoaded);
	m_onTextureLoaded = nullptr;

	m_renderTarget = nullptr;
	m_renderBuffer = nullptr;
	m_texture = nullptr;

	m_material = nullptr;

	m_graphicMain->ShutdownRenderEngine();
}

void RenderBufferManagerTest::FrameUpdate()
{
	AppDelegate::FrameUpdate();
	if (m_timer)
	{
		m_timer->Update();
		m_tick += m_timer->GetElapseTime();
	}
	if (m_material)
	{
		if (auto var_timer = m_material->GetEffectVariableInPassBySemantic(PassTopRightName, "ANIM_TIMER"))
		{
			var_timer.value().get().AssignValue(m_tick);
		}
		if (auto var_scale = m_material->GetEffectVariableInPassBySemantic(PassTopRightName, "ANIM_SCALE"))
		{
			var_scale.value().get().AssignValue(3.0f);
		}
		if (auto var_timer = m_material->GetEffectVariableInPassBySemantic(PassBottomLeftName, "ANIM_TIMER"))
		{
			var_timer.value().get().AssignValue(-m_tick);
		}
		if (auto var_scale = m_material->GetEffectVariableInPassBySemantic(PassBottomLeftName, "ANIM_SCALE"))
		{
			var_scale.value().get().AssignValue(1.5f);
		}
		if (auto var_timer = m_material->GetEffectVariableInPassBySemantic(PassBottomRightName, "ANIM_TIMER"))
		{
			var_timer.value().get().AssignValue(m_tick);
		}
		if (auto var_scale = m_material->GetEffectVariableInPassBySemantic(PassBottomRightName, "ANIM_SCALE"))
		{
			var_scale.value().get().AssignValue(3.0f);
		}
		if (auto var_timer = m_material->GetEffectVariableInPassBySemantic(PassTopLeftName, "ANIM_TIMER"))
		{
			var_timer.value().get().AssignValue(-m_tick);
		}
		if (auto var_scale = m_material->GetEffectVariableInPassBySemantic(PassTopLeftName, "ANIM_SCALE"))
		{
			var_scale.value().get().AssignValue(1.5f);
		}
	}
}

void RenderBufferManagerTest::RenderFrame()
{
	if ((!m_material) || (!m_renderBuffer) || (!m_renderTarget)) return;
	m_renderTarget->Bind();
	m_renderTarget->BindViewPort();
	m_renderTarget->Clear();
	IGraphicAPI::Instance()->BeginScene();
	m_material->SelectRendererTechnique("technique");
	m_material->SelectVisualTechnique("slash");
	//m_material->SelectVisualTechnique("backslash");
	m_renderBuffer->Draw(Matrix4::IDENTITY, m_material, GeometrySegment(0, 4, 0, 6));
	IGraphicAPI::Instance()->EndScene();
	m_renderTarget->Flip();
}

void RenderBufferManagerTest::OnRenderTargetCreated(const IEventPtr& e)
{
	if (!e) return;
	std::shared_ptr<PrimaryRenderTargetCreated> ev = std::dynamic_pointer_cast<PrimaryRenderTargetCreated, IEvent>(e);
	if (!ev) return;
	m_renderTarget = m_rendererManager->GetRenderTarget(ev->GetRenderTargetName());
}

void RenderBufferManagerTest::OnEffectMaterialCompiled(const IEventPtr& e)
{
	if (!e) return;
	std::shared_ptr<EffectMaterialCompiled> ev = std::dynamic_pointer_cast<EffectMaterialCompiled, IEvent>(e);
	if ((!ev) || (!ev->HasEffect())) return;
	if (ev->GetFilename() != MaterialName) return;
	m_material = ev->GetEffect();
	BuildVariables();
}

void RenderBufferManagerTest::OnEffectCompileFailed(const IEventPtr& e)
{
	if (!e) return;
	std::shared_ptr<EffectMaterialCompileFailed> ev = std::dynamic_pointer_cast<EffectMaterialCompileFailed, IEvent>(e);
	if (!ev) return;
	Enigma::Platforms::Debug::ErrorPrintf("effect material %s compile failed : %s\n", ev->GetFilename().c_str(), ev->GetErrorCode().message().c_str());
}

void RenderBufferManagerTest::OnRenderBufferBuilt(const Enigma::Frameworks::IEventPtr& e)
{
	if (!e) return;
	auto ev = std::dynamic_pointer_cast<RenderBufferBuilt, IEvent>(e);
	if (ev->GetName() != RenderBufferName) return;
	m_renderBuffer = ev->GetBuffer();
}

void RenderBufferManagerTest::OnRenderBufferBuildFailed(const Enigma::Frameworks::IEventPtr& e)
{
	if (!e) return;
	auto ev = std::dynamic_pointer_cast<RenderBufferBuildFailed, IEvent>(e);
	if (!ev) return;
	Enigma::Platforms::Debug::ErrorPrintf("render buffer %s build failed : %s\n", ev->GetName().c_str(),
		ev->GetErrorCode().message().c_str());
}

void RenderBufferManagerTest::OnTextureLoaded(const IEventPtr& e)
{
	if (!e) return;
	std::shared_ptr<TextureLoaded> ev = std::dynamic_pointer_cast<TextureLoaded, IEvent>(e);
	if (!ev) return;
	if (ev->GetTextureName() != TextureName) return;
	m_texture = IGraphicAPI::Instance()->GetGraphicAsset<ITexturePtr>(ev->GetTextureName());
	BuildVariables();
}

void RenderBufferManagerTest::BuildVariables()
{
	if ((!m_material) || (!m_texture)) return;

	auto vars = m_material->GetEffectVariablesByName("DiffuseTexture");
	if (!vars.empty())
	{
		for (auto& var : vars)
		{
			var.get().AssignValue(IShaderVariable::TextureVarTuple{ m_texture, std::nullopt });
		}
	}
	if (auto offset_var = m_material->GetEffectVariableInPassByName(PassTopLeftName, "offset_x"))
	{
		offset_var.value().get().AssignValue(-0.3f);
	}
	if (auto offset_var = m_material->GetEffectVariableInPassByName(PassTopLeftName, "offset_y"))
	{
		offset_var.value().get().AssignValue(0.3f);
	}
	if (auto offset_var = m_material->GetEffectVariableInPassByName(PassBottomLeftName, "offset_x"))
	{
		offset_var.value().get().AssignValue(-0.3f);
	}
	if (auto offset_var = m_material->GetEffectVariableInPassByName(PassBottomLeftName, "offset_y"))
	{
		offset_var.value().get().AssignValue(-0.3f);
	}
	if (auto offset_var = m_material->GetEffectVariableInPassByName(PassTopRightName, "offset_x"))
	{
		offset_var.value().get().AssignValue(0.3f);
	}
	if (auto offset_var = m_material->GetEffectVariableInPassByName(PassTopRightName, "offset_y"))
	{
		offset_var.value().get().AssignValue(0.3f);
	}
	if (auto offset_var = m_material->GetEffectVariableInPassByName(PassBottomRightName, "offset_x"))
	{
		offset_var.value().get().AssignValue(0.3f);
	}
	if (auto offset_var = m_material->GetEffectVariableInPassByName(PassBottomRightName, "offset_y"))
	{
		offset_var.value().get().AssignValue(-0.3f);
	}
}
