﻿#include "ViewerAppDelegate.h"
#include "Platforms/MemoryAllocMacro.h"
#include "Platforms/MemoryMacro.h"
#include "Platforms/PlatformLayerUtilities.h"
#include "FileSystem/FileSystem.h"
#include "GraphicAPIDx11/GraphicAPIDx11.h"
#include "GameEngine/DeviceCreatingPolicy.h"
#include "GameEngine/EngineInstallingPolicy.h"
#include "Renderer/RendererInstallingPolicy.h"
#include "Animators/AnimatorInstallingPolicy.h"
#include "SceneGraph/SceneGraphInstallingPolicy.h"
#include "InputHandlers/InputHandlerInstallingPolicy.h"
#include "GameCommon/GameCommonInstallingPolicies.h"
#include "GameCommon/SceneRendererInstallingPolicy.h"
#include "Gateways/JsonFileDtoDeserializer.h"
#include "Gateways/JsonFileEffectProfileDeserializer.h"
#include "CameraDtoMaker.h"
#include "FileSystem/StdMountPath.h"
#include "Frameworks/CommandBus.h"
#include "Frameworks/EventPublisher.h"
#include "SceneGraph/SceneGraphCommands.h"
#include "SceneGraph/SceneGraphEvents.h"
#include "SceneGraph/Pawn.h"
#include "GameCommon/GameSceneService.h"
#include "ViewerCommands.h"
#include "Animators/AnimatorCommands.h"
#include "Animators/ModelPrimitiveAnimator.h"
#include "GameCommon/AvatarRecipes.h"
#include <memory>
#include <Gateways/DtoJsonGateway.h>
#include "GameCommon/GameLightCommands.h"

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

std::string PrimaryTargetName = "primary_target";
std::string DefaultRendererName = "default_renderer";
std::string SceneRootName = "_SceneRoot_";
std::string PortalManagementName = "_PortalManagement_";

ViewerAppDelegate::ViewerAppDelegate()
{
    m_hasLogFile = false;
    m_graphicMain = nullptr;
    m_hwnd = nullptr;
}

ViewerAppDelegate::~ViewerAppDelegate()
{
}

void ViewerAppDelegate::Initialize(IGraphicAPI::APIVersion api_ver, IGraphicAPI::AsyncType useAsyncDevice,
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

void ViewerAppDelegate::Finalize()
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
    delete FileSystem::Instance();

    CoUninitialize();
}

void ViewerAppDelegate::InitializeMountPaths()
{
    if (FileSystem::Instance())
    {
        auto path = std::filesystem::current_path();
        auto mediaPath = path / "../../../Media/";
        FileSystem::Instance()->AddMountPath(std::make_shared<StdMountPath>(mediaPath.string(), "APK_PATH"));
        FileSystem::Instance()->AddMountPath(std::make_shared<StdMountPath>(path.string(), "DataPath"));
    }
}

void ViewerAppDelegate::InstallEngine()
{
    m_onPawnPrimitiveBuilt = std::make_shared<EventSubscriber>([=](auto e) { this->OnPawnPrimitiveBuilt(e); });
    EventPublisher::Subscribe(typeid(PawnPrimitiveBuilt), m_onPawnPrimitiveBuilt);

    m_doChangingMeshTexture = std::make_shared<CommandSubscriber>([=](auto c) { this->DoChangingMeshTexture(c); });
    CommandBus::Subscribe(typeid(ChangeMeshTexture), m_doChangingMeshTexture);
    m_doAddingAnimationClip = std::make_shared<CommandSubscriber>([=](auto c) { this->DoAddingAnimationClip(c); });
    CommandBus::Subscribe(typeid(AddAnimationClip), m_doAddingAnimationClip);
    m_doDeletingAnimationClip = std::make_shared<CommandSubscriber>([=](auto c) { this->DoDeletingAnimationClip(c); });
    CommandBus::Subscribe(typeid(DeleteAnimationClip), m_doDeletingAnimationClip);
    m_doPlayingAnimationClip = std::make_shared<CommandSubscriber>([=](auto c) { this->DoPlayingAnimationClip(c); });
    CommandBus::Subscribe(typeid(PlayAnimationClip), m_doPlayingAnimationClip);
    m_doChangingAnimationTimeValue = std::make_shared<CommandSubscriber>([=](auto c) { this->DoChangingAnimationTimeValue(c); });
    CommandBus::Subscribe(typeid(ChangeAnimationTimeValue), m_doChangingAnimationTimeValue);

    assert(m_graphicMain);

    auto creating_policy = std::make_shared<DeviceCreatingPolicy>(DeviceRequiredBits(), m_hwnd);
    auto engine_policy = std::make_shared<EngineInstallingPolicy>(std::make_shared<JsonFileEffectProfileDeserializer>());
    auto render_sys_policy = std::make_shared<RenderSystemInstallingPolicy>();
    auto animator_policy = std::make_shared<AnimatorInstallingPolicy>();
    auto scene_graph_policy = std::make_shared<SceneGraphInstallingPolicy>(
        std::make_shared<JsonFileDtoDeserializer>());
    auto input_handler_policy = std::make_shared<Enigma::InputHandlers::InputHandlerInstallingPolicy>();
    auto game_camera_policy = std::make_shared<GameCameraInstallingPolicy>(CameraDtoMaker::MakeCameraDto());
    auto scene_renderer_policy = std::make_shared<SceneRendererInstallingPolicy>(DefaultRendererName, PrimaryTargetName, true);
    auto game_scene_policy = std::make_shared<GameSceneInstallingPolicy>(SceneRootName, PortalManagementName);
    auto animated_pawn = std::make_shared<AnimatedPawnInstallingPolicy>();
    auto game_light_policy = std::make_shared<GameLightInstallingPolicy>();
    m_graphicMain->InstallRenderEngine({ creating_policy, engine_policy, render_sys_policy, animator_policy, scene_graph_policy,
        input_handler_policy, game_camera_policy, scene_renderer_policy, game_scene_policy, animated_pawn, game_light_policy });
    m_inputHandler = input_handler_policy->GetInputHandler();
    m_sceneRenderer = m_graphicMain->GetSystemServiceAs<SceneRendererService>();
}

void ViewerAppDelegate::RegisterMediaMountPaths(const std::string& media_path)
{
}

void ViewerAppDelegate::ShutdownEngine()
{
    m_pawn = nullptr;

    EventPublisher::Unsubscribe(typeid(PawnPrimitiveBuilt), m_onPawnPrimitiveBuilt);
    m_onPawnPrimitiveBuilt = nullptr;

    CommandBus::Unsubscribe(typeid(ChangeMeshTexture), m_doChangingMeshTexture);
    m_doChangingMeshTexture = nullptr;
    CommandBus::Unsubscribe(typeid(AddAnimationClip), m_doAddingAnimationClip);
    m_doAddingAnimationClip = nullptr;
    CommandBus::Unsubscribe(typeid(DeleteAnimationClip), m_doDeletingAnimationClip);
    m_doDeletingAnimationClip = nullptr;
    CommandBus::Unsubscribe(typeid(PlayAnimationClip), m_doPlayingAnimationClip);
    m_doPlayingAnimationClip = nullptr;
    CommandBus::Unsubscribe(typeid(ChangeAnimationTimeValue), m_doChangingAnimationTimeValue);
    m_doChangingAnimationTimeValue = nullptr;

    m_graphicMain->ShutdownRenderEngine();
}

void ViewerAppDelegate::FrameUpdate()
{
    if (m_graphicMain) m_graphicMain->FrameUpdate();
}

void ViewerAppDelegate::PrepareRender()
{
    if (!m_sceneRenderer.expired()) m_sceneRenderer.lock()->PrepareGameScene();
}

void ViewerAppDelegate::RenderFrame()
{
    if (!m_sceneRenderer.expired())
    {
        m_sceneRenderer.lock()->RenderGameScene();
        m_sceneRenderer.lock()->Flip();
    }
}

void ViewerAppDelegate::OnTimerElapsed()
{
    if (!m_graphicMain) return;

    FrameUpdate();

    PrepareRender();
    RenderFrame();
}

void ViewerAppDelegate::LoadPawn(const AnimatedPawnDto& pawn_dto)
{
    CommandBus::Post(std::make_shared<OutputMessage>("Load Pawn " + pawn_dto.Name()));
    CommandBus::Post(std::make_shared<BuildSceneGraph>("viewing_pawn", std::vector{ pawn_dto.ToGenericDto() }));
}

void ViewerAppDelegate::SavePawnFile(const std::filesystem::path& filepath)
{
    if (!m_pawn) return;
    auto pawn_dto = m_pawn->SerializeDto();
    std::string json = DtoJsonGateway::Serialize(std::vector<GenericDto>{pawn_dto});
    IFilePtr iFile = FileSystem::Instance()->OpenFile(filepath.generic_string(), "w+b");
    iFile->Write(0, convert_to_buffer(json));
    FileSystem::Instance()->CloseFile(iFile);
}

void ViewerAppDelegate::LoadPawnFile(const std::filesystem::path& filepath)
{
    if (m_pawn)
    {
        m_pawn->DetachFromParent();
        m_pawn = nullptr;
    }
    IFilePtr iFile = FileSystem::Instance()->OpenFile(filepath.generic_string(), "rb");
    size_t file_size = iFile->Size();

    auto read_buf = iFile->Read(0, file_size);
    FileSystem::Instance()->CloseFile(iFile);
    auto dtos = DtoJsonGateway::Deserialize(convert_to_string(read_buf.value(), file_size));
    CommandBus::Post(std::make_shared<BuildSceneGraph>("viewing_pawn", dtos));
}

void ViewerAppDelegate::OnPawnPrimitiveBuilt(const IEventPtr& e)
{
    if (!e) return;
    auto ev = std::dynamic_pointer_cast<PawnPrimitiveBuilt, IEvent>(e);
    if (!ev) return;
    m_pawn = std::dynamic_pointer_cast<AnimatedPawn, Pawn>(ev->GetPawn());
    if (!m_pawn) return;
    m_pawn->BakeAvatarRecipes();
    CommandBus::Post(std::make_shared<RefreshAnimationClipList>(m_pawn->TheAnimationClipMap()));
    auto scene_service = m_graphicMain->GetSystemServiceAs<GameSceneService>();
    if (!scene_service) return;
    Enigma::MathLib::Matrix4 mx = Enigma::MathLib::Matrix4::MakeRotationXTransform(-Enigma::MathLib::Math::HALF_PI);
    error er = scene_service->GetSceneRoot()->AttachChild(m_pawn, mx);
    auto prim = m_pawn->GetPrimitive();
    if (prim)
    {
        auto model = std::dynamic_pointer_cast<ModelPrimitive, Primitive>(prim);
        if (!model) return;
        CommandBus::Post(std::make_shared<RefreshModelNodeTree>(model));
        if (auto ani = model->GetAnimator())
        {
            ani->Reset();
            CommandBus::Post(std::make_shared<AddListeningAnimator>(ani));
            if (auto model_ani = std::dynamic_pointer_cast<ModelPrimitiveAnimator, Animator>(ani))
            {
                model_ani->PlayAnimation(AnimationClip{ 0.0f, 20.0f, AnimationClip::WarpMode::Loop, 0 });
            }
        }
    }
    CommandBus::Post(std::make_shared<CreateAmbientLight>("amb_lit", Enigma::MathLib::ColorRGBA(0.8, 0.2, 0.2, 1.0)));
}

void ViewerAppDelegate::DoChangingMeshTexture(const Enigma::Frameworks::ICommandPtr& c)
{
    if (!c) return;
    auto cmd = std::dynamic_pointer_cast<ChangeMeshTexture, ICommand>(c);
    if (!cmd) return;
    if (!m_pawn) return;
    TextureMappingDto tex_dto;
    tex_dto.Filename() = "image/" + cmd->GetTextureFilename();
    tex_dto.TextureName() = cmd->GetTextureFilename();
    tex_dto.PathId() = "APK_PATH";
    tex_dto.Semantic() = "DiffuseMap";
    auto recipe = std::make_shared<ChangeAvatarTexture>(cmd->GetMeshName(), tex_dto);
    m_pawn->AddAvatarRecipe(recipe);
    m_pawn->BakeAvatarRecipes();
}

void ViewerAppDelegate::DoAddingAnimationClip(const Enigma::Frameworks::ICommandPtr& c)
{
    if (!c) return;
    auto cmd = std::dynamic_pointer_cast<AddAnimationClip, ICommand>(c);
    if (!cmd) return;
    if (!m_pawn) return;
    if (auto act_clip = m_pawn->TheAnimationClipMap().FindAnimationClip(cmd->GetName()); !act_clip)
    {
        AnimationClipMap::AnimClip act_clip_new(cmd->GetName(), cmd->GetClip());
        m_pawn->TheAnimationClipMap().InsertClip(act_clip_new);
    }
    else
    {
        act_clip.value().get().ChangeClip(cmd->GetClip());
    }
}

void ViewerAppDelegate::DoDeletingAnimationClip(const Enigma::Frameworks::ICommandPtr& c)
{
    if (!c) return;
    auto cmd = std::dynamic_pointer_cast<DeleteAnimationClip, ICommand>(c);
    if (!cmd) return;
    if (!m_pawn) return;
    m_pawn->TheAnimationClipMap().RemoveClip(cmd->GetName());
}

void ViewerAppDelegate::DoPlayingAnimationClip(const Enigma::Frameworks::ICommandPtr& c)
{
    if (!c) return;
    auto cmd = std::dynamic_pointer_cast<PlayAnimationClip, ICommand>(c);
    if (!cmd) return;
    if (!m_pawn) return;
    m_pawn->PlayAnimation(cmd->GetName());
}

void ViewerAppDelegate::DoChangingAnimationTimeValue(const Enigma::Frameworks::ICommandPtr& c)
{
    if (!c) return;
    auto cmd = std::dynamic_pointer_cast<ChangeAnimationTimeValue, ICommand>(c);
    if (!cmd) return;
    if (!m_pawn) return;
    bool isNameChanged = false;
    if ((m_pawn->TheAnimationClipMap().FindAnimationClip(cmd->GetOldName()))
        && (cmd->GetOldName() != cmd->GetNewName()))
    {
        isNameChanged = true;
    }

    if (!isNameChanged)
    {
        if (auto act_clip = m_pawn->TheAnimationClipMap().FindAnimationClip(cmd->GetNewName()); act_clip)
        {
            act_clip.value().get().ChangeClip(cmd->GetClip());
        }
    }
    else
    {
        m_pawn->TheAnimationClipMap().RemoveClip(cmd->GetOldName());
        Enigma::GameCommon::AnimationClipMap::AnimClip act_clip_new(cmd->GetNewName(), cmd->GetClip());
        m_pawn->TheAnimationClipMap().InsertClip(act_clip_new);
    }
}
