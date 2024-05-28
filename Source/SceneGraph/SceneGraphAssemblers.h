﻿/*********************************************************************
 * \file   SceneGraphAssemblers.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   May 2023
 *********************************************************************/
#ifndef SCENE_GRAPH_ASSEMBLERS_H
#define SCENE_GRAPH_ASSEMBLERS_H

#include "CameraFrustumDtos.h"
#include "GameEngine/BoundingVolume.h"
#include "SceneGraphDtos.h"
#include "SceneGraph/Spatial.h"
#include "PortalDtos.h"
#include <string>

namespace Enigma::SceneGraph
{
    class Camera;
    class Pawn;
    class PortalManagementNode;
    class VisibilityManagedNode;
    class CameraAssembler
    {
    public:
        CameraAssembler(const SpatialId& id);

        CameraAssembler& eyePosition(const MathLib::Vector3& eye_position);
        CameraAssembler& lookAt(const MathLib::Vector3& look_at);
        CameraAssembler& upDirection(const MathLib::Vector3& up_direction);

        CameraAssembler& frustum(Frustum::ProjectionType projection_type);
        CameraAssembler& frustumFrontBackZ(float front_z, float back_z);
        CameraAssembler& frustumFov(float fov);
        CameraAssembler& frustumNearPlaneDimension(float width, float height);

        CameraAssembler& asNative(const std::string& file_at_path);
        CameraDto toCameraDto();

        std::shared_ptr<Camera> constitute();

    protected:
        Engine::GenericDto toGenericDto();

    protected:
        SpatialId m_id;
        CameraDto m_cameraDto;
        FrustumDto m_frustumDto;
    };

    class SpatialAssembler
    {
    public:
        SpatialAssembler(const SpatialId& id);

        const SpatialId& id() const { return m_id; }

        SpatialAssembler& factory(const Engine::FactoryDesc& factory);
        SpatialAssembler& localTransform(const MathLib::Matrix4& local_transform);
        SpatialAssembler& worldTransform(const MathLib::Matrix4& world_transform);
        SpatialAssembler& modelBound(const Engine::BoundingVolume& model_bound);
        SpatialAssembler& topLevel(bool top_level);
        SpatialAssembler& cullingMode(Spatial::CullingMode cull_mode);
        SpatialAssembler& notifyFlags(Spatial::NotifyFlags notify_flags);
        SpatialAssembler& spatialFlags(Spatial::SpatialFlags spatial_flags);
        SpatialAssembler& graphDepth(unsigned graph_depth);

        Engine::BoundingVolume modelBound() const { return m_modelBound; }
        MathLib::Matrix4 localTransform() const { return m_dto.localTransform(); }
        MathLib::Matrix4 worldTransform() const { return m_dto.worldTransform(); }

        Engine::GenericDto toGenericDto() const;

    private:
        SpatialId m_id;
        SpatialDto m_dto;
        Engine::BoundingVolume m_modelBound;
    };

    class PawnAssembler
    {
    public:
        PawnAssembler(const SpatialId& id);

        const SpatialId& id() const { return m_id; }

        SpatialAssembler& spatial();

        PawnAssembler& factory(const Engine::FactoryDesc& factory);
        PawnAssembler& primitive(const Primitives::PrimitiveId& primitive_id);
        PawnAssembler& asNative(const std::string& file_at_path);

        Engine::GenericDto toGenericDto() const;
        PawnDto toPawnDto() const;
        std::shared_ptr<Pawn> constitute();

    private:
        SpatialId m_id;
        SpatialAssembler m_spatialAssembler;
        PawnDto m_dto;
    };

    class NodeAssembler
    {
    public:
        NodeAssembler(const SpatialId& id);

        const SpatialId& id() const { return m_id; }

        SpatialAssembler& spatial();

        NodeAssembler& factory(const Engine::FactoryDesc& factory);
        NodeAssembler& child(const SpatialId& child_id);
        NodeAssembler& child(const SpatialId& child_id, const Engine::GenericDto& child_dto);
        NodeAssembler& child(const SpatialId& child_id, const SpatialAssembler* child);
        NodeAssembler& child(const SpatialId& child_id, const NodeAssembler* child);
        NodeAssembler& asNative(const std::string& file_at_path);

        void consistChildrenLocationBounding();

        Engine::GenericDto toGenericDto() const;
        NodeDto toNodeDto() const;
        std::shared_ptr<Node> constitute();

    private:
        SpatialId m_id;
        SpatialAssembler m_spatialAssembler;
        NodeDto m_dto;
        std::vector<SpatialAssembler*> m_children;
        std::vector<NodeAssembler*> m_childNodes;
    };

    class LazyNodeAssembler
    {
    public:
        LazyNodeAssembler(const SpatialId& id);

        const SpatialId& id() const { return m_id; }

        NodeAssembler& node();

        LazyNodeAssembler& factory(const Engine::FactoryDesc& factory);
        LazyNodeAssembler& asDeferred(const std::string& file_name, const std::string& path_id);

        Engine::GenericDto toHydratedGenericDto() const;
        LazyNodeDto toHydratedDto() const;
        std::shared_ptr<LazyNode> constituteHydrated();
        Engine::GenericDto toDeHydratedGenericDto() const;
        LazyNodeDto toDeHydratedDto() const;
        std::shared_ptr<LazyNode> constituteDeHydrated();

    private:
        SpatialId m_id;
        NodeAssembler m_nodeAssembler;
        LazyNodeDto m_dto;
    };

    class PortalAssembler
    {
    public:
        PortalAssembler(const SpatialId& id);

        const SpatialId& id() const { return m_id; }

        SpatialAssembler& spatial();

        PortalAssembler& factory(const Engine::FactoryDesc& factory);
        PortalAssembler& adjacentZoneNodeId(const SpatialId& adjacent_zone_node_id);
        PortalAssembler& isOpen(bool is_open);
        PortalAssembler& asNative(const std::string& file_at_path);

        Engine::GenericDto toGenericDto() const;
        PortalDto toPortalDto() const;
        std::shared_ptr<Portal> constitute();

    private:
        SpatialId m_id;
        SpatialAssembler m_spatialAssembler;
        PortalDto m_dto;
    };

    class PortalZoneNodeAssembler
    {
    public:
        PortalZoneNodeAssembler(const SpatialId& id);

        const SpatialId& id() const { return m_id; }

        LazyNodeAssembler& lazyNode();

        PortalZoneNodeAssembler& factory(const Engine::FactoryDesc& factory);
        PortalZoneNodeAssembler& portalParentId(const SpatialId& portal_parent_id);
        PortalZoneNodeAssembler& asDeferred(const std::string& file_name, const std::string& path_id);

        Engine::GenericDto toHydratedGenericDto() const;
        PortalZoneNodeDto toHydratedDto() const;
        std::shared_ptr<PortalZoneNode> constituteHydrated();
        Engine::GenericDto toDeHydratedGenericDto() const;
        PortalZoneNodeDto toDeHydratedDto() const;
        std::shared_ptr<PortalZoneNode> constituteDeHydrated();

    private:
        SpatialId m_id;
        LazyNodeAssembler m_lazyNodeAssembler;
        PortalZoneNodeDto m_dto;
    };

    class PortalManagementNodeAssembler
    {
    public:
        PortalManagementNodeAssembler(const SpatialId& id);

        const SpatialId& id() const { return m_id; }

        NodeAssembler& node();

        PortalManagementNodeAssembler& factory(const Engine::FactoryDesc& factory);
        PortalManagementNodeAssembler& outsideZoneNodeId(const SpatialId& outside_zone_node_id);
        PortalManagementNodeAssembler& asNative(const std::string& file_at_path);

        Engine::GenericDto toGenericDto() const;
        PortalManagementNodeDto toPortalManagementNodeDto() const;
        std::shared_ptr<PortalManagementNode> constitute();

    private:
        SpatialId m_id;
        NodeAssembler m_nodeAssembler;
        PortalManagementNodeDto m_dto;
    };

    class VisibilityManagedNodeAssembler
    {
    public:
        VisibilityManagedNodeAssembler(const SpatialId& id);

        const SpatialId& id() const { return m_id; }

        LazyNodeAssembler& lazyNode();

        VisibilityManagedNodeAssembler& factory(const Engine::FactoryDesc& factory);
        VisibilityManagedNodeAssembler& asDeferred(const std::string& file_name, const std::string& path_id);

        Engine::GenericDto toHydratedGenericDto() const;
        VisibilityManagedNodeDto toHydratedDto() const;
        std::shared_ptr<VisibilityManagedNode> constituteHydrated();
        Engine::GenericDto toDeHydratedGenericDto() const;
        VisibilityManagedNodeDto toDeHydratedDto() const;
        std::shared_ptr<VisibilityManagedNode> constituteDeHydrated();

    private:
        SpatialId m_id;
        LazyNodeAssembler m_lazyNodeAssembler;
        VisibilityManagedNodeDto m_dto;
    };
}

#endif // SCENE_GRAPH_ASSEMBLERS_H
