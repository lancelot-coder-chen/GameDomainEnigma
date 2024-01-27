﻿/*********************************************************************
 * \file   RenderablePrimitiveBuilder.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   December 2022
 *********************************************************************/
#ifndef _RENDERABLE_PRIMITIVE_BUILDER_H
#define _RENDERABLE_PRIMITIVE_BUILDER_H

#include "Frameworks/SystemService.h"
#include "Frameworks/EventSubscriber.h"
#include "Frameworks/CommandSubscriber.h"
#include "Primitives/Primitive.h"
#include "Primitives/PrimitiveId.h"
#include "Geometries/GeometryRepository.h"
#include "Primitives/PrimitiveRepository.h"
#include "PrimitiveHydratingPlan.h"
#include <queue>
#include <mutex>
#include <system_error>

namespace Enigma::Renderables
{
    using error = std::error_code;

    class MeshPrimitiveBuilder;
    class ModelPrimitiveBuilder;

    class RenderablePrimitiveBuilder : public Frameworks::ISystemService
    {
        DECLARE_EN_RTTI;
    public:
        RenderablePrimitiveBuilder(Frameworks::ServiceManager* mngr, const std::shared_ptr<Primitives::PrimitiveRepository>& primitive_repository, const std::shared_ptr<Geometries::GeometryRepository>& geometry_repository);
        RenderablePrimitiveBuilder(const RenderablePrimitiveBuilder&) = delete;
        RenderablePrimitiveBuilder(RenderablePrimitiveBuilder&&) = delete;
        ~RenderablePrimitiveBuilder() override;
        RenderablePrimitiveBuilder& operator=(const RenderablePrimitiveBuilder&) = delete;
        RenderablePrimitiveBuilder& operator=(RenderablePrimitiveBuilder&&) = delete;

        virtual Frameworks::ServiceResult onInit() override;
        virtual Frameworks::ServiceResult onTick() override;
        virtual Frameworks::ServiceResult onTerm() override;

    protected:
        std::shared_ptr<Primitives::Primitive> createMesh(const Primitives::PrimitiveId& id);
        std::shared_ptr<Primitives::Primitive> constituteMesh(const Primitives::PrimitiveId& id, const Engine::GenericDto& dto);
        std::shared_ptr<Primitives::Primitive> createSkinMesh(const Primitives::PrimitiveId& id);
        std::shared_ptr<Primitives::Primitive> constituteSkinMesh(const Primitives::PrimitiveId& id, const Engine::GenericDto& dto);
        std::shared_ptr<Primitives::Primitive> createModel(const Primitives::PrimitiveId& id);
        std::shared_ptr<Primitives::Primitive> constituteModel(const Primitives::PrimitiveId& id, const Engine::GenericDto& dto);

        void hydrateRenderablePrimitive(const PrimitiveHydratingPlan& plan);

        void onPrimitiveHydrated(const Frameworks::IEventPtr& e);
        void onHydratePrimitiveFailed(const Frameworks::IEventPtr& e);

    protected:
        std::weak_ptr<Primitives::PrimitiveRepository> m_primitiveRepository;
        std::weak_ptr<Geometries::GeometryRepository> m_geometryRepository;
        std::queue<PrimitiveHydratingPlan> m_primitivePlans;
        std::mutex m_primitivePlansLock;
        std::optional<Primitives::PrimitiveId> m_currentBuildingId;

        Frameworks::EventSubscriberPtr m_onMeshPrimitiveHydrated;
        Frameworks::EventSubscriberPtr m_onHydrateMeshPrimitiveFailed;

        MeshPrimitiveBuilder* m_meshBuilder;
    };
}

#endif // _RENDERABLE_PRIMITIVE_BUILDER_H
