﻿/*********************************************************************
 * \file   TextureManager.h
 * \brief  
 * 
 * \author Lancelot 'Robin' Chen
 * \date   September 2022
 *********************************************************************/
#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include "Frameworks/SystemService.h"
#include "Frameworks/ServiceManager.h"
#include "Texture.h"
#include "TextureLoadingPolicies.h"
#include "Frameworks/Command.h"
#include "Frameworks/Event.h"
#include "Frameworks/CommandSubscriber.h"
#include "Frameworks/EventSubscriber.h"
#include <queue>

namespace Enigma::Engine
{
    using error = std::error_code;

    class TextureLoader;

    class TextureManager : public Frameworks::ISystemService
    {
        DECLARE_EN_RTTI;
    public:
        TextureManager(Frameworks::ServiceManager* srv_manager);
        TextureManager(const TextureManager&) = delete;
        TextureManager(TextureManager&&) = delete;
        ~TextureManager();
        TextureManager& operator=(const TextureManager&) = delete;
        TextureManager& operator=(TextureManager&&) = delete;

        virtual Frameworks::ServiceResult OnInit() override;
        virtual Frameworks::ServiceResult OnTick() override;
        virtual Frameworks::ServiceResult OnTerm() override;

        error LoadTexture(const TexturePolicy& policy);

        bool HasTexture(const std::string& name);
        std::shared_ptr<Texture> QueryTexture(const std::string& name);

    private:
        void OnTextureLoaded(const Frameworks::IEventPtr& e);
        void OnLoadTextureFailed(const Frameworks::IEventPtr& e);
        void DoLoadingTexture(const Frameworks::ICommandPtr& c);

    private:
        Frameworks::EventSubscriberPtr m_onTextureLoaded;
        Frameworks::EventSubscriberPtr m_onLoadTextureFailed;
        Frameworks::CommandSubscriberPtr m_doLoadingTexture;

    	using TextureMap = std::unordered_map<std::string, std::weak_ptr<Texture>>;

        TextureMap m_textures;
        std::recursive_mutex m_textureMapLock;

        TextureLoader* m_loader;
        std::queue<TexturePolicy> m_policies;
        bool m_isCurrentLoading;
        std::mutex m_policiesLock;
    };
}

#endif // TEXTURE_MANAGER_H
