﻿#include "LevelEditorAppDelegate.h"
#include "Platforms/PlatformLayerUtilities.h"
#include "FileSystem/FileSystem.h"
#include "Platforms/MemoryMacro.h"
#include "GraphicAPIDx11/GraphicAPIDx11.h"
#include "GameEngine/DeviceCreatingPolicy.h"
#include "GameEngine/EngineInstallingPolicy.h"
#include "Renderer/RendererInstallingPolicy.h"
#include "Gateways/JsonFileDtoDeserializer.h"
#include "Gateways/JsonFileEffectProfileDeserializer.h"
#include "GameCommon/SceneRendererInstallingPolicy.h"
#include "Animators/AnimatorInstallingPolicy.h"
#include "SceneGraph/SceneGraphInstallingPolicy.h"
#include "InputHandlers/InputHandlerInstallingPolicy.h"
#include "GameCommon/GameCommonInstallingPolicies.h"
#include "SceneGraph/SceneGraphDtoHelper.h"
#include "WorldMap/WorldMapInstallingPolicy.h"
#include "WorldMap/WorldMapDto.h"
#include "WorldMap/WorldMapCommands.h"
#include "Frameworks/CommandBus.h"
#include "SceneGraph/SceneGraphEvents.h"
#include "Frameworks/EventPublisher.h"
#include "GameCommon/GameSceneEvents.h"
#include "SceneGraph/SceneFlattenTraversal.h"
#include "LevelEditorCommands.h"
#include "WorldMap/WorldMapEvents.h"
#include "WorldMap/WorldMap.h"
#include <memory>

using namespace LevelEditor;
using namespace Enigma::Graphics;
using namespace Enigma::Platforms;
using namespace Enigma::Controllers;
using namespace Enigma::FileSystem;
using namespace Enigma::Devices;
using namespace Enigma::Engine;
using namespace Enigma::Renderer;
using namespace Enigma::Gateways;
using namespace Enigma::GameCommon;
using namespace Enigma::Animators;
using namespace Enigma::SceneGraph;
using namespace Enigma::WorldMap;
using namespace Enigma::Frameworks;

std::string PrimaryTargetName = "primary_target";
std::string DefaultRendererName = "default_renderer";

EditorAppDelegate::EditorAppDelegate()
{
    m_hasLogFile = false;
    m_graphicMain = nullptr;
    m_hwnd = nullptr;
}

EditorAppDelegate::~EditorAppDelegate()
{
}

void EditorAppDelegate::Initialize(IGraphicAPI::APIVersion api_ver, IGraphicAPI::AsyncType useAsyncDevice,
                                   const std::string& log_filename, HWND hwnd)
{
    m_hwnd = hwnd;
    if (log_filename.length() > 0)
    {
        Logger::InitLoggerFile(log_filename);
        m_hasLogFile = true;
    }

    FileSystem::Create();
    InitializeMountPaths();

    m_graphicMain = menew GraphicMain();
    m_graphicMain->InstallFrameworks();

    menew GraphicAPIDx11(useAsyncDevice);

    CoInitializeEx(NULL, COINIT_MULTITHREADED);  // for WIC Texture Loader

    InstallEngine();
}

void EditorAppDelegate::Finalize()
{
    ShutdownEngine();

    std::this_thread::sleep_for(std::chrono::seconds(1)); // 放一點時間給thread 執行 cleanup
    IGraphicAPI::Instance()->TerminateGraphicThread(); // 先跳出thread
    delete IGraphicAPI::Instance();

    m_graphicMain->ShutdownFrameworks();
    SAFE_DELETE(m_graphicMain);

    if (m_hasLogFile)
    {
        Logger::CloseLoggerFile();
    }
    delete FileSystem::FileSystem::Instance();

    CoUninitialize();
}

void EditorAppDelegate::InitializeMountPaths()
{
}

void EditorAppDelegate::RegisterMediaMountPaths(const std::string& media_path)
{
}

void EditorAppDelegate::InstallEngine()
{
    m_onSceneGraphChanged = std::make_shared<EventSubscriber>([=](auto e) { OnSceneGraphChanged(e); });
    EventPublisher::Subscribe(typeid(SceneGraphChanged), m_onSceneGraphChanged);
    m_onSceneRootCreated = std::make_shared<EventSubscriber>([=](auto e) { OnSceneRootCreated(e); });
    EventPublisher::Subscribe(typeid(SceneRootCreated), m_onSceneRootCreated);
    m_onWorldMapCreated = std::make_shared<EventSubscriber>([=](auto e) { OnWorldMapCreated(e); });
    EventPublisher::Subscribe(typeid(WorldMapCreated), m_onWorldMapCreated);

    assert(m_graphicMain);

    auto creating_policy = std::make_shared<DeviceCreatingPolicy>(DeviceRequiredBits(), m_hwnd);
    auto engine_policy = std::make_shared<EngineInstallingPolicy>(std::make_shared<JsonFileEffectProfileDeserializer>());
    auto render_sys_policy = std::make_shared<RenderSystemInstallingPolicy>();
    auto scene_render_config = std::make_shared<SceneRendererServiceConfiguration>();
    auto scene_renderer_policy = std::make_shared<SceneRendererInstallingPolicy>(DefaultRendererName, PrimaryTargetName, scene_render_config);
    auto animator_policy = std::make_shared<AnimatorInstallingPolicy>();
    auto scene_graph_policy = std::make_shared<SceneGraphInstallingPolicy>(
        std::make_shared<JsonFileDtoDeserializer>());
    auto game_scene_policy = std::make_shared<GameSceneInstallingPolicy>("_scene_root_", "_portal_management_");
    auto input_handler_policy = std::make_shared<Enigma::InputHandlers::InputHandlerInstallingPolicy>();
    auto game_camera_policy = std::make_shared<GameCameraInstallingPolicy>(
        CameraDtoHelper("camera").EyePosition(Enigma::MathLib::Vector3(-5.0f, 5.0f, -5.0f)).LookAt(Enigma::MathLib::Vector3(1.0f, -1.0f, 1.0f)).UpDirection(Enigma::MathLib::Vector3::UNIT_Y)
        .Frustum("frustum", Frustum::ProjectionType::Perspective).FrustumFov(Enigma::MathLib::Math::PI / 4.0f).FrustumFrontBackZ(0.1f, 100.0f)
        .FrustumNearPlaneDimension(40.0f, 30.0f).ToCameraDto());
    auto world_map_policy = std::make_shared<WorldMapInstallingPolicy>();
    m_graphicMain->InstallRenderEngine({ creating_policy, engine_policy, render_sys_policy, scene_renderer_policy, animator_policy, scene_graph_policy, input_handler_policy, game_camera_policy, world_map_policy, game_scene_policy });
    m_inputHandler = input_handler_policy->GetInputHandler();
    m_sceneRenderer = m_graphicMain->GetSystemServiceAs<SceneRendererService>();
}

void EditorAppDelegate::ShutdownEngine()
{
    EventPublisher::Unsubscribe(typeid(SceneGraphChanged), m_onSceneGraphChanged);
    m_onSceneGraphChanged = nullptr;
    EventPublisher::Unsubscribe(typeid(SceneRootCreated), m_onSceneRootCreated);
    m_onSceneRootCreated = nullptr;
    EventPublisher::Unsubscribe(typeid(WorldMapCreated), m_onWorldMapCreated);
    m_onWorldMapCreated = nullptr;

    assert(m_graphicMain);
    m_graphicMain->ShutdownRenderEngine();
}

void EditorAppDelegate::FrameUpdate()
{
    if (m_graphicMain) m_graphicMain->FrameUpdate();
}

void EditorAppDelegate::PrepareRender()
{
    if (!m_sceneRenderer.expired()) m_sceneRenderer.lock()->PrepareGameScene();
}

void EditorAppDelegate::RenderFrame()
{
    if (!m_sceneRenderer.expired())
    {
        m_sceneRenderer.lock()->RenderGameScene();
        m_sceneRenderer.lock()->Flip();
    }
}

void EditorAppDelegate::OnTimerElapsed()
{
    if (!m_graphicMain) return;

    FrameUpdate();

    PrepareRender();
    RenderFrame();
}

void EditorAppDelegate::CreateWorldMap(const std::string& map_name)
{
    WorldMapDto world_map_dto;
    world_map_dto.Name() = map_name;
    world_map_dto.IsTopLevel() = true;
    CommandBus::Post(std::make_shared<CreateEmptyWorldMap>(world_map_dto.ToGenericDto()));
}

void EditorAppDelegate::OnSceneGraphChanged(const IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<SceneGraphChanged, IEvent>(e);
    if (!ev) return;
    if (m_sceneRoot.expired()) return;
    SceneFlattenTraversal traversal;
    m_sceneRoot.lock()->VisitBy(&traversal);
    CommandBus::Post(std::make_shared<RefreshSceneGraph>(traversal.GetSpatials()));
}

void EditorAppDelegate::OnSceneRootCreated(const Enigma::Frameworks::IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<SceneRootCreated, IEvent>(e);
    if (!ev) return;
    CommandBus::Post(std::make_shared<OutputMessage>("scene root created : " + ev->GetSceneRoot()->GetSpatialName()));
    m_sceneRoot = ev->GetSceneRoot();
    SceneFlattenTraversal traversal;
    m_sceneRoot.lock()->VisitBy(&traversal);
    CommandBus::Post(std::make_shared<RefreshSceneGraph>(traversal.GetSpatials()));
}

void EditorAppDelegate::OnWorldMapCreated(const Enigma::Frameworks::IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<WorldMapCreated, IEvent>(e);
    if (!ev) return;
    CommandBus::Post(std::make_shared<OutputMessage>("world map created : " + ev->GetName()));
    if (!m_sceneRoot.expired())
    {
        m_sceneRoot.lock()->AttachChild(ev->GetWorld(), Enigma::MathLib::Matrix4::IDENTITY);
    }
}
