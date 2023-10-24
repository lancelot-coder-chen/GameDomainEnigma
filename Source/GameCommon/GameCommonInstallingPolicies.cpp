﻿#include "GameCommonInstallingPolicies.h"
#include "Frameworks/ServiceManager.h"
#include "GameCameraService.h"
#include "GameSceneService.h"
#include "GameLightService.h"
#include "SceneGraph/SceneGraphRepository.h"
#include "Frameworks/CommandBus.h"
#include "SceneGraph/SceneGraphCommands.h"
#include "AnimatedPawn.h"
#include <cassert>

using namespace Enigma::GameCommon;

error GameCameraInstallingPolicy::Install(Frameworks::ServiceManager* service_manager)
{
    assert(service_manager);
    auto camera_service = std::make_shared<GameCameraService>(service_manager,
        service_manager->GetSystemServiceAs<SceneGraph::SceneGraphRepository>());
    service_manager->RegisterSystemService(camera_service);
    camera_service->CreatePrimaryCamera(m_cameraDto);
    return error();
}

error GameCameraInstallingPolicy::Shutdown(Frameworks::ServiceManager* service_manager)
{
    assert(service_manager);
    service_manager->ShutdownSystemService(GameCameraService::TYPE_RTTI);
    return error();
}

error GameLightInstallingPolicy::Install(Frameworks::ServiceManager* service_manager)
{
    assert(service_manager);
    auto light_service = std::make_shared<GameLightService>(service_manager,
               service_manager->GetSystemServiceAs<SceneGraph::SceneGraphRepository>());
    service_manager->RegisterSystemService(light_service);
    return error();
}

error GameLightInstallingPolicy::Shutdown(Frameworks::ServiceManager* service_manager)
{
    assert(service_manager);
    service_manager->ShutdownSystemService(GameLightService::TYPE_RTTI);
    return error();
}

error GameSceneInstallingPolicy::Install(Frameworks::ServiceManager* service_manager)
{
    assert(service_manager);
    auto scene_graph_repository = service_manager->GetSystemServiceAs<SceneGraph::SceneGraphRepository>();
    auto camera_service = service_manager->GetSystemServiceAs<GameCameraService>();
    auto scene_service = std::make_shared<GameSceneService>(service_manager, scene_graph_repository, camera_service);
    service_manager->RegisterSystemService(scene_service);
    scene_service->CreateRootScene(m_sceneRootName, m_portalManagedName);
    return error();
}

error GameSceneInstallingPolicy::Shutdown(Frameworks::ServiceManager* service_manager)
{
    assert(service_manager);
    service_manager->ShutdownSystemService(GameSceneService::TYPE_RTTI);
    return error();
}

error AnimatedPawnInstallingPolicy::Install(Frameworks::ServiceManager* service_manager)
{
    Frameworks::CommandBus::post(std::make_shared<SceneGraph::RegisterSpatialDtoFactory>(AnimatedPawn::TYPE_RTTI.getName(),
        [](auto o) { return new AnimatedPawn(o); }));
    return error();
}

error AnimatedPawnInstallingPolicy::Shutdown(Frameworks::ServiceManager* service_manager)
{
    Frameworks::CommandBus::post(std::make_shared<SceneGraph::UnRegisterSpatialDtoFactory>(AnimatedPawn::TYPE_RTTI.getName()));
    return error();
}
