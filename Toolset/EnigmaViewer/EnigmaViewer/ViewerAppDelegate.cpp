﻿#include "Animators/AnimatorCommands.h"
#include "Animators/AnimatorInstallingPolicy.h"
#include "Controllers/ControllerEvents.h"
#include "DaeParser.h"
#include "FileStorage/AnimationAssetFileStoreMapper.h"
#include "FileStorage/AnimatorFileStoreMapper.h"
#include "FileStorage/EffectMaterialSourceFileStoreMapper.h"
#include "FileStorage/GeometryDataFileStoreMapper.h"
#include "FileStorage/SceneGraphFileStoreMapper.h"
#include "FileStorage/TextureFileStoreMapper.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/StdMountPath.h"
#include "Frameworks/CommandBus.h"
#include "Frameworks/EventPublisher.h"
#include "GameCommon/AvatarRecipes.h"
#include "GameCommon/GameCommonInstallingPolicies.h"
#include "GameCommon/GameLightCommands.h"
#include "GameCommon/GameSceneCommands.h"
#include "GameCommon/GameSceneEvents.h"
#include "GameCommon/GameSceneService.h"
#include "GameCommon/SceneRendererInstallingPolicy.h"
#include "GameEngine/DeviceCreatingPolicy.h"
#include "GameEngine/EffectMaterialSourceRepositoryInstallingPolicy.h"
#include "GameEngine/EngineInstallingPolicy.h"
#include "GameEngine/TextureRepositoryInstallingPolicy.h"
#include "Gateways/DtoJsonGateway.h"
#include "Geometries/GeometryInstallingPolicy.h"
#include "GraphicAPIDx11/GraphicAPIDx11.h"
#include "InputHandlers/InputHandlerInstallingPolicy.h"
#include "Platforms/MemoryMacro.h"
#include "Platforms/PlatformLayerUtilities.h"
#include "Primitives/PrimitiveRepositoryInstallingPolicy.h"
#include "Renderables/ModelPrimitive.h"
#include "Renderables/ModelPrimitiveAnimator.h"
#include "Renderables/RenderablesInstallingPolicy.h"
#include "Renderer/RendererInstallingPolicy.h"
#include "SceneGraph/SceneGraphAssemblers.h"
#include "SceneGraph/SceneGraphCommands.h"
#include "SceneGraph/SceneGraphInstallingPolicy.h"
#include "ShadowMap/ShadowMapInstallingPolicies.h"
#include "ShadowMap/ShadowMapServiceConfiguration.h"
#include "ShadowMap/SpatialShadowFlags.h"
#include "ViewerAppDelegate.h"
#include "ViewerCommands.h"
#include "ViewerRenderablesFileStoreMapper.h"
#include "FloorReceiverMaker.h"
#include "ViewerTextureFileStoreMapper.h"
#include <memory>

using namespace EnigmaViewer;
using namespace Enigma::Graphics;
using namespace Enigma::Platforms;
using namespace Enigma::FileSystem;
using namespace Enigma::Controllers;
using namespace Enigma::Devices;
using namespace Enigma::Engine;
using namespace Enigma::Renderer;
using namespace Enigma::Animators;
using namespace Enigma::SceneGraph;
using namespace Enigma::GameCommon;
using namespace Enigma::Gateways;
using namespace Enigma::Frameworks;
using namespace Enigma::ShadowMap;
using namespace Enigma::MathLib;
using namespace Enigma::FileStorage;

std::string PrimaryTargetName = "primary_target";
std::string DefaultRendererName = "default_renderer";
std::string SceneRootName = "_SceneRoot_";
std::string PortalManagementName = "_PortalManagement_";
std::string ViewingPawnName = "_ViewingPawn_";

ViewerAppDelegate::ViewerAppDelegate()
{
    m_hasLogFile = false;
    m_graphicMain = nullptr;
    m_hwnd = nullptr;
}

ViewerAppDelegate::~ViewerAppDelegate()
{
}

void ViewerAppDelegate::initialize(IGraphicAPI::APIVersion api_ver, IGraphicAPI::AsyncType useAsyncDevice,
    const std::string& log_filename, HWND hwnd)
{
    m_hwnd = hwnd;
    if (log_filename.length() > 0)
    {
        Logger::InitLoggerFile(log_filename);
        m_hasLogFile = true;
    }

    FileSystem::create();
    initializeMountPaths();

    m_graphicMain = menew GraphicMain();
    m_graphicMain->installFrameworks();

    menew GraphicAPIDx11(useAsyncDevice);

    CoInitializeEx(NULL, COINIT_MULTITHREADED);  // for WIC Texture Loader

    installEngine();
}

void ViewerAppDelegate::finalize()
{
    shutdownEngine();

    std::this_thread::sleep_for(std::chrono::seconds(1)); // 放一點時間給thread 執行 cleanup
    IGraphicAPI::instance()->TerminateGraphicThread(); // 先跳出thread
    delete IGraphicAPI::instance();

    m_graphicMain->shutdownFrameworks();
    SAFE_DELETE(m_graphicMain);

    if (m_hasLogFile)
    {
        Logger::CloseLoggerFile();
    }
    delete FileSystem::instance();

    CoUninitialize();
}

void ViewerAppDelegate::initializeMountPaths()
{
    if (FileSystem::instance())
    {
        auto path = std::filesystem::current_path();
        auto dataPath = path / "Data";
        auto mediaPath = path / "../../../Media/";
        FileSystem::instance()->addMountPath(std::make_shared<StdMountPath>(mediaPath.string(), "APK_PATH"));
        if (!std::filesystem::exists(dataPath))
        {
            std::filesystem::create_directory(dataPath);
        }
        FileSystem::instance()->addMountPath(std::make_shared<StdMountPath>(dataPath.string(), "DataPath"));
    }
}

void ViewerAppDelegate::installEngine()
{
    m_onRenderEngineInstalled = std::make_shared<EventSubscriber>([=](auto e) { this->onRenderEngineInstalled(e); });
    EventPublisher::subscribe(typeid(RenderEngineInstalled), m_onRenderEngineInstalled);
    m_onSceneGraphRootCreated = std::make_shared<EventSubscriber>([=](auto e) { this->onSceneGraphRootCreated(e); });
    EventPublisher::subscribe(typeid(NodalSceneRootCreated), m_onSceneGraphRootCreated);

    m_changeMeshTexture = std::make_shared<CommandSubscriber>([=](auto c) { this->changeMeshTexture(c); });
    CommandBus::subscribe(typeid(ChangeMeshTexture), m_changeMeshTexture);
    m_addAnimationClip = std::make_shared<CommandSubscriber>([=](auto c) { this->addAnimationClip(c); });
    CommandBus::subscribe(typeid(AddAnimationClip), m_addAnimationClip);
    m_deleteAnimationClip = std::make_shared<CommandSubscriber>([=](auto c) { this->deleteAnimationClip(c); });
    CommandBus::subscribe(typeid(DeleteAnimationClip), m_deleteAnimationClip);
    m_playAnimationClip = std::make_shared<CommandSubscriber>([=](auto c) { this->playAnimationClip(c); });
    CommandBus::subscribe(typeid(PlayAnimationClip), m_playAnimationClip);
    m_changeAnimationTimeValue = std::make_shared<CommandSubscriber>([=](auto c) { this->changeAnimationTimeValue(c); });
    CommandBus::subscribe(typeid(ChangeAnimationTimeValue), m_changeAnimationTimeValue);
    m_loadModelPrimitive = std::make_shared<CommandSubscriber>([=](const ICommandPtr& c) { this->loadModelPrimitive(c); });
    CommandBus::subscribe(typeid(LoadModelPrimitive), m_loadModelPrimitive);
    m_removeModelPrimitive = std::make_shared<CommandSubscriber>([=](const ICommandPtr& c) { this->removeModelPrimitive(c); });
    CommandBus::subscribe(typeid(RemoveModelPrimitive), m_removeModelPrimitive);
    m_createAnimatedPawn = std::make_shared<CommandSubscriber>([=](const ICommandPtr& c) { this->createAnimatedPawn(c); });
    CommandBus::subscribe(typeid(CreateAnimatedPawn), m_createAnimatedPawn);
    m_loadAnimatedPawn = std::make_shared<CommandSubscriber>([=](const ICommandPtr& c) { this->loadAnimatedPawn(c); });
    CommandBus::subscribe(typeid(LoadAnimatedPawn), m_loadAnimatedPawn);
    m_removeAnimatedPawn = std::make_shared<CommandSubscriber>([=](const ICommandPtr& c) { this->removeAnimatedPawn(c); });
    CommandBus::subscribe(typeid(RemoveAnimatedPawn), m_removeAnimatedPawn);

    assert(m_graphicMain);

    auto creating_policy = std::make_shared<DeviceCreatingPolicy>(DeviceRequiredBits(), m_hwnd);
    auto engine_policy = std::make_shared<EngineInstallingPolicy>();
    auto render_sys_policy = std::make_shared<RenderSystemInstallingPolicy>();
    m_geometryDataFileStoreMapper = std::make_shared<GeometryDataFileStoreMapper>("geometries.db.txt@APK_PATH", std::make_shared<DtoJsonGateway>());
    auto geometry_policy = std::make_shared<Enigma::Geometries::GeometryInstallingPolicy>(m_geometryDataFileStoreMapper);
    m_primitiveFileStoreMapper = std::make_shared<ViewerRenderablesFileStoreMapper>("primitives.db.txt@APK_PATH", std::make_shared<DtoJsonGateway>());
    auto primitive_policy = std::make_shared<Enigma::Primitives::PrimitiveRepositoryInstallingPolicy>(m_primitiveFileStoreMapper);
    m_animationAssetFileStoreMapper = std::make_shared<AnimationAssetFileStoreMapper>("animation_assets.db.txt@APK_PATH ", std::make_shared<DtoJsonGateway>());
    m_animatorFileStoreMapper = std::make_shared<AnimatorFileStoreMapper>("animators.db.txt@APK_PATH", std::make_shared<DtoJsonGateway>());
    auto animator_policy = std::make_shared<AnimatorInstallingPolicy>(m_animatorFileStoreMapper, m_animationAssetFileStoreMapper);
    m_sceneGraphFileStoreMapper = std::make_shared<ViewerSceneGraphFileStoreMapper>("scene_graph.db.txt@DataPath", "lazy_scene.db.txt@DataPath", std::make_shared<DtoJsonGateway>());
    auto scene_graph_policy = std::make_shared<SceneGraphInstallingPolicy>(m_sceneGraphFileStoreMapper);
    auto effect_material_source_policy = std::make_shared<EffectMaterialSourceRepositoryInstallingPolicy>(std::make_shared<EffectMaterialSourceFileStoreMapper>("effect_materials.db.txt@APK_PATH"));
    m_textureFileStoreMapper = std::make_shared<ViewerTextureFileStoreMapper>("textures.db.txt@APK_PATH", std::make_shared<DtoJsonGateway>());
    auto texture_policy = std::make_shared<TextureRepositoryInstallingPolicy>(m_textureFileStoreMapper);
    auto renderables_policy = std::make_shared<Enigma::Renderables::RenderablesInstallingPolicy>();
    auto input_handler_policy = std::make_shared<Enigma::InputHandlers::InputHandlerInstallingPolicy>();
    auto camera_id = SpatialId("camera", Camera::TYPE_RTTI);
    auto game_camera_policy = std::make_shared<GameCameraInstallingPolicy>(camera_id, CameraAssembler(camera_id).eyePosition(Enigma::MathLib::Vector3(-5.0f, 5.0f, -5.0f)).lookAt(Enigma::MathLib::Vector3(1.0f, -1.0f, 1.0f)).upDirection(Enigma::MathLib::Vector3::UNIT_Y).frustum(Frustum::ProjectionType::Perspective).frustumFov(Enigma::MathLib::Math::PI / 4.0f).frustumFrontBackZ(0.1f, 100.0f).frustumNearPlaneDimension(40.0f, 30.0f).asNative(camera_id.name() + ".cam@DataPath").toCameraDto().toGenericDto());
    auto deferred_config = std::make_shared<DeferredRendererServiceConfiguration>();
    deferred_config->sunLightEffect() = EffectMaterialId("fx/DeferredShadingWithShadowSunLightPass");
    deferred_config->sunLightSpatialFlags() |= SpatialShadowFlags::Spatial_ShadowReceiver;
    auto deferred_renderer_policy = std::make_shared<DeferredRendererInstallingPolicy>(DefaultRendererName, PrimaryTargetName, deferred_config);
    //auto scene_render_config = std::make_shared<SceneRendererServiceConfiguration>();
    //auto scene_renderer_policy = std::make_shared<SceneRendererInstallingPolicy>(DefaultRendererName, PrimaryTargetName, scene_render_config);
    auto game_scene_policy = std::make_shared<GameSceneInstallingPolicy>();
    auto animated_pawn = std::make_shared<AnimatedPawnInstallingPolicy>();
    auto game_light_policy = std::make_shared<GameLightInstallingPolicy>();
    auto shadow_map_config = std::make_shared<ShadowMapServiceConfiguration>();
    auto shadow_map_policy = std::make_shared<ShadowMapInstallingPolicy>("shadowmap_renderer", "shadowmap_target", shadow_map_config);
    m_graphicMain->installRenderEngine({ creating_policy, engine_policy, render_sys_policy, animator_policy, scene_graph_policy,
        input_handler_policy, game_camera_policy, deferred_renderer_policy /*scene_renderer_policy*/, game_scene_policy, animated_pawn, game_light_policy, shadow_map_policy, geometry_policy, primitive_policy,
        effect_material_source_policy, texture_policy, renderables_policy });
    m_inputHandler = input_handler_policy->GetInputHandler();
    m_sceneRenderer = m_graphicMain->getSystemServiceAs<SceneRendererService>();
    m_shadowMapService = m_graphicMain->getSystemServiceAs<ShadowMapService>();

    m_primitiveFileStoreMapper->subscribeHandlers();
    m_sceneGraphFileStoreMapper->subscribeHandlers();
    m_textureFileStoreMapper->subscribeHandlers();
    m_viewingPawnId = SpatialId(ViewingPawnName, AnimatedPawn::TYPE_RTTI);
    m_viewingPawnPresentation.subscribeHandlers();
}

void ViewerAppDelegate::registerMediaMountPaths(const std::string& media_path)
{
}

void ViewerAppDelegate::shutdownEngine()
{
    m_viewingPawnPresentation.removePawn(m_sceneRootId);
    m_viewingPawnPresentation.unsubscribeHandlers();
    m_pawn = nullptr;
    m_sceneRoot = nullptr;

    m_primitiveFileStoreMapper->unsubscribeHandlers();
    m_sceneGraphFileStoreMapper->unsubscribeHandlers();
    m_textureFileStoreMapper->unsubscribeHandlers();

    EventPublisher::unsubscribe(typeid(RenderEngineInstalled), m_onRenderEngineInstalled);
    m_onRenderEngineInstalled = nullptr;
    EventPublisher::unsubscribe(typeid(NodalSceneRootCreated), m_onSceneGraphRootCreated);
    m_onSceneGraphRootCreated = nullptr;

    CommandBus::unsubscribe(typeid(ChangeMeshTexture), m_changeMeshTexture);
    m_changeMeshTexture = nullptr;
    CommandBus::unsubscribe(typeid(AddAnimationClip), m_addAnimationClip);
    m_addAnimationClip = nullptr;
    CommandBus::unsubscribe(typeid(DeleteAnimationClip), m_deleteAnimationClip);
    m_deleteAnimationClip = nullptr;
    CommandBus::unsubscribe(typeid(PlayAnimationClip), m_playAnimationClip);
    m_playAnimationClip = nullptr;
    CommandBus::unsubscribe(typeid(ChangeAnimationTimeValue), m_changeAnimationTimeValue);
    m_changeAnimationTimeValue = nullptr;
    CommandBus::unsubscribe(typeid(LoadModelPrimitive), m_loadModelPrimitive);
    m_loadModelPrimitive = nullptr;
    CommandBus::unsubscribe(typeid(RemoveModelPrimitive), m_removeModelPrimitive);
    m_removeModelPrimitive = nullptr;
    CommandBus::unsubscribe(typeid(CreateAnimatedPawn), m_createAnimatedPawn);
    m_createAnimatedPawn = nullptr;
    CommandBus::unsubscribe(typeid(LoadAnimatedPawn), m_loadAnimatedPawn);
    m_loadAnimatedPawn = nullptr;
    CommandBus::unsubscribe(typeid(RemoveAnimatedPawn), m_removeAnimatedPawn);
    m_removeAnimatedPawn = nullptr;

    m_graphicMain->shutdownRenderEngine();
}

void ViewerAppDelegate::frameUpdate()
{
    if (m_graphicMain) m_graphicMain->frameUpdate();
}

void ViewerAppDelegate::prepareRender()
{
    if (!m_shadowMapService.expired()) m_shadowMapService.lock()->prepareShadowScene();
    if (!m_sceneRenderer.expired()) m_sceneRenderer.lock()->prepareGameScene();
}

void ViewerAppDelegate::renderFrame()
{
    if (!m_shadowMapService.expired()) m_shadowMapService.lock()->renderShadowScene();
    if (!m_sceneRenderer.expired())
    {
        m_sceneRenderer.lock()->renderGameScene();
        m_sceneRenderer.lock()->flip();
    }
}

void ViewerAppDelegate::onTimerElapsed()
{
    if (!m_graphicMain) return;

    frameUpdate();

    prepareRender();
    renderFrame();
}

void ViewerAppDelegate::importDaeFile(const std::string& filename)
{
    DaeParser parser(m_geometryDataFileStoreMapper, m_animationAssetFileStoreMapper, m_animatorFileStoreMapper, m_primitiveFileStoreMapper);
    parser.loadDaeFile(filename);
    refreshModelList();
}

void ViewerAppDelegate::saveAnimatedPawn()
{
    if (!m_viewingPawnPresentation.hasPawn()) return;
    if (!m_sceneGraphFileStoreMapper) return;
    m_sceneGraphFileStoreMapper->putSpatial(m_viewingPawnPresentation.presentingPawnId(), m_viewingPawnPresentation.pawn()->serializeDto());
    refreshPawnList();
}

void ViewerAppDelegate::onRenderEngineInstalled(const Enigma::Frameworks::IEventPtr& e)
{
    m_sceneRootId = SpatialId(SceneRootName, Node::TYPE_RTTI);
    if ((!e) || (e->typeInfo() != typeid(RenderEngineInstalled))) return;
    if (!m_sceneGraphFileStoreMapper->hasSpatial(m_sceneRootId))
    {
        NodeAssembler root_assembler(m_sceneRootId);
        root_assembler.asNative(m_sceneRootId.name() + ".node@DataPath");
        m_sceneGraphFileStoreMapper->putSpatial(m_sceneRootId, root_assembler.toGenericDto());
    }
    CommandBus::post(std::make_shared<CreateNodalSceneRoot>(m_sceneRootId));

    refreshModelList();
    refreshPawnList();
}

void ViewerAppDelegate::onSceneGraphRootCreated(const Enigma::Frameworks::IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<NodalSceneRootCreated, IEvent>(e);
    if (!ev) return;
    m_sceneRoot = ev->root();
    CommandBus::post(std::make_shared<CreateAmbientLight>(m_sceneRootId, SpatialId("amb_lit", Light::TYPE_RTTI), Enigma::MathLib::ColorRGBA(0.2f, 0.2f, 0.2f, 1.0f)));
    CommandBus::post(std::make_shared<CreateSunLight>(m_sceneRootId, SpatialId("sun_lit", Light::TYPE_RTTI), Enigma::MathLib::Vector3(-1.0, -1.0, -1.0), Enigma::MathLib::ColorRGBA(0.6f, 0.6f, 0.6f, 1.0f)));
    auto mx = Enigma::MathLib::Matrix4::MakeTranslateTransform(2.0f, 2.0f, 2.0f);
    CommandBus::post(std::make_shared<CreatePointLight>(m_sceneRootId, mx, SpatialId("point_lit", Light::TYPE_RTTI), Enigma::MathLib::Vector3(2.0f, 2.0f, 2.0f), Enigma::MathLib::ColorRGBA(3.0f, 0.0f, 3.0f, 1.0f), 3.50f));
    FloorReceiverMaker::makeFloorReceiver(m_sceneRoot);
}


void ViewerAppDelegate::changeMeshTexture(const Enigma::Frameworks::ICommandPtr& c)
{
    if (!c) return;
    auto cmd = std::dynamic_pointer_cast<ChangeMeshTexture, ICommand>(c);
    if (!cmd) return;
    auto pawn = m_viewingPawnPresentation.pawn();
    if (!pawn) return;
    TextureMappingDto tex_dto;
    tex_dto.textureId() = cmd->textureId();
    tex_dto.semantic() = "DiffuseMap";
    auto recipe = std::make_shared<ChangeAvatarTexture>(cmd->meshId(), tex_dto);
    pawn->addAvatarRecipe(recipe);
    pawn->bakeAvatarRecipes();
}

void ViewerAppDelegate::addAnimationClip(const Enigma::Frameworks::ICommandPtr& c)
{
    if (!c) return;
    auto cmd = std::dynamic_pointer_cast<AddAnimationClip, ICommand>(c);
    if (!cmd) return;
    if (!m_pawn) return;
    if (auto act_clip = m_pawn->animationClipMap().findAnimationClip(cmd->name()); !act_clip)
    {
        AnimationClipMap::AnimClip act_clip_new(cmd->name(), cmd->clip());
        m_pawn->animationClipMap().insertClip(act_clip_new);
    }
    else
    {
        act_clip.value().get().changeClip(cmd->clip());
    }
}

void ViewerAppDelegate::deleteAnimationClip(const Enigma::Frameworks::ICommandPtr& c)
{
    if (!c) return;
    auto cmd = std::dynamic_pointer_cast<DeleteAnimationClip, ICommand>(c);
    if (!cmd) return;
    if (!m_pawn) return;
    m_pawn->animationClipMap().removeClip(cmd->name());
}

void ViewerAppDelegate::playAnimationClip(const Enigma::Frameworks::ICommandPtr& c)
{
    if (!c) return;
    auto cmd = std::dynamic_pointer_cast<PlayAnimationClip, ICommand>(c);
    if (!cmd) return;
    if (!m_pawn) return;
    m_pawn->playAnimation(cmd->name());
}

void ViewerAppDelegate::changeAnimationTimeValue(const Enigma::Frameworks::ICommandPtr& c)
{
    if (!c) return;
    auto cmd = std::dynamic_pointer_cast<ChangeAnimationTimeValue, ICommand>(c);
    if (!cmd) return;
    if (!m_pawn) return;
    bool isNameChanged = false;
    if ((m_pawn->animationClipMap().findAnimationClip(cmd->oldName()))
        && (cmd->oldName() != cmd->newName()))
    {
        isNameChanged = true;
    }

    if (!isNameChanged)
    {
        if (auto act_clip = m_pawn->animationClipMap().findAnimationClip(cmd->newName()); act_clip)
        {
            act_clip.value().get().changeClip(cmd->clip());
        }
    }
    else
    {
        m_pawn->animationClipMap().removeClip(cmd->oldName());
        Enigma::GameCommon::AnimationClipMap::AnimClip act_clip_new(cmd->newName(), cmd->clip());
        m_pawn->animationClipMap().insertClip(act_clip_new);
    }
}

void ViewerAppDelegate::loadModelPrimitive(const Enigma::Frameworks::ICommandPtr& c)
{
    if (!c) return;
    auto cmd = std::dynamic_pointer_cast<LoadModelPrimitive, ICommand>(c);
    if (!cmd) return;
    loadModelPrimitive(cmd->name());
}

void ViewerAppDelegate::createAnimatedPawn(const Enigma::Frameworks::ICommandPtr& c)
{
    if (!c) return;
    auto cmd = std::dynamic_pointer_cast<CreateAnimatedPawn, ICommand>(c);
    if (!cmd) return;
    m_creatingPawnId = SpatialId(cmd->name(), AnimatedPawn::TYPE_RTTI);
    m_viewingPawnPresentation.presentPawn(m_creatingPawnId, cmd->modelId(), m_sceneRootId);
}

void ViewerAppDelegate::loadAnimatedPawn(const Enigma::Frameworks::ICommandPtr& c)
{
    if (!c) return;
    auto cmd = std::dynamic_pointer_cast<LoadAnimatedPawn, ICommand>(c);
    if (!cmd) return;
    loadAnimatedPawn(cmd->name());
}

void ViewerAppDelegate::removeModelPrimitive(const Enigma::Frameworks::ICommandPtr& c)
{
    if (!c) return;
    auto cmd = std::dynamic_pointer_cast<RemoveModelPrimitive, ICommand>(c);
    if (!cmd) return;
    removeModelPrimitive(cmd->name());
}

void ViewerAppDelegate::removeAnimatedPawn(const Enigma::Frameworks::ICommandPtr& c)
{
    if (!c) return;
    auto cmd = std::dynamic_pointer_cast<RemoveAnimatedPawn, ICommand>(c);
    if (!cmd) return;
    removeAnimatedPawn(cmd->name());
}

void ViewerAppDelegate::refreshModelList()
{
    if (m_primitiveFileStoreMapper)
    {
        CommandBus::post(std::make_shared<RefreshModelPrimitiveList>(m_primitiveFileStoreMapper->modelNames()));
    }
}

void ViewerAppDelegate::loadModelPrimitive(const std::string& model_name)
{
    if (!m_primitiveFileStoreMapper) return;
    auto model_id = m_primitiveFileStoreMapper->modelId(model_name);
    if (!model_id) return;
    m_viewingPawnPresentation.presentPawn(m_viewingPawnId, model_id.value(), m_sceneRootId);
}

void ViewerAppDelegate::removeModelPrimitive(const std::string& model_name)
{
    if (!m_primitiveFileStoreMapper) return;
    auto model_id = m_primitiveFileStoreMapper->modelId(model_name);
    if (!model_id) return;
    m_primitiveFileStoreMapper->removePrimitive(model_id.value());
    refreshModelList();
}

void ViewerAppDelegate::refreshPawnList()
{
    if (m_sceneGraphFileStoreMapper)
    {
        CommandBus::post(std::make_shared<RefreshPawnList>(m_sceneGraphFileStoreMapper->pawnNames()));
    }
}

void ViewerAppDelegate::loadAnimatedPawn(const std::string& pawn_name)
{
    if (!m_sceneGraphFileStoreMapper) return;
    auto pawn_id = m_sceneGraphFileStoreMapper->pawnId(pawn_name);
    if (!pawn_id) return;
    m_viewingPawnPresentation.loadPawn(pawn_id.value(), m_sceneRootId);
}

void ViewerAppDelegate::removeAnimatedPawn(const std::string& pawn_name)
{
    if (!m_sceneGraphFileStoreMapper) return;
    auto pawn_id = m_sceneGraphFileStoreMapper->pawnId(pawn_name);
    if (!pawn_id) return;
    m_sceneGraphFileStoreMapper->removeSpatial(pawn_id.value());
    refreshPawnList();
}
