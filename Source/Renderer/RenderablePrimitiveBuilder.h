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
#include "Frameworks/RequestSubscriber.h"
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

        virtual Frameworks::ServiceResult onInit() override;
        virtual Frameworks::ServiceResult onTick() override;
        virtual Frameworks::ServiceResult onTerm() override;

        error BuildPrimitive(const Frameworks::Ruid& requester_ruid, const std::shared_ptr<RenderablePrimitivePolicy>& policy);

    protected:
        void BuildRenderablePrimitive(const Frameworks::Ruid& requester_ruid, const std::shared_ptr<RenderablePrimitivePolicy>& policy);

        void OnPrimitiveBuilt(const Frameworks::IEventPtr& e);
        void OnBuildPrimitiveFailed(const Frameworks::IEventPtr& e);

        void DoBuildingPrimitive(const Frameworks::IRequestPtr& r);

    protected:
        std::queue<std::tuple<Frameworks::Ruid, std::shared_ptr<RenderablePrimitivePolicy>>> m_policies;
        std::mutex m_policiesLock;
        bool m_isCurrentBuilding;
        Frameworks::Ruid m_buildingRuid;

        Frameworks::EventSubscriberPtr m_onMeshPrimitiveBuilt;
        Frameworks::EventSubscriberPtr m_onBuildMeshPrimitiveFailed;
        Frameworks::EventSubscriberPtr m_onModelPrimitiveBuilt;
        Frameworks::EventSubscriberPtr m_onBuildModelPrimitiveFailed;

        Frameworks::RequestSubscriberPtr m_doBuildingPrimitive;

        MeshPrimitiveBuilder* m_meshBuilder;
        ModelPrimitiveBuilder* m_modelBuilder;
    };
}

#endif // _RENDERABLE_PRIMITIVE_BUILDER_H
