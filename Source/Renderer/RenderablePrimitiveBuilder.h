﻿/*********************************************************************
 * \file   RenderablePrimitiveBuilder.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   December 2022
 *********************************************************************/
#ifndef _RENDERABLE_PRIMITIVE_BUILDER_H
#define _RENDERABLE_PRIMITIVE_BUILDER_H

#include "RenderablePrimitivePolicies.h"
#include "Frameworks/SystemService.h"
#include "Frameworks/EventSubscriber.h"
#include "Frameworks/CommandSubscriber.h"
#include <queue>
#include <mutex>
#include <system_error>

namespace Enigma::Renderer
{
    using error = std::error_code;

    class MeshPrimitiveBuilder;
    class ModelPrimitiveBuilder;

    class RenderablePrimitiveBuilder : public Frameworks::ISystemService
    {
        DECLARE_EN_RTTI;
    public:
        RenderablePrimitiveBuilder(Frameworks::ServiceManager* mngr);
        RenderablePrimitiveBuilder(const RenderablePrimitiveBuilder&) = delete;
        RenderablePrimitiveBuilder(RenderablePrimitiveBuilder&&) = delete;
        ~RenderablePrimitiveBuilder() override;
        RenderablePrimitiveBuilder& operator=(const RenderablePrimitiveBuilder&) = delete;
        RenderablePrimitiveBuilder& operator=(RenderablePrimitiveBuilder&&) = delete;

        virtual Frameworks::ServiceResult OnInit() override;
        virtual Frameworks::ServiceResult OnTick() override;
        virtual Frameworks::ServiceResult OnTerm() override;

        error BuildPrimitive(const std::shared_ptr<RenderablePrimitivePolicy>& policy);

    protected:
        void BuildRenderablePrimitive(const std::shared_ptr<RenderablePrimitivePolicy>& policy);

        void OnPrimitiveBuilt(const Frameworks::IEventPtr& e);
        void OnBuildPrimitiveFailed(const Frameworks::IEventPtr& e);
        void DoBuildingPrimitive(const Frameworks::ICommandPtr& c);

    protected:
        std::queue<std::shared_ptr<RenderablePrimitivePolicy>> m_policies;
        std::mutex m_policiesLock;
        bool m_isCurrentBuilding;
        Frameworks::Ruid m_buildingRuid;

        Frameworks::EventSubscriberPtr m_onPrimitiveBuilt;
        Frameworks::EventSubscriberPtr m_onBuildPrimitiveFailed;
        Frameworks::CommandSubscriberPtr m_doBuildingPrimitive;

        MeshPrimitiveBuilder* m_meshBuilder;
        ModelPrimitiveBuilder* m_modelBuilder;
    };
}

#endif // _RENDERABLE_PRIMITIVE_BUILDER_H