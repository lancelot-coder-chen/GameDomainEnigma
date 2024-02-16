﻿#include "SceneRendererService.h"
#include "GameSceneService.h"
#include "GameCameraEvents.h"
#include "GameCameraService.h"
#include "Renderer/RendererManager.h"
#include "Renderer/RenderTarget.h"
#include "Renderer/Renderer.h"
#include "Renderer/RendererEvents.h"
#include "Frameworks/EventPublisher.h"
#include <cassert>
#include <system_error>

using namespace Enigma::GameCommon;
using namespace Enigma::Frameworks;
using namespace Enigma::Renderer;

DEFINE_RTTI(GameCommon, SceneRendererService, ISystemService);

SceneRendererService::SceneRendererService(ServiceManager* mngr, const std::shared_ptr<GameSceneService>& scene_service,
    const std::shared_ptr<GameCameraService>& camera_service,
    const std::shared_ptr<RendererManager>& renderer_manager, const std::shared_ptr<SceneRendererServiceConfiguration>& config) : ISystemService(mngr)
{
    m_config = config;
    m_sceneService = scene_service;
    m_cameraService = camera_service;
    m_rendererManager = renderer_manager;
    m_needTick = false;
}

SceneRendererService::~SceneRendererService()
{

}

ServiceResult SceneRendererService::onInit()
{
    m_onPrimaryCameraCreated = std::make_shared<EventSubscriber>([this](const IEventPtr& e) { onPrimaryCameraCreated(e); });
    m_onPrimaryTargetCreated = std::make_shared<EventSubscriber>([this](const IEventPtr& e) { onPrimaryTargetCreated(e); });
    EventPublisher::subscribe(typeid(GameCameraCreated), m_onPrimaryCameraCreated);
    EventPublisher::subscribe(typeid(PrimaryRenderTargetCreated), m_onPrimaryTargetCreated);

    return ServiceResult::Complete;
}

ServiceResult SceneRendererService::onTerm()
{
    EventPublisher::unsubscribe(typeid(GameCameraCreated), m_onPrimaryCameraCreated);
    EventPublisher::unsubscribe(typeid(PrimaryRenderTargetCreated), m_onPrimaryTargetCreated);
    m_onPrimaryCameraCreated = nullptr;
    m_onPrimaryTargetCreated = nullptr;

    return ServiceResult::Complete;
}

void SceneRendererService::createSceneRenderSystem(const std::string& renderer_name, const std::string& target_name)
{
    assert(m_config);
    assert(!m_rendererManager.expired());
    error er = m_rendererManager.lock()->CreateRenderer(renderer_name);
    if (er) return;
    er = m_rendererManager.lock()->CreateRenderTarget(target_name, m_config->IsPrimary() ? RenderTarget::PrimaryType::IsPrimary : RenderTarget::PrimaryType::NotPrimary, { Graphics::RenderTextureUsage::Default });
    if (er) return;
    auto renderer = std::dynamic_pointer_cast<Renderer::Renderer, Engine::IRenderer>(m_rendererManager.lock()->GetRenderer(renderer_name));
    renderer->selectRendererTechnique(m_config->PrimaryRendererTechniqueName());
    auto target = m_rendererManager.lock()->GetRenderTarget(target_name);
    std::dynamic_pointer_cast<Renderer::Renderer, Engine::IRenderer>(renderer)->SetRenderTarget(target);
    m_renderer = renderer;
    attachCamera();
}

void SceneRendererService::destroySceneRenderSystem(const std::string& renderer_name, const std::string& target_name)
{
    assert(!m_rendererManager.expired());
    m_rendererManager.lock()->DestroyRenderTarget(target_name);
    m_rendererManager.lock()->DestroyRenderer(renderer_name);
}

void SceneRendererService::prepareGameScene()
{
    if ((!m_renderer.expired()) && (!m_sceneService.expired()) && (m_sceneService.lock()->GetSceneCuller()))
    {
        m_renderer.lock()->PrepareScene(m_sceneService.lock()->GetSceneCuller()->GetVisibleSet());
    }
}

void SceneRendererService::renderGameScene()
{
    if (m_renderer.expired()) return;
    m_renderer.lock()->ClearRenderTarget();
    m_renderer.lock()->BeginScene();
    m_renderer.lock()->DrawScene();
    m_renderer.lock()->EndScene();
}

void SceneRendererService::flip()
{
    if (m_renderer.expired()) return;
    m_renderer.lock()->Flip();
}

void SceneRendererService::attachCamera()
{
    if ((m_renderer.expired()) || (m_cameraService.expired())) return;
    m_renderer.lock()->SetAssociatedCamera(m_cameraService.lock()->primaryCamera());
    auto [w, h] = m_renderer.lock()->GetRenderTarget()->GetDimension();
    if ((w > 0) && (h > 0))
    {
        m_cameraService.lock()->primaryCamera()->changeAspectRatio(static_cast<float>(w) / static_cast<float>(h));
    }
}

void SceneRendererService::onPrimaryCameraCreated(const IEventPtr& e)
{
    attachCamera();
}

void SceneRendererService::onPrimaryTargetCreated(const Frameworks::IEventPtr& e)
{
    attachCamera();
}

