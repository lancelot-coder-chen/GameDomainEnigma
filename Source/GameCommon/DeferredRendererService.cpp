﻿#include "DeferredRendererService.h"
#include "DeferredRendererServiceConfiguration.h"
#include "GameCameraEvents.h"
#include "GameLightEvents.h"
#include "GameSceneCommands.h"
#include "GameSceneService.h"
#include "LightVolumePawn.h"
#include "LightQuadPawn.h"
#include "Controllers/GraphicMain.h"
#include "Frameworks/CommandBus.h"
#include "Platforms/PlatformLayer.h"
#include "Renderer/DeferredRenderer.h"
#include "SceneGraph/SceneGraphEvents.h"
#include "SceneGraph/Light.h"
#include "GameEngine/EffectDtoHelper.h"
#include "GameEngine/StandardGeometryDtoHelper.h"
#include "Renderer/RenderablePrimitiveDtos.h"
#include "Renderer/RenderablePrimitiveRequests.h"
#include "Renderer/RenderablePrimitiveResponses.h"
#include "Frameworks/EventPublisher.h"
#include "Frameworks/RequestBus.h"
#include "Frameworks/ResponseBus.h"
#include "Renderer/RendererEvents.h"
#include "SceneGraph/SceneGraphCommands.h"
#include "SceneGraph/SceneGraphDtoHelper.h"
#include "GameCommon/GameCameraService.h"

using namespace Enigma::GameCommon;
using namespace Enigma::Frameworks;
using namespace Enigma::Renderer;
using namespace Enigma::Engine;
using namespace Enigma::SceneGraph;


DEFINE_RTTI(GameCommon, DeferredRendererService, SceneRendererService);

//!  samples 的數量太少，光球的 poly 太粗，會導致算出的受光亮面偏移
#define SPHERE_STACKS 40
#define SPHERE_SLICES 80

std::string DeferredSunLightQuadName = "_deferred_sun_light_quad_";
std::string DeferredAmbientLightQuadName = "_deferred_ambient_light_quad_";

DeferredRendererService::DeferredRendererService(ServiceManager* mngr,
    const std::shared_ptr<GameSceneService>& scene_service, const std::shared_ptr<GameCameraService>& camera_service,
    const std::shared_ptr<Renderer::RendererManager>& renderer_manager,
    const std::shared_ptr<SceneGraph::SceneGraphRepository>& scene_graph_repository,
    const std::shared_ptr<DeferredRendererServiceConfiguration>& configuration) : SceneRendererService(mngr, scene_service, camera_service, renderer_manager, configuration)
{
    m_configuration = configuration;
    m_sceneGraphRepository = scene_graph_repository;
    LightVolumePawn::SetDefaultVisualTech(m_configuration->VisualTechniqueNameForCameraDefault());
    LightVolumePawn::SetInsideVisualTech(m_configuration->VisualTechniqueNameForCameraInside());
}

DeferredRendererService::~DeferredRendererService()
{
    m_configuration = nullptr;
    m_lightingPawns.clear();
}

ServiceResult DeferredRendererService::OnInit()
{
    m_onPrimaryRenderTargetCreated = std::make_shared<EventSubscriber>([=](auto e) { OnPrimaryRenderTargetCreated(e); });
    EventPublisher::Subscribe(typeid(Renderer::PrimaryRenderTargetCreated), m_onPrimaryRenderTargetCreated);
    m_onPrimaryRenderTargetResized = std::make_shared<EventSubscriber>([=](auto e) { OnPrimaryRenderTargetResized(e); });
    EventPublisher::Subscribe(typeid(Renderer::RenderTargetResized), m_onPrimaryRenderTargetResized);
    m_onGameCameraUpdated = std::make_shared<EventSubscriber>([=](auto e) { OnGameCameraUpdated(e); });
    EventPublisher::Subscribe(typeid(GameCameraUpdated), m_onGameCameraUpdated);
    m_onSceneGraphChanged = std::make_shared<EventSubscriber>([=](auto e) { OnSceneGraphChanged(e); });
    EventPublisher::Subscribe(typeid(SceneGraph::SceneGraphChanged), m_onSceneGraphChanged);
    m_onGBufferTextureCreated = std::make_shared<EventSubscriber>([=](auto e) { OnGBufferTextureCreated(e); });
    EventPublisher::Subscribe(typeid(Renderer::RenderTargetTextureCreated), m_onGBufferTextureCreated);
    m_onGameLightCreated = std::make_shared<EventSubscriber>([=](auto e) { OnGameLightCreated(e); });
    EventPublisher::Subscribe(typeid(GameLightCreated), m_onGameLightCreated);
    m_onLightInfoDeleted = std::make_shared<EventSubscriber>([=](auto e) { OnLightInfoDeleted(e); });
    EventPublisher::Subscribe(typeid(SceneGraph::LightInfoDeleted), m_onLightInfoDeleted);
    m_onLightInfoUpdated = std::make_shared<EventSubscriber>([=](auto e) { OnLightInfoUpdated(e); });
    EventPublisher::Subscribe(typeid(SceneGraph::LightInfoUpdated), m_onLightInfoUpdated);
    m_onSceneGraphBuilt = std::make_shared<EventSubscriber>([=](auto e) { OnSceneGraphBuilt(e); });
    EventPublisher::Subscribe(typeid(SceneGraph::FactorySceneGraphBuilt), m_onSceneGraphBuilt);
    m_onPawnPrimitiveBuilt = std::make_shared<EventSubscriber>([=](auto e) { OnPawnPrimitiveBuilt(e); });
    EventPublisher::Subscribe(typeid(SceneGraph::PawnPrimitiveBuilt), m_onPawnPrimitiveBuilt);

    return SceneRendererService::OnInit();
}

ServiceResult DeferredRendererService::OnTerm()
{
    EventPublisher::Unsubscribe(typeid(Renderer::PrimaryRenderTargetCreated), m_onPrimaryRenderTargetCreated);
    m_onPrimaryRenderTargetCreated = nullptr;
    EventPublisher::Unsubscribe(typeid(Renderer::RenderTargetResized), m_onPrimaryRenderTargetResized);
    m_onPrimaryRenderTargetResized = nullptr;
    EventPublisher::Unsubscribe(typeid(GameCameraUpdated), m_onGameCameraUpdated);
    m_onGameCameraUpdated = nullptr;
    EventPublisher::Unsubscribe(typeid(SceneGraph::SceneGraphChanged), m_onSceneGraphChanged);
    m_onSceneGraphChanged = nullptr;
    EventPublisher::Unsubscribe(typeid(Renderer::RenderTargetTextureCreated), m_onGBufferTextureCreated);
    m_onGBufferTextureCreated = nullptr;
    EventPublisher::Unsubscribe(typeid(GameLightCreated), m_onGameLightCreated);
    m_onGameLightCreated = nullptr;
    EventPublisher::Unsubscribe(typeid(SceneGraph::LightInfoDeleted), m_onLightInfoDeleted);
    m_onLightInfoDeleted = nullptr;
    EventPublisher::Unsubscribe(typeid(SceneGraph::LightInfoUpdated), m_onLightInfoUpdated);
    m_onLightInfoUpdated = nullptr;
    EventPublisher::Unsubscribe(typeid(SceneGraph::FactorySceneGraphBuilt), m_onSceneGraphBuilt);
    m_onSceneGraphBuilt = nullptr;
    EventPublisher::Unsubscribe(typeid(SceneGraph::PawnPrimitiveBuilt), m_onPawnPrimitiveBuilt);
    m_onPawnPrimitiveBuilt = nullptr;

    m_lightingPawns.clear();

    return SceneRendererService::OnTerm();
}

void DeferredRendererService::CreateSceneRenderSystem(const std::string& renderer_name, const std::string& target_name)
{
    assert(!m_rendererManager.expired());
    assert(m_configuration);
    auto rendererManager = m_rendererManager.lock();
    Engine::IRendererPtr renderer = std::make_shared<Renderer::DeferredRenderer>(renderer_name);
    error er = rendererManager->InsertRenderer(renderer_name, renderer);
    if (er) return;

    RenderTargetPtr primaryTarget = rendererManager->GetPrimaryRenderTarget();
    if (!primaryTarget)
    {
        rendererManager->CreateRenderTarget(target_name, RenderTarget::PrimaryType::IsPrimary, { Graphics::RenderTextureUsage::Default });
        primaryTarget = rendererManager->GetPrimaryRenderTarget();
    }
    if (FATAL_LOG_EXPR(!primaryTarget)) return;

    m_renderer = std::dynamic_pointer_cast<Renderer::Renderer, Engine::IRenderer>(rendererManager->GetRenderer(renderer_name));
    m_renderer.lock()->SelectRendererTechnique(m_configuration->DeferredRendererTechniqueName());
    m_renderer.lock()->SetRenderTarget(primaryTarget);
    if (primaryTarget->GetBackSurface() && primaryTarget->GetDepthStencilSurface())
    {
        CreateGBuffer(primaryTarget);
    }
}

void DeferredRendererService::DestroySceneRenderSystem(const std::string& renderer_name, const std::string& target_name)
{
    assert(!m_rendererManager.expired());
    assert(m_configuration);
    m_rendererManager.lock()->DestroyRenderTarget(target_name);
    m_rendererManager.lock()->DestroyRenderTarget(m_configuration->GbufferTargetName());
    m_rendererManager.lock()->DestroyRenderer(renderer_name);
}

void DeferredRendererService::PrepareGameScene()
{
    if (!m_renderer.expired())
    {
        //if (!m_ambientLightQuad.expired()) m_ambientLightQuad.lock()->InsertToRendererWithTransformUpdating(m_renderer.lock(),
          //  MathLib::Matrix4::IDENTITY, m_ambientQuadLightingState);
        //if (!m_sunLightQuad.expired()) m_sunLightQuad.lock()->InsertToRendererWithTransformUpdating(m_renderer.lock(),
          //  MathLib::Matrix4::IDENTITY, m_sunLightQuadLightingState);
    }
    SceneRendererService::PrepareGameScene();
}

void DeferredRendererService::CreateGBuffer(const Renderer::RenderTargetPtr& primary_target)
{
    assert(primary_target);
    assert(!m_rendererManager.expired());
    assert(m_configuration);

    auto [width, height] = primary_target->GetDimension();

    m_rendererManager.lock()->CreateRenderTarget(m_configuration->GbufferTargetName(), RenderTarget::PrimaryType::NotPrimary,
        m_configuration->GbufferUsages());
    m_gBuffer = m_rendererManager.lock()->GetRenderTarget(m_configuration->GbufferTargetName());
    if (!m_gBuffer.expired())
    {
        m_gBuffer.lock()->InitMultiBackSurface(m_configuration->GbufferSurfaceName(), MathLib::Dimension{ width, height }, static_cast<unsigned>(m_configuration->GbufferFormats().size()),
            m_configuration->GbufferFormats());
        m_gBuffer.lock()->ShareDepthStencilSurface(m_configuration->GbufferDepthName(), primary_target->GetDepthStencilSurface());
    }
    if (const auto deferRender = std::dynamic_pointer_cast<DeferredRenderer, Renderer::Renderer>(m_renderer.lock()))
    {
        deferRender->AttachGBufferTarget(m_gBuffer.lock());
    }
    //if (!m_ambientLightPawn.expired()) BindGBufferToLightQuad(m_ambientLightPawn.lock());
    //if (!m_sunLightPawn.expired()) BindGBufferToLightQuad(m_sunLightPawn.lock());
    for (auto& light : m_lightingPawns)
    {
        if (!light.second.expired()) BindGBufferToLightingPawn(light.second.lock());
    }
}

void DeferredRendererService::OnPrimaryRenderTargetCreated(const Frameworks::IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<Renderer::PrimaryRenderTargetCreated, Frameworks::IEvent>(e);
    if (!ev) return;
    auto primaryTarget = ev->GetRenderTarget();
    if (!primaryTarget) return;

    CreateGBuffer(primaryTarget);
}

void DeferredRendererService::OnPrimaryRenderTargetResized(const Frameworks::IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<Renderer::RenderTargetResized, Frameworks::IEvent>(e);
    if (!ev) return;
    const auto target = ev->GetRenderTarget();
    if ((!target) || (!target->IsPrimary())) return;

    if ((!m_gBuffer.expired()) && (m_gBuffer.lock()->GetDimension() != target->GetDimension())) m_gBuffer.lock()->Resize(target->GetDimension());
}

void DeferredRendererService::OnGameCameraUpdated(const Frameworks::IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<GameCommon::GameCameraUpdated, Frameworks::IEvent>(e);
    if (!ev) return;
    if (!ev->GetCamera()) return;
    for (auto& kv : m_lightingPawns)
    {
        if (kv.second.expired()) continue;
        if (auto volume = std::dynamic_pointer_cast<LightVolumePawn, LightingPawn>(kv.second.lock()))
        {
            CheckLightVolumeBackfaceCulling(volume, ev->GetCamera());
        }
    }
}

void DeferredRendererService::OnSceneGraphChanged(const Frameworks::IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<SceneGraph::SceneGraphChanged, Frameworks::IEvent>(e);
    if ((!ev) || (ev->GetChild())) return;
    // 抓light entity 被 attach 的訊息來改變 light volume 的 parent
    if (!ev->GetChild()->TypeInfo().IsDerived(Light::TYPE_RTTI)) return;
    if (ev->GetNotifyCode() != SceneGraphChanged::NotifyCode::AttachChild) return;
    const auto light = std::dynamic_pointer_cast<Light, Spatial>(ev->GetChild());
    if (!light) return;
    const auto lightPawn = FindLightingPawn(light->GetSpatialName());
    auto parent_node = std::dynamic_pointer_cast<Node, Spatial>(ev->GetParentNode());
    if (lightPawn) lightPawn->ChangeWorldPosition(lightPawn->GetWorldPosition(), parent_node);
}

void DeferredRendererService::OnGBufferTextureCreated(const Frameworks::IEventPtr& e)
{
    assert(!m_gBuffer.expired());
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<Renderer::RenderTargetTextureCreated, Frameworks::IEvent>(e);
    if (!ev) return;
    if (ev->GetRenderTarget() != m_gBuffer.lock()) return;
    //if (!m_ambientLightPawn.expired()) BindGBufferToLightQuad(m_ambientLightPawn.lock());
    //if (!m_sunLightPawn.expired()) BindGBufferToLightQuad(m_sunLightPawn.lock());
    for (auto& light : m_lightingPawns)
    {
        if (!light.second.expired()) BindGBufferToLightingPawn(light.second.lock());
    }
}

void DeferredRendererService::OnGameLightCreated(const Frameworks::IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<GameLightCreated, Frameworks::IEvent>(e);
    if ((!ev) || (!ev->GetLight())) return;
    if (ev->GetLight()->Info().GetLightType() == SceneGraph::LightInfo::LightType::Ambient)
    {
        CreateAmbientLightQuad(ev->GetLight());
    }
    else if (ev->GetLight()->Info().GetLightType() == SceneGraph::LightInfo::LightType::SunLight)
    {
        CreateSunLightQuad(ev->GetLight());
    }
    else if (ev->GetLight()->Info().GetLightType() == SceneGraph::LightInfo::LightType::Point)
    {
        CreatePointLightVolume(ev->GetLight());
    }
}

void DeferredRendererService::OnLightInfoDeleted(const Frameworks::IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<SceneGraph::LightInfoDeleted, Frameworks::IEvent>(e);
    if (!ev) return;
    RemoveLightingPawn(ev->GetLightName());
    /*if (ev->GetLightType() == SceneGraph::LightInfo::LightType::Ambient)
    {
        m_ambientLightPawn.reset();
        //m_ambientLightQuad.reset();
    }
    else if (ev->GetLightType() == SceneGraph::LightInfo::LightType::SunLight)
    {
        m_sunLightPawn.reset();
        //m_sunLightQuad.reset();
    }
    else if (ev->GetLightType() == SceneGraph::LightInfo::LightType::Point)
    {
        RemovePointLightVolume(ev->GetLightName());
    }*/
    CommandBus::Post(std::make_shared<DeleteSceneSpatial>(ev->GetLightName()));
}

void DeferredRendererService::OnLightInfoUpdated(const Frameworks::IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<SceneGraph::LightInfoUpdated, Frameworks::IEvent>(e);
    if ((!ev) || (!ev->GetLight())) return;
    if (ev->GetLight()->Info().GetLightType() == SceneGraph::LightInfo::LightType::Ambient)
    {
        UpdateAmbientLightQuad(ev->GetLight(), ev->GetNotifyCode());
    }
    else if (ev->GetLight()->Info().GetLightType() == SceneGraph::LightInfo::LightType::SunLight)
    {
        UpdateSunLightQuad(ev->GetLight(), ev->GetNotifyCode());
    }
    else if (ev->GetLight()->Info().GetLightType() == SceneGraph::LightInfo::LightType::Point)
    {
        UpdatePointLightVolume(ev->GetLight(), ev->GetNotifyCode());
    }
}

void DeferredRendererService::OnSceneGraphBuilt(const Frameworks::IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<SceneGraph::FactorySceneGraphBuilt, Frameworks::IEvent>(e);
    if (!ev) return;
    auto top_spatials = ev->GetTopLevelSpatial();
    if (top_spatials.empty()) return;
    if (!top_spatials[0]) return;
    if (auto pawn = std::dynamic_pointer_cast<LightQuadPawn, Spatial>(top_spatials[0]))
    {
        OnLightQuadBuilt(ev->GetSceneGraphId(), top_spatials[0]);
        //m_sunLightPawn = std::dynamic_pointer_cast<LightQuadPawn, Spatial>(top_spatials[0]);
    }
    else
    {
        OnLightVolumeBuilt(ev->GetSceneGraphId(), top_spatials[0]);
    }
    if (const auto lit = m_buildingLightPawns.find(top_spatials[0]->GetSpatialName()); lit != m_buildingLightPawns.end())
    {
        if (!lit->second.m_parentNodeName.empty())
        {
            CommandBus::Post(std::make_shared<AttachNodeChild>(lit->second.m_parentNodeName, top_spatials[0], lit->second.m_localTransform));
        }
        m_buildingLightPawns.erase(lit);
    }
}

void DeferredRendererService::OnLightVolumeBuilt(const std::string& lit_name, const std::shared_ptr<SceneGraph::Spatial>& spatial)
{
    auto pawn = std::dynamic_pointer_cast<LightVolumePawn, Spatial>(spatial);
    if (!pawn) return;
    /*if ((!m_sceneService.expired()) && (m_sceneService.lock()->GetSceneRoot()))
    {
        m_sceneService.lock()->GetSceneRoot()->AttachChild(pawn, pawn->GetLocalTransform());
    }*/
    if (!m_sceneGraphRepository.expired())
    {
        pawn->SetHostLight(m_sceneGraphRepository.lock()->QueryLight(lit_name));
    }
    m_lightingPawns.insert_or_assign(lit_name, pawn);
    BindGBufferToLightingPawn(pawn);
    CheckLightVolumeBackfaceCulling(lit_name);
}

void DeferredRendererService::OnLightQuadBuilt(const std::string& lit_name, const std::shared_ptr<SceneGraph::Spatial>& spatial)
{
    auto pawn = std::dynamic_pointer_cast<LightQuadPawn, Spatial>(spatial);
    if (!pawn) return;
    if (!m_sceneGraphRepository.expired())
    {
        pawn->SetHostLight(m_sceneGraphRepository.lock()->QueryLight(lit_name));
    }
    m_lightingPawns.insert_or_assign(lit_name, pawn);
    BindGBufferToLightingPawn(pawn);
    /*if (lit_name.find("sun") != std::string::npos)
    {
        m_sunLightPawn = pawn;
    }
    else if (lit_name.find("amb") != std::string::npos)
    {
        m_ambientLightPawn = pawn;
    }
    if (!m_sceneGraphRepository.expired())
    {
        pawn->SetHostLight(m_sceneGraphRepository.lock()->QueryLight(lit_name));
    }
    BindGBufferToLightQuad(pawn);*/
}

void DeferredRendererService::OnPawnPrimitiveBuilt(const IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<PawnPrimitiveBuilt, IEvent>(e);
    if ((!ev) || (!ev->GetPawn())) return;
    auto lighting_pawn = std::dynamic_pointer_cast<LightingPawn, Pawn>(ev->GetPawn());
    if (!lighting_pawn) return;
    BindGBufferToLightingPawn(lighting_pawn);
    lighting_pawn->NotifySpatialRenderStateChanged();
    CheckLightVolumeBackfaceCulling(lighting_pawn->GetHostLightName());

    //OnLightingPawnPrimitiveBuilt(lighting_pawn);
    /*if ((!m_sunLightPawn.expired()) && (ev->GetPawn() == m_sunLightPawn.lock()))
    {
        BindGBufferToLightQuad(m_sunLightPawn.lock());
        m_sunLightPawn.lock()->NotifySpatialRenderStateChanged();
        //m_sunLightQuad = std::dynamic_pointer_cast<MeshPrimitive, Primitive>(ev->GetPawn()->GetPrimitive());
        //if (!m_sunLightQuad.expired()) BindGBufferToLightingMesh(m_sunLightQuad.lock());
    }
    else if ((!m_ambientLightPawn.expired()) && (ev->GetPawn() == m_ambientLightPawn.lock()))
    {
        BindGBufferToLightQuad(m_ambientLightPawn.lock());
        m_ambientLightPawn.lock()->NotifySpatialRenderStateChanged();
        //m_ambientLightQuad = std::dynamic_pointer_cast<MeshPrimitive, Primitive>(ev->GetPawn()->GetPrimitive());
        //if (!m_ambientLightQuad.expired()) BindGBufferToLightingMesh(m_ambientLightQuad.lock());
    }
    else
    {
        OnLightVolumePrimitiveBuilt(ev->GetPawn());
    }*/
}

void DeferredRendererService::OnLightingPawnPrimitiveBuilt(const std::shared_ptr<LightingPawn>& lighting_pawn)
{
    /*auto pawn = std::dynamic_pointer_cast<LightVolumePawn, Pawn>(volume);
    if (!pawn) return;
    BindGBufferToLightVolume(pawn);*/
    if (!lighting_pawn) return;
    lighting_pawn->NotifySpatialRenderStateChanged();
    CheckLightVolumeBackfaceCulling(lighting_pawn->GetHostLightName());
}

void DeferredRendererService::CreateAmbientLightQuad(const std::shared_ptr<SceneGraph::Light>& lit)
{
    assert(lit);
    std::string quad_geo_name = lit->GetSpatialName() + "_lit_quad" + ".geo";
    SquareQuadDtoHelper quad_dto_helper(quad_geo_name);
    quad_dto_helper.XYQuad(MathLib::Vector3(-1.0f, -1.0f, 0.5f), MathLib::Vector3(1.0f, 1.0f, 0.5f))
        .TextureCoord(MathLib::Vector2(0.0f, 1.0f), MathLib::Vector2(1.0f, 0.0f));
    EffectMaterialDtoHelper eff_dto_helper(m_configuration->AmbientEffectName());
    eff_dto_helper.FilenameAtPath(m_configuration->AmbientPassFxFileName());

    MeshPrimitiveDto mesh_dto;
    mesh_dto.Name() = quad_geo_name;
    mesh_dto.GeometryName() = quad_geo_name;
    mesh_dto.TheGeometry() = quad_dto_helper.ToGenericDto();
    mesh_dto.Effects().emplace_back(eff_dto_helper.ToGenericDto());
    mesh_dto.RenderListID() = Renderer::Renderer::RenderListID::DeferredLighting;

    PawnDtoHelper pawn_helper(lit->GetSpatialName() + "_lit_quad");
    pawn_helper.MeshPrimitive(mesh_dto)
        .SpatialFlags(Spatial::Spatial_BelongToParent).TopLevel(true)
        .Factory(FactoryDesc(LightQuadPawn::TYPE_RTTI.GetName()));
    auto pawn_dto = pawn_helper.ToGenericDto();
    auto dtos = { pawn_dto };
    CommandBus::Post(std::make_shared<BuildSceneGraph>(lit->GetSpatialName(), dtos));
    InsertLightPawnBuildingMeta(pawn_dto.GetName(), lit);
    m_ambientQuadLightingState.SetAmbientLightColor(lit->GetLightColor());
}

void DeferredRendererService::CreateSunLightQuad(const std::shared_ptr<SceneGraph::Light>& lit)
{
    assert(lit);
    std::string quad_geo_name = lit->GetSpatialName() + "_lit_quad" + ".geo";
    SquareQuadDtoHelper quad_dto_helper(quad_geo_name);
    quad_dto_helper.XYQuad(MathLib::Vector3(-1.0f, -1.0f, 0.5f), MathLib::Vector3(1.0f, 1.0f, 0.5f))
        .TextureCoord(MathLib::Vector2(0.0f, 1.0f), MathLib::Vector2(1.0f, 0.0f));
    EffectMaterialDtoHelper eff_dto_helper(m_configuration->SunLightEffectName());
    eff_dto_helper.FilenameAtPath(m_configuration->SunLightPassFxFileName());

    MeshPrimitiveDto mesh_dto;
    mesh_dto.Name() = quad_geo_name;
    mesh_dto.GeometryName() = quad_geo_name;
    mesh_dto.TheGeometry() = quad_dto_helper.ToGenericDto();
    mesh_dto.Effects().emplace_back(eff_dto_helper.ToGenericDto());
    mesh_dto.RenderListID() = Renderer::Renderer::RenderListID::DeferredLighting;

    PawnDtoHelper pawn_helper(lit->GetSpatialName() + "_lit_quad");
    pawn_helper.MeshPrimitive(mesh_dto)
        .SpatialFlags(m_configuration->SunLightSpatialFlags()).TopLevel(true)
        .Factory(FactoryDesc(LightQuadPawn::TYPE_RTTI.GetName()));
    auto pawn_dto = pawn_helper.ToGenericDto();
    auto dtos = { pawn_dto };
    CommandBus::Post(std::make_shared<BuildSceneGraph>(lit->GetSpatialName(), dtos));
    InsertLightPawnBuildingMeta(pawn_dto.GetName(), lit);
    m_sunLightQuadLightingState.SetSunLight(lit->GetLightDirection(), lit->GetLightColor());
}

void DeferredRendererService::CreatePointLightVolume(const std::shared_ptr<SceneGraph::Light>& lit)
{
    assert(lit);
    std::string vol_geo_name = "deferred_" + lit->GetSpatialName() + "_lit_volume.geo";
    SphereDtoHelper sphere_dto_helper(vol_geo_name);
    sphere_dto_helper.Sphere(MathLib::Vector3::ZERO, lit->GetLightRange(), SPHERE_SLICES, SPHERE_STACKS).BoxBound();
    EffectMaterialDtoHelper eff_dto_helper(m_configuration->LightVolumeEffectName());
    eff_dto_helper.FilenameAtPath(m_configuration->LightVolumePassFxFileName());

    MeshPrimitiveDto mesh_dto;
    mesh_dto.Name() = vol_geo_name;
    mesh_dto.GeometryName() = vol_geo_name;
    mesh_dto.TheGeometry() = sphere_dto_helper.ToGenericDto();
    mesh_dto.Effects().emplace_back(eff_dto_helper.ToGenericDto());
    mesh_dto.RenderListID() = Renderer::Renderer::RenderListID::DeferredLighting;

    PawnDtoHelper pawn_helper(lit->GetSpatialName() + "_lit_volume");
    pawn_helper.Factory(FactoryDesc(LightVolumePawn::TYPE_RTTI.GetName())).MeshPrimitive(mesh_dto)
        .SpatialFlags(Spatial::Spatial_BelongToParent).TopLevel(true).LocalTransform(lit->GetLocalTransform());
    auto pawn_dto = pawn_helper.ToGenericDto();
    auto dtos = { pawn_dto };
    CommandBus::Post(std::make_shared<BuildSceneGraph>(lit->GetSpatialName(), dtos));
    InsertLightPawnBuildingMeta(pawn_dto.GetName(), lit);
}

void DeferredRendererService::InsertLightPawnBuildingMeta(const std::string& pawn_name, const std::shared_ptr<SceneGraph::Light>& lit)
{
    if (!lit) return;
    SceneGraphLightPawnMeta meta;
    if (lit->GetParent()) meta.m_parentNodeName = lit->GetParent()->GetSpatialName();
    meta.m_localTransform = lit->GetLocalTransform();
    m_buildingLightPawns.insert({ pawn_name, meta });
}

void DeferredRendererService::RemoveLightingPawn(const std::string& name)
{
    if (const auto it = m_lightingPawns.find(name); it != m_lightingPawns.end())
    {
        m_lightingPawns.erase(it);
    }
}

void DeferredRendererService::UpdateAmbientLightQuad(const std::shared_ptr<SceneGraph::Light>& lit, SceneGraph::LightInfoUpdated::NotifyCode notify)
{
    assert(lit);
    if (notify != LightInfoUpdated::NotifyCode::Color) return;
    const auto& pawn = FindLightingPawn(lit->GetSpatialName());
    if (!pawn) return;
    pawn->NotifySpatialRenderStateChanged();
    //m_ambientQuadLightingState.SetAmbientLightColor(lit->GetLightColor());
}

void DeferredRendererService::UpdatePointLightVolume(const std::shared_ptr<SceneGraph::Light>& lit, SceneGraph::LightInfoUpdated::NotifyCode notify)
{
    assert(lit);
    if (lit->Info().GetLightType() != LightInfo::LightType::Point) return;
    const auto& pawn = FindLightingPawn(lit->GetSpatialName());
    if (!pawn) return;
    if (notify == LightInfoUpdated::NotifyCode::Position)
    {
        pawn->ChangeWorldPosition(lit->GetLightPosition(), std::nullopt);
    }
    else if (notify == LightInfoUpdated::NotifyCode::Range)
    {
        pawn->SetLocalUniformScale(lit->GetLightRange());
    }
    else if (notify == LightInfoUpdated::NotifyCode::Enable)
    {
        if (lit->Info().IsEnable())
        {
            pawn->RemoveSpatialFlag(Spatial::Spatial_Hide);
        }
        else
        {
            pawn->AddSpatialFlag(Spatial::Spatial_Hide);
        }
    }
    else if (notify == LightInfoUpdated::NotifyCode::Color)
    {
        pawn->NotifySpatialRenderStateChanged();
    }

    CheckLightVolumeBackfaceCulling(lit->GetSpatialName());
}

void DeferredRendererService::UpdateSunLightQuad(const std::shared_ptr<SceneGraph::Light>& lit, SceneGraph::LightInfoUpdated::NotifyCode notify)
{
    assert(lit);
    const auto& pawn = FindLightingPawn(lit->GetSpatialName());
    if (!pawn) return;
    if ((notify == LightInfoUpdated::NotifyCode::Color) || (notify == LightInfoUpdated::NotifyCode::Direction))
    {
        pawn->NotifySpatialRenderStateChanged();
        //m_sunLightQuadLightingState.SetSunLight(lit->GetLightDirection(), lit->GetLightColor());
    }
}

void DeferredRendererService::BindGBufferToLightingMesh(const Renderer::MeshPrimitivePtr& mesh)
{
    assert(m_configuration);
    if (!mesh) return;
    if (m_gBuffer.expired()) return;
    if (!m_gBuffer.lock()->GetRenderTargetTexture()) return;

    EffectTextureMap::EffectTextures textures = {
        { m_configuration->GbufferNormalSemantic(), m_gBuffer.lock()->GetRenderTargetTexture(), m_gBuffer.lock()->FindRenderTextureUsageIndex(Graphics::RenderTextureUsage::Normal) },
        { m_configuration->GbufferDiffuseSemantic(), m_gBuffer.lock()->GetRenderTargetTexture(), m_gBuffer.lock()->FindRenderTextureUsageIndex(Graphics::RenderTextureUsage::Albedo) },
        { m_configuration->GbufferSpecularSemantic(), m_gBuffer.lock()->GetRenderTargetTexture(), m_gBuffer.lock()->FindRenderTextureUsageIndex(Graphics::RenderTextureUsage::Specular) },
        { m_configuration->GbufferDepthSemantic(), m_gBuffer.lock()->GetRenderTargetTexture(), m_gBuffer.lock()->FindRenderTextureUsageIndex(Graphics::RenderTextureUsage::Depth) } };
    mesh->ChangeTextureMap({ textures });
}

void DeferredRendererService::BindGBufferToLightingPawn(const std::shared_ptr<LightingPawn>& lighting_pawn)
{
    if (!lighting_pawn) return;
    auto mesh = std::dynamic_pointer_cast<MeshPrimitive, Primitive>(lighting_pawn->GetPrimitive());
    if (mesh) BindGBufferToLightingMesh(mesh);
}

/*void DeferredRendererService::BindGBufferToLightQuad(const std::shared_ptr<LightQuadPawn>& quad)
{
    if (!quad) return;
    auto mesh = std::dynamic_pointer_cast<MeshPrimitive, Primitive>(quad->GetPrimitive());
    if (mesh) BindGBufferToLightingMesh(mesh);
}*/

std::shared_ptr<LightingPawn> DeferredRendererService::FindLightingPawn(const std::string& name)
{
    if (const auto it = m_lightingPawns.find(name); it != m_lightingPawns.end())
    {
        return it->second.lock();
    }
    return nullptr;
}

void DeferredRendererService::CheckLightVolumeBackfaceCulling(const std::string& lit_name)
{
    auto lit_vol = std::dynamic_pointer_cast<LightVolumePawn, LightingPawn>(FindLightingPawn(lit_name));
    if (!lit_vol) return;
    if (m_cameraService.expired()) return;
    auto camera = m_cameraService.lock()->GetPrimaryCamera();
    if (!camera) return;
    CheckLightVolumeBackfaceCulling(lit_vol, camera);
}

void DeferredRendererService::CheckLightVolumeBackfaceCulling(const std::shared_ptr<LightVolumePawn>& lit_vol,
    const std::shared_ptr<SceneGraph::Camera>& cam)
{
    if (!lit_vol) return;
    if (!cam) return;
    const BoundingVolume& bv = lit_vol->GetWorldBound();
    if (bv.IsEmpty()) return;
    if (bv.PointInside(cam->GetLocation()))
    {
        lit_vol->ToggleCameraInside(true);
    }
    else
    {
        lit_vol->ToggleCameraInside(false);
    }
}
