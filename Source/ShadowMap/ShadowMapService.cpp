﻿#include "ShadowMapService.h"
#include "ShadowMapServiceConfiguration.h"
#include "SunLightCamera.h"
#include "Frameworks/EventPublisher.h"
#include "SceneGraph/Spatial.h"
#include "GraphicKernel/IGraphicAPI.h"
#include "SceneGraph/SceneGraphEvents.h"
#include "SceneGraph/Light.h"
#include "SceneGraph/LightInfo.h"
#include "GameCommon/GameCameraService.h"
#include "GameEngine/MaterialVariableMap.h"
#include "SceneGraph/Pawn.h"

using namespace Enigma::ShadowMap;
using namespace Enigma::Frameworks;
using namespace Enigma::SceneGraph;
using namespace Enigma::Renderer;

ShadowMapService::ShadowMapService(ServiceManager* manager, const std::shared_ptr<GameCommon::GameSceneService>& scene_service,
    const std::shared_ptr<GameCommon::GameCameraService>& camera_service,
    const std::shared_ptr<RendererManager>& renderer_manager, std::unique_ptr<ShadowMapServiceConfiguration> configuration)
    : ISystemService(manager)
{
    m_needTick = true;
    m_configuration = std::move(configuration);
    m_sceneService = scene_service;
    m_cameraService = camera_service;
    m_rendererManager = renderer_manager;
}

ShadowMapService::~ShadowMapService()
{
}

ServiceResult ShadowMapService::OnInit()
{
    m_onLightInfoCreated = std::make_shared<EventSubscriber>([=](auto e) { OnLightInfoCreated(e); });
    EventPublisher::Subscribe(typeid(LightInfoCreated), m_onLightInfoCreated);
    m_onLightInfoDeleted = std::make_shared<EventSubscriber>([=](auto e) { OnLightInfoDeleted(e); });
    EventPublisher::Subscribe(typeid(LightInfoDeleted), m_onLightInfoDeleted);
    m_onLightInfoUpdated = std::make_shared<EventSubscriber>([=](auto e) { OnLightInfoUpdated(e); });
    EventPublisher::Subscribe(typeid(LightInfoUpdated), m_onLightInfoUpdated);
    m_onPawnPrimitiveBuilt = std::make_shared<EventSubscriber>([=](auto e) { OnPawnPrimitiveBuilt(e); });
    EventPublisher::Subscribe(typeid(PawnPrimitiveBuilt), m_onPawnPrimitiveBuilt);

    Engine::MaterialVariableMap::InsertAutoVariableFunctionToMap(m_configuration->LightViewProjSemantic(), [=](auto v) { AssignLightViewProjectionTransform(v); });
    return ServiceResult::Complete;
}

ServiceResult ShadowMapService::OnTick()
{
    if ((!m_sceneService.expired()) && (m_sunLightCamera))
    {
        m_sunLightCamera->CalcLightCameraSystemMatrix(m_sceneService.lock()->GetSceneCuller());
    }
    return ServiceResult::Pendding;
}

ServiceResult ShadowMapService::OnTerm()
{
    EventPublisher::Unsubscribe(typeid(LightInfoCreated), m_onLightInfoCreated);
    m_onLightInfoCreated = nullptr;
    EventPublisher::Unsubscribe(typeid(LightInfoDeleted), m_onLightInfoDeleted);
    m_onLightInfoDeleted = nullptr;
    EventPublisher::Unsubscribe(typeid(LightInfoUpdated), m_onLightInfoUpdated);
    m_onLightInfoUpdated = nullptr;
    EventPublisher::Unsubscribe(typeid(PawnPrimitiveBuilt), m_onPawnPrimitiveBuilt);
    m_onPawnPrimitiveBuilt = nullptr;

    return ServiceResult::Complete;
}

void ShadowMapService::CreateShadowRenderSystem(const std::string& renderer_name, const std::string& target_name)
{
    assert(!m_rendererManager.expired());
    m_rendererManager.lock()->CreateRenderer(renderer_name);
    m_rendererManager.lock()->CreateRenderTarget(target_name, RenderTarget::PrimaryType::NotPrimary, { Graphics::RenderTextureUsage::ShadowMap });

    m_renderer = std::dynamic_pointer_cast<Renderer::Renderer, Engine::IRenderer>(m_rendererManager.lock()->GetRenderer(renderer_name));
    m_renderer.lock()->SelectRendererTechnique(m_configuration->ShadowMapTechniqueName());

    m_shadowMapRenderTarget = m_rendererManager.lock()->GetRenderTarget(target_name);
    m_shadowMapRenderTarget.lock()->InitBackSurface(m_configuration->ShadowMapSurfaceName(), m_configuration->ShadowMapDimension(), Graphics::GraphicFormat::FMT_R32F);
    m_shadowMapRenderTarget.lock()->InitDepthStencilSurface(m_configuration->ShadowMapDepthName(), m_configuration->ShadowMapDimension(),
        Graphics::IGraphicAPI::Instance()->GetDepthSurfaceFormat());
    m_renderer.lock()->SetRenderTarget(m_shadowMapRenderTarget.lock());
    m_shadowMapRenderTarget.lock()->ChangeClearingProperty(RenderTargetClearChangingProperty{ MathLib::ColorRGBA(1.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0, std::nullopt });

    if (m_sunLightCamera)
    {
        m_renderer.lock()->SetAssociatedCamera(m_sunLightCamera);
    }
}

void ShadowMapService::DestroyShadowRenderSystem(const std::string& renderer_name, const std::string& target_name)
{
    assert(!m_rendererManager.expired());
    m_rendererManager.lock()->DestroyRenderTarget(target_name);
    m_rendererManager.lock()->DestroyRenderer(renderer_name);
}

void ShadowMapService::PrepareShadowScene()
{
    if ((!m_renderer.expired()) && (!m_sceneService.expired()) && (m_sceneService.lock()->GetSceneCuller()))
    {
        m_renderer.lock()->PrepareScene(m_sceneService.lock()->GetSceneCuller()->GetVisibleSet(),
            Spatial::SpatialBit::Spatial_ShadowCaster);
    }
}

void ShadowMapService::RenderShadowScene()
{
    if (m_renderer.expired()) return;
    m_renderer.lock()->ClearRenderTarget();
    m_renderer.lock()->BeginScene();
    m_renderer.lock()->DrawScene();
    m_renderer.lock()->EndScene();
}

void ShadowMapService::OnLightInfoCreated(const IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<LightInfoCreated, IEvent>(e);
    if ((!ev) || (!ev->GetLight())) return;
    if (ev->GetLight()->Info().GetLightType() != LightInfo::LightType::SunLight) return;
    CreateSunLightCamera(ev->GetLight());
}

void ShadowMapService::OnLightInfoDeleted(const IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<LightInfoDeleted, IEvent>(e);
    if (!ev) return;
    if (ev->GetLight()->Info().GetLightType() != LightInfo::LightType::SunLight) return;
    if (!m_renderer.expired())
    {
        m_renderer.lock()->SetAssociatedCamera(nullptr);
    }
    m_sunLightCamera = nullptr;
}

void ShadowMapService::OnLightInfoUpdated(const IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<LightInfoUpdated, IEvent>(e);
    if ((!ev) || (!ev->GetLight())) return;
    if (ev->GetLight()->Info().GetLightType() != LightInfo::LightType::SunLight) return;
    if (m_sunLightCamera)
    {
        m_sunLightCamera->SetSunLightDir(ev->GetLight()->GetLightDirection());
    }
}

void ShadowMapService::OnPawnPrimitiveBuilt(const IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<PawnPrimitiveBuilt, IEvent>(e);
    if ((!ev) || (!ev->GetPawn())) return;
    if ((ev->GetPawn()->TestSpatialFlag(Spatial::SpatialBit::Spatial_ShadowCaster))
        || (ev->GetPawn()->TestSpatialFlag(Spatial::SpatialBit::Spatial_ShadowReceiver)))
    {
        BindShadowMapToPawn(ev->GetPawn());
    }
}

void ShadowMapService::CreateSunLightCamera(const std::shared_ptr<Light>& lit)
{
    assert(!m_cameraService.expired());
    m_sunLightCamera = std::make_shared<SunLightCamera>(m_configuration->SunLightCameraName());
    MathLib::Vector3 vecSunDir = MathLib::Vector3(-1.0f, -1.0f, 0.0f);
    if (lit) vecSunDir = lit->GetLightDirection();
    m_sunLightCamera->SetSunLightDir(vecSunDir);
    if (auto cam = m_cameraService.lock()->GetPrimaryCamera())
    {
        m_sunLightCamera->SetViewerCamera(cam);
    }
    if (!m_renderer.expired())
    {
        m_renderer.lock()->SetAssociatedCamera(m_sunLightCamera);
    }
}

void ShadowMapService::BindShadowMapToPawn(const std::shared_ptr<Pawn>& pawn)
{
    if (!pawn) return;
    if (!pawn->GetPrimitive()) return;
    if (const auto model = std::dynamic_pointer_cast<ModelPrimitive, Engine::Primitive>(pawn->GetPrimitive()))
    {
        const auto mesh_count = model->GetMeshPrimitiveCount();
        for (unsigned i = 0; i < mesh_count; i++)
        {
            BindShadowMapToMesh(model->GetMeshPrimitive(i));
        }
    }
    else if (const auto mesh = std::dynamic_pointer_cast<MeshPrimitive, Engine::Primitive>(pawn->GetPrimitive()))
    {
        BindShadowMapToMesh(mesh);
    }
}

void ShadowMapService::BindShadowMapToMesh(const std::shared_ptr<MeshPrimitive>& mesh)
{
    if (!mesh) return;
    if (m_shadowMapRenderTarget.expired()) return;

    Engine::EffectTextureMap::EffectTextures textures = {
        { m_configuration->ShadowMapSemantic(), m_shadowMapRenderTarget.lock()->GetRenderTargetTexture(), std::nullopt },
    };
    mesh->ChangeTextureMap({ textures });
}

void ShadowMapService::AssignLightViewProjectionTransform(Engine::EffectVariable& var)
{
    if (m_sunLightCamera)
    {
        var.AssignValue(m_sunLightCamera->GetLightViewProjMatrix());
    }
}
