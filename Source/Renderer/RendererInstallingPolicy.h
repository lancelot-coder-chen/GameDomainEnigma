﻿/*********************************************************************
 * \file   RendererInstallingPolicy.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   March 2023
 *********************************************************************/
#ifndef _RENDERER_INSTALLING_POLICY_H
#define _RENDERER_INSTALLING_POLICY_H

#include "GameEngine/InstallingPolicy.h"

namespace Enigma::Renderer
{
    using error = std::error_code;

    class RenderSystemInstallingPolicy : public Engine::InstallingPolicy
    {
    public:
        RenderSystemInstallingPolicy() = default;
        virtual ~RenderSystemInstallingPolicy() override = default;

        virtual error Install(Frameworks::ServiceManager* service_manager) override;
        virtual error Shutdown(Frameworks::ServiceManager* service_manager) override;
    };

    class RendererInstallingPolicy : public Engine::InstallingPolicy
    {
    public:
        RendererInstallingPolicy() = default;
        virtual ~RendererInstallingPolicy() override = default;
    };;

    class DefaultRendererInstallingPolicy : public RendererInstallingPolicy
    {
    public:
        DefaultRendererInstallingPolicy(const std::string& renderer_name, const std::string& primary_target_name)
            : m_rendererName(renderer_name), m_primaryTargetName(primary_target_name) {}
        virtual ~DefaultRendererInstallingPolicy() override = default;

        virtual error Install(Frameworks::ServiceManager* service_manager) override;
        virtual error Shutdown(Frameworks::ServiceManager* service_manager) override;

    protected:
        std::string m_rendererName;
        std::string m_primaryTargetName;
    };
}

#endif // _RENDERER_INSTALLING_POLICY_H
