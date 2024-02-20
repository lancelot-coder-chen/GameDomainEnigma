﻿#include "RendererManager.h"
#include "RendererErrors.h"
#include "RendererCommands.h"
#include "RendererEvents.h"
#include "Renderer.h"
#include "Frameworks/Rtti.h"
#include "Frameworks/SystemService.h"
#include "Frameworks/EventPublisher.h"
#include "Frameworks/CommandBus.h"
#include "Platforms/MemoryMacro.h"
#include <cassert>

#include "Platforms/PlatformLayer.h"

using namespace Enigma::Renderer;
using namespace Enigma::Engine;

DEFINE_RTTI(Renderer, RendererManager, ISystemService);

RendererManager::CustomRendererFactoryTable RendererManager::m_customRendererFactoryTable;

RendererManager::RendererManager(Frameworks::ServiceManager* srv_mngr) : ISystemService(srv_mngr)
{
    m_needTick = false;
    m_accumulateRendererStamp = 0;
}

RendererManager::~RendererManager()
{
    assert(m_renderers.empty());
}

Enigma::Frameworks::ServiceResult RendererManager::onInit()
{
    m_accumulateRendererStamp = 0;

    m_doCreatingRenderer =
        std::make_shared<Frameworks::CommandSubscriber>([=](auto c) { this->DoCreatingRenderer(c); });
    Frameworks::CommandBus::subscribe(typeid(Enigma::Renderer::CreateRenderer), m_doCreatingRenderer);
    m_doDestroyingRenderer =
        std::make_shared<Frameworks::CommandSubscriber>([=](auto c) { this->DoDestroyingRenderer(c); });
    Frameworks::CommandBus::subscribe(typeid(Enigma::Renderer::DestroyRenderer), m_doDestroyingRenderer);

    m_doCreatingRenderTarget =
        std::make_shared<Frameworks::CommandSubscriber>([=](auto c) { this->DoCreatingRenderTarget(c); });
    Frameworks::CommandBus::subscribe(typeid(Enigma::Renderer::CreateRenderTarget), m_doCreatingRenderTarget);
    m_doDestroyingRenderTarget =
        std::make_shared<Frameworks::CommandSubscriber>([=](auto c) { this->DoDestroyingRenderTarget(c); });
    Frameworks::CommandBus::subscribe(typeid(Enigma::Renderer::DestroyRenderTarget), m_doDestroyingRenderTarget);

    m_doResizingPrimaryTarget =
        std::make_shared<Frameworks::CommandSubscriber>([=](auto c) { this->DoResizingPrimaryTarget(c); });
    Frameworks::CommandBus::subscribe(typeid(ResizePrimaryRenderTarget), m_doResizingPrimaryTarget);

    m_doChangingViewPort =
        std::make_shared<Frameworks::CommandSubscriber>([=](auto c) { this->DoChangingViewPort(c); });
    Frameworks::CommandBus::subscribe(typeid(ChangeTargetViewPort), m_doChangingViewPort);
    m_doChangingClearingProperty =
        std::make_shared<Frameworks::CommandSubscriber>([=](auto c) { this->DoChangingClearingProperty(c); });
    Frameworks::CommandBus::subscribe(typeid(ChangeTargetClearingProperty), m_doChangingClearingProperty);

    return Frameworks::ServiceResult::Complete;
}

Enigma::Frameworks::ServiceResult RendererManager::onTerm()
{
    Frameworks::CommandBus::unsubscribe(typeid(Enigma::Renderer::CreateRenderer), m_doCreatingRenderer);
    m_doCreatingRenderer = nullptr;
    Frameworks::CommandBus::unsubscribe(typeid(Enigma::Renderer::DestroyRenderer), m_doDestroyingRenderer);
    m_doDestroyingRenderer = nullptr;

    Frameworks::CommandBus::unsubscribe(typeid(Enigma::Renderer::CreateRenderTarget), m_doCreatingRenderTarget);
    m_doCreatingRenderTarget = nullptr;
    Frameworks::CommandBus::unsubscribe(typeid(Enigma::Renderer::DestroyRenderTarget), m_doDestroyingRenderTarget);
    m_doDestroyingRenderTarget = nullptr;

    Frameworks::CommandBus::unsubscribe(typeid(ResizePrimaryRenderTarget), m_doResizingPrimaryTarget);
    m_doResizingPrimaryTarget = nullptr;

    Frameworks::CommandBus::unsubscribe(typeid(ChangeTargetViewPort), m_doChangingViewPort);
    m_doChangingViewPort = nullptr;
    Frameworks::CommandBus::unsubscribe(typeid(ChangeTargetClearingProperty), m_doChangingClearingProperty);
    m_doChangingClearingProperty = nullptr;

    ClearAllRenderer();
    ClearAllRenderTarget();
    m_accumulateRendererStamp = 0;
    return Frameworks::ServiceResult::Complete;
}
void RendererManager::RegisterCustomRendererFactory(const std::string& type_name, const CustomRendererFactoryFunc& fn)
{
    m_customRendererFactoryTable.emplace(type_name, fn);
}

error RendererManager::createRenderer(const std::string& name)
{
    IRendererPtr render = getRenderer(name);
    if (render)
    {
        // render already exist
        return ErrorCode::rendererAlreadyExist;
    }
    render = std::make_shared<Renderer>(name);
    m_renderers.emplace(name, render);
    assert(m_renderers.size() <= 32);

    unsigned int stamp = 1;
    while (m_accumulateRendererStamp & stamp)
    {
        stamp = stamp << 1;
    }
    m_accumulateRendererStamp |= stamp;
    render->SetStampBitMask(stamp);

    Frameworks::EventPublisher::post(std::make_shared<RendererCreated>(render->getName(), render));

    return ErrorCode::ok;
}

error RendererManager::CreateCustomRenderer(const std::string& type_name, const std::string& name)
{
    IRendererPtr render = getRenderer(name);
    if (render)
    {
        // render already exist
        return ErrorCode::rendererAlreadyExist;
    }
    // new renderer by factory
    CustomRendererFactoryTable::iterator iter = m_customRendererFactoryTable.find(type_name);
    if (iter == m_customRendererFactoryTable.end()) return ErrorCode::rendererFactoryFail;
    render = (iter->second)(name);
    m_renderers.emplace(name, render);
    assert(m_renderers.size() <= 32);

    unsigned int stamp = 1;
    while (m_accumulateRendererStamp & stamp)
    {
        stamp = stamp << 1;
    }
    m_accumulateRendererStamp |= stamp;
    render->SetStampBitMask(stamp);

    Frameworks::EventPublisher::post(std::make_shared<RendererCreated>(render->getName(), render));

    return ErrorCode::ok;
}

error RendererManager::InsertRenderer(const std::string& name, const Engine::IRendererPtr& renderer)
{
    IRendererPtr render = getRenderer(name);
    if (render)
    {
        // render already exist
        return ErrorCode::rendererAlreadyExist;
    }
    m_renderers.emplace(name, renderer);
    assert(m_renderers.size() <= 32);
    unsigned int stamp = 1;
    while (m_accumulateRendererStamp & stamp)
    {
        stamp = stamp << 1;
    }
    m_accumulateRendererStamp |= stamp;
    renderer->SetStampBitMask(stamp);
    Frameworks::EventPublisher::post(std::make_shared<RendererCreated>(renderer->getName(), renderer));
    return ErrorCode::ok;
}

error RendererManager::destroyRenderer(const std::string& name)
{
    const IRendererPtr render = getRenderer(name);
    if (!render) return ErrorCode::rendererNotExist;

    const unsigned int stamp = render->GetStampBitMask();
    m_accumulateRendererStamp &= (~stamp);
    m_renderers.erase(name);

    Frameworks::EventPublisher::post(std::make_shared<RendererDestroyed>(name));

    return ErrorCode::ok;
}

IRendererPtr RendererManager::getRenderer(const std::string& name) const
{
    const auto it = m_renderers.find(name);
    if (it == m_renderers.end()) return nullptr;
    return it->second;
}

error RendererManager::createRenderTarget(const std::string& name, RenderTarget::PrimaryType primary, const std::vector<Graphics::RenderTextureUsage>& usages)
{
    if (auto target_check = getRenderTarget(name))
    {
        // render already exist
        return ErrorCode::renderTargetAlreadyExisted;
    }
    RenderTargetPtr target = RenderTargetPtr{ menew RenderTarget(name, primary, usages) };
    m_renderTargets.emplace(name, target);

    if (primary == RenderTarget::PrimaryType::IsPrimary)
    {
        m_primaryRenderTargetName = name;
    }

    return ErrorCode::ok;
}

error RendererManager::destroyRenderTarget(const std::string& name)
{
    const auto target = getRenderTarget(name);
    if (!target) return ErrorCode::renderTargetNotExist;
    m_renderTargets.erase(name);

    Frameworks::EventPublisher::post(std::make_shared<RenderTargetDestroyed>(name));

    return ErrorCode::ok;
}

RenderTargetPtr RendererManager::getRenderTarget(const std::string& name) const
{
    RenderTargetMap::const_iterator iter = m_renderTargets.find(name);
    if (iter == m_renderTargets.end()) return nullptr;
    return iter->second;
}

RenderTargetPtr RendererManager::GetPrimaryRenderTarget() const
{
    return getRenderTarget(m_primaryRenderTargetName);
}

void RendererManager::DoResizingPrimaryTarget(const Frameworks::ICommandPtr& c) const
{
    if (!c) return;
    const auto cmd = std::dynamic_pointer_cast<ResizePrimaryRenderTarget, Frameworks::ICommand>(c);
    if (!cmd) return;
    const auto target = GetPrimaryRenderTarget();
    if (!target) return;
    target->Resize(cmd->GetDimension());
}

void RendererManager::ClearAllRenderer()
{
    if (m_renderers.size() == 0) return;
    m_renderers.clear();

    m_accumulateRendererStamp = 0;
}

void RendererManager::ClearAllRenderTarget()
{
    if (m_renderTargets.size() == 0) return;
    m_renderTargets.clear();
}

void RendererManager::DoCreatingRenderer(const Frameworks::ICommandPtr& c)
{
    if (!c) return;
    const auto cmd = std::dynamic_pointer_cast<Enigma::Renderer::CreateRenderer, Frameworks::ICommand>(c);
    if (!cmd) return;
    createRenderer(cmd->GetRendererName());
}

void RendererManager::DoDestroyingRenderer(const Frameworks::ICommandPtr& c)
{
    if (!c) return;
    const auto cmd = std::dynamic_pointer_cast<Enigma::Renderer::DestroyRenderer, Frameworks::ICommand>(c);
    if (!cmd) return;
    destroyRenderer(cmd->GetRendererName());
}

void RendererManager::DoCreatingRenderTarget(const Frameworks::ICommandPtr& c)
{
    if (!c) return;
    const auto cmd = std::dynamic_pointer_cast<Enigma::Renderer::CreateRenderTarget, Frameworks::ICommand>(c);
    if (!cmd) return;
    createRenderTarget(cmd->GetRenderTargetName(), cmd->GetPrimaryType(), cmd->GetUsages());
}

void RendererManager::DoDestroyingRenderTarget(const Frameworks::ICommandPtr& c)
{
    if (!c) return;
    const auto cmd = std::dynamic_pointer_cast<Enigma::Renderer::DestroyRenderTarget, Frameworks::ICommand>(c);
    if (!cmd) return;
    destroyRenderTarget(cmd->GetRenderTargetName());
}

void RendererManager::DoChangingViewPort(const Frameworks::ICommandPtr& c) const
{
    if (!c) return;
    const auto cmd = std::dynamic_pointer_cast<ChangeTargetViewPort, Frameworks::ICommand>(c);
    if (!cmd) return;
    const auto target = getRenderTarget(cmd->GetRenderTargetName());
    if (!target) return;
    target->SetViewPort(cmd->GetViewPort());
}

void RendererManager::DoChangingClearingProperty(const Frameworks::ICommandPtr& c) const
{
    if (!c) return;
    const auto cmd = std::dynamic_pointer_cast<ChangeTargetClearingProperty, Frameworks::ICommand>(c);
    if (!cmd) return;
    const auto target = getRenderTarget(cmd->GetRenderTargetName());
    if (!target) return;
    target->changeClearingProperty(cmd->GetProperty());
}
