﻿/********************************************************************
 * \file   RendererManager.h
 * \brief  Renderer Manager, manage renderer(s) and render targets
 *
 * \author Lancelot 'Robin' Chen
 * \date   June 2022
 *********************************************************************/
#ifndef RENDERER_MANAGER_H
#define RENDERER_MANAGER_H

#include "RenderTarget.h"
#include "Frameworks/SystemService.h"
#include "Frameworks/Rtti.h"
#include "GameEngine/IRenderer.h"
#include "GameEngine/RenderBufferRepository.h"
#include <system_error>
#include <memory>
#include <unordered_map>
#include <functional>

namespace Enigma::Renderer
{
    class RenderElementBuilder;

    using error = std::error_code;

    using  CustomRendererFactoryFunc = std::function<Engine::IRendererPtr(const std::string&)>;

    /** Renderer Manager Service */
    class RendererManager : public Frameworks::ISystemService
    {
        DECLARE_EN_RTTI;
    public:
        RendererManager(Frameworks::ServiceManager* srv_mngr);
        RendererManager(const RendererManager&) = delete;
        RendererManager(RendererManager&&) = delete;
        virtual ~RendererManager() override;
        RendererManager& operator=(const RendererManager&) = delete;
        RendererManager& operator=(RendererManager&&) = delete;

        /// On Init
        virtual Frameworks::ServiceResult onInit() override;
        /// On Term
        virtual Frameworks::ServiceResult onTerm() override;

        /** register renderer factory */
        static void RegisterCustomRendererFactory(const std::string& type_name, const CustomRendererFactoryFunc& fn);

        /** create renderer */
        error createRenderer(const std::string& name);
        /** create custom renderer */
        error CreateCustomRenderer(const std::string& type_name, const std::string& name);
        /** insert renderer */
        error InsertRenderer(const std::string& name, const Engine::IRendererPtr& renderer);

        /** destroy renderer by name : remove from map, & destroy  */
        error destroyRenderer(const std::string& name);
        /** get renderer */
        Engine::IRendererPtr getRenderer(const std::string& name) const;

        /** create render target */
        error createRenderTarget(const std::string& name, RenderTarget::PrimaryType primary, const std::vector<Graphics::RenderTextureUsage>& usages);

        /** destroy render target by name : remove from map, & destroy  */
        error destroyRenderTarget(const std::string& name);
        /** get render target */
        RenderTargetPtr getRenderTarget(const std::string& name) const;
        /** get primary render target */
        RenderTargetPtr GetPrimaryRenderTarget() const;

    protected:
        void ClearAllRenderer();
        void ClearAllRenderTarget();

        void DoCreatingRenderer(const Frameworks::ICommandPtr& c);
        void DoDestroyingRenderer(const Frameworks::ICommandPtr& c);
        void DoCreatingRenderTarget(const Frameworks::ICommandPtr& c);
        void DoDestroyingRenderTarget(const Frameworks::ICommandPtr& c);
        void DoResizingPrimaryTarget(const Frameworks::ICommandPtr& c) const;
        void DoChangingViewPort(const Frameworks::ICommandPtr& c) const;
        void DoChangingClearingProperty(const Frameworks::ICommandPtr& c) const;

    protected:
        Frameworks::CommandSubscriberPtr m_doCreatingRenderer;
        Frameworks::CommandSubscriberPtr m_doDestroyingRenderer;
        Frameworks::CommandSubscriberPtr m_doCreatingRenderTarget;
        Frameworks::CommandSubscriberPtr m_doDestroyingRenderTarget;
        Frameworks::CommandSubscriberPtr m_doResizingPrimaryTarget;
        Frameworks::CommandSubscriberPtr m_doChangingViewPort;
        Frameworks::CommandSubscriberPtr m_doChangingClearingProperty;

        using RendererMap = std::unordered_map<std::string, Engine::IRendererPtr>;
        using RenderTargetMap = std::unordered_map<std::string, RenderTargetPtr>;
        RendererMap m_renderers;
        RenderTargetMap m_renderTargets;

        std::string m_primaryRenderTargetName;

        unsigned int m_accumulateRendererStamp;  ///< 記錄哪些stamp bit已經被使用過

        using CustomRendererFactoryTable = std::unordered_map<std::string, CustomRendererFactoryFunc>;
        static CustomRendererFactoryTable m_customRendererFactoryTable;

    };
};


#endif // RENDERER_MANAGER_H
