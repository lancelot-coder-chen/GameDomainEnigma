﻿#include "TextureManager.h"
#include "TextureLoader.h"
#include "Platforms/MemoryMacro.h"
#include "TextureCommands.h"
#include "TextureEvents.h"
#include "Frameworks/EventPublisher.h"
#include "Frameworks/CommandBus.h"
#include "EngineErrors.h"
#include "Platforms/PlatformLayer.h"
#include <cassert>

using namespace Enigma::Engine;

DEFINE_RTTI(Engine, TextureManager);

TextureManager::TextureManager(Frameworks::ServiceManager* srv_manager) : ISystemService(srv_manager)
{
    IMPLEMENT_RTTI(Enigma, Engine, TextureManager, ISystemService);
    m_needTick = false;
    m_isCurrentLoading = false;
    m_loader = new TextureLoader(this);
}

TextureManager::~TextureManager()
{
    SAFE_DELETE(m_loader);
}

Enigma::Frameworks::ServiceResult TextureManager::OnInit()
{
    m_onTextureLoaded =
        std::make_shared<Frameworks::EventSubscriber>([=](auto c) { this->OnTextureLoaded(c); });
    Frameworks::EventPublisher::Subscribe(typeid(TextureLoader::TextureLoaded), m_onTextureLoaded);
    m_onLoadTextureFailed =
        std::make_shared<Frameworks::EventSubscriber>([=](auto c) { this->OnLoadTextureFailed(c); });
    Frameworks::EventPublisher::Subscribe(typeid(LoadTextureFailed), m_onLoadTextureFailed);

    m_doLoadingTexture =
        std::make_shared<Frameworks::CommandSubscriber>([=](auto c) { this->DoLoadingTexture(c); });
    Frameworks::CommandBus::Subscribe(typeid(Engine::LoadTexture), m_doLoadingTexture);

    return Frameworks::ServiceResult::Complete;
}

Enigma::Frameworks::ServiceResult TextureManager::OnTick()
{
    if (m_isCurrentLoading) return Frameworks::ServiceResult::Pendding;
    std::lock_guard locker{ m_policiesLock };
    if (m_policies.empty())
    {
        m_needTick = false;
        return Frameworks::ServiceResult::Pendding;
    }
    assert(m_loader);
    m_loader->LoadTexture(m_policies.front());
    m_policies.pop();
    m_isCurrentLoading = true;
    return Frameworks::ServiceResult::Pendding;
}

Enigma::Frameworks::ServiceResult TextureManager::OnTerm()
{
    Frameworks::EventPublisher::Unsubscribe(typeid(TextureLoader::TextureLoaded), m_onTextureLoaded);
    m_onTextureLoaded = nullptr;
    Frameworks::EventPublisher::Unsubscribe(typeid(LoadTextureFailed), m_onLoadTextureFailed);
    m_onLoadTextureFailed = nullptr;

    Frameworks::CommandBus::Unsubscribe(typeid(Engine::LoadTexture), m_doLoadingTexture);
    m_doLoadingTexture = nullptr;
    return Frameworks::ServiceResult::Complete;
}

bool TextureManager::HasTexture(const std::string& name)
{
    std::lock_guard locker{ m_textureMapLock };
    auto it = m_textures.find(name);
    return ((it != m_textures.end()) && (!it->second.expired()));
}

std::shared_ptr<Texture> TextureManager::QueryTexture(const std::string& name)
{
    std::lock_guard locker{ m_textureMapLock };
    auto it = m_textures.find(name);
    if (it == m_textures.end()) return nullptr;
    if (it->second.expired()) return nullptr;
    return it->second.lock();
}

error TextureManager::LoadTexture(const TexturePolicy& policy)
{
    std::lock_guard locker{ m_policiesLock };
    m_policies.push(policy);
    m_needTick = true;
    return ErrorCode::ok;
}

void TextureManager::OnTextureLoaded(const Frameworks::IEventPtr& e)
{
    assert(m_loader);
    if (!e) return;
    auto ev = std::dynamic_pointer_cast<TextureLoader::TextureLoaded, Frameworks::IEvent>(e);
    if (!ev) return;
    std::lock_guard locker{ m_textureMapLock };
    m_textures.try_emplace(ev->GetTextureName(), ev->GetTexture());
    m_isCurrentLoading = false;
    Frameworks::EventPublisher::Post(std::make_shared<TextureLoaded>(ev->GetTextureName(), ev->GetTexture()));
}

void TextureManager::OnLoadTextureFailed(const Frameworks::IEventPtr& e)
{
    assert(m_loader);
    if (!e) return;
    auto ev = std::dynamic_pointer_cast<LoadTextureFailed, Frameworks::IEvent>(e);
    if (!ev) return;
    Platforms::Debug::ErrorPrintf("texture %s load failed : %s\n",
        ev->GetTextureName().c_str(), ev->GetError().message().c_str());
    m_isCurrentLoading = false;
}

void TextureManager::DoLoadingTexture(const Frameworks::ICommandPtr& c)
{
    if (!c) return;
    auto cmd = std::dynamic_pointer_cast<Engine::LoadTexture, Frameworks::ICommand>(c);
    if (!cmd) return;
    LoadTexture(cmd->GetPolicy());
}
