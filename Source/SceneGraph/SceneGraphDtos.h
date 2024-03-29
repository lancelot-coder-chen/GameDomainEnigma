﻿/*********************************************************************
 * \file   SceneGraphDtos.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   October 2022
 *********************************************************************/
#ifndef SCENE_GRAPH_DTOS_H
#define SCENE_GRAPH_DTOS_H

#include "MathLib/Matrix4.h"
#include "GameEngine/FactoryDesc.h"
#include "GameEngine/GenericDto.h"
#include "Primitives/PrimitiveId.h"
#include "SpatialId.h"
#include <string>
#include <vector>
#include <optional>
#include <memory>

namespace Enigma::SceneGraph
{
    class SpatialDto
    {
    public:
        SpatialDto();
        SpatialDto(const Engine::GenericDto& dto);

        [[nodiscard]] const Engine::FactoryDesc& factoryDesc() const { return m_factoryDesc; }
        Engine::FactoryDesc& factoryDesc() { return m_factoryDesc; }
        [[nodiscard]] bool isTopLevel() const { return m_isTopLevel; }
        bool& isTopLevel() { return m_isTopLevel; }
        SpatialId& id() { return m_id; }
        [[nodiscard]] const SpatialId& id() const { return m_id; }
        const std::string& parentName() const { return m_parentName; }
        std::string& parentName() { return m_parentName; }
        [[nodiscard]] MathLib::Matrix4 localTransform() const { return m_localTransform; }
        MathLib::Matrix4& localTransform() { return m_localTransform; }
        [[nodiscard]] MathLib::Matrix4 worldTransform() const { return m_worldTransform; }
        MathLib::Matrix4& worldTransform() { return m_worldTransform; }
        [[nodiscard]] unsigned int graphDepth() const { return m_graphDepth; }
        unsigned int& graphDepth() { return m_graphDepth; }
        [[nodiscard]] Engine::GenericDto modelBound() const { return m_modelBound; }
        Engine::GenericDto& modelBound() { return m_modelBound; }
        [[nodiscard]] Engine::GenericDto worldBound() const { return m_worldBound; }
        Engine::GenericDto& worldBound() { return m_worldBound; }
        [[nodiscard]] unsigned int cullingMode() const { return m_cullingMode; }
        unsigned int& cullingMode() { return m_cullingMode; }
        [[nodiscard]] unsigned int spatialFlag() const { return m_spatialFlag; }
        unsigned int& spatialFlag() { return m_spatialFlag; }
        [[nodiscard]] unsigned int notifyFlag() const { return m_notifyFlag; }
        unsigned int& notifyFlag() { return m_notifyFlag; }
        [[nodiscard]] const std::optional<SpatialId>& parentId() const { return m_parentId; }
        std::optional<SpatialId>& parentId() { return m_parentId; }

        Engine::GenericDto toGenericDto() const;

    protected:
        Engine::FactoryDesc m_factoryDesc;
        bool m_isTopLevel;
        SpatialId m_id;
        std::string m_parentName;
        MathLib::Matrix4 m_localTransform;
        MathLib::Matrix4 m_worldTransform;
        unsigned int m_graphDepth;
        Engine::GenericDto m_modelBound;
        Engine::GenericDto m_worldBound;
        unsigned int m_cullingMode;
        unsigned int m_spatialFlag;
        unsigned int m_notifyFlag;
        std::optional<SpatialId> m_parentId;
    };

    class NodeDto : public SpatialDto
    {
    public:
        class ChildDto
        {
        public:
            ChildDto() = default;
            ChildDto(const Engine::GenericDto& dto_from);
            ChildDto(const SpatialId& id);
            ChildDto(const SpatialId& id, const Engine::GenericDto& child_dto);

            const SpatialId& id() const { return m_id; }
            const std::optional<Engine::GenericDto>& dto() const { return m_dto; }

            Engine::GenericDto toGenericDto() const;
        protected:
            SpatialId m_id;
            std::optional<Engine::GenericDto> m_dto;
        };
    public:
        NodeDto();
        NodeDto(const SpatialDto& spatial_dto);
        NodeDto(const Engine::GenericDto& dto);

        const std::vector<ChildDto>& children() const { return m_children; }
        std::vector<ChildDto>& children() { return m_children; }

        Engine::GenericDto toGenericDto() const;

    protected:
        std::vector<SpatialId> tokensToIds(const std::vector<std::string>& child_tokens);
        std::vector<std::string> idsToTokens(const std::vector<SpatialId> child_ids) const;

    protected:
        std::vector<ChildDto> m_children;
    };

    class LightDto : public SpatialDto
    {
    public:
        LightDto();
        LightDto(const SpatialDto& spatial_dto);
        LightDto(const Engine::GenericDto& dto);

        [[nodiscard]] Engine::GenericDto lightInfo() const { return m_lightInfo; }
        Engine::GenericDto& lightInfo() { return m_lightInfo; }

        Engine::GenericDto toGenericDto() const;

    protected:
        Engine::GenericDto m_lightInfo;
    };

    class PawnDto : public SpatialDto
    {
    public:
        PawnDto();
        PawnDto(const SpatialDto& spatial_dto);
        PawnDto(const Engine::GenericDto& dto);

        [[nodiscard]] const std::optional<Primitives::PrimitiveId>& primitiveId() const { return m_primitiveId; }
        std::optional<Primitives::PrimitiveId>& primitiveId() { return m_primitiveId; }

        Engine::GenericDto toGenericDto() const;

    protected:
        std::optional<Primitives::PrimitiveId> m_primitiveId;
    };

    class LazyNodeDto : public NodeDto
    {
    public:
        LazyNodeDto();
        LazyNodeDto(const NodeDto& node_dto);
        LazyNodeDto(const Engine::GenericDto& dto);

        Engine::GenericDto toGenericDto() const;
    };

    class VisibilityManagedNodeDto : public LazyNodeDto
    {
    public:
        VisibilityManagedNodeDto();
        VisibilityManagedNodeDto(const LazyNodeDto& lazy_node_dto);
        VisibilityManagedNodeDto(const Engine::GenericDto& dto);

        Engine::GenericDto toGenericDto() const;
    };
}

#endif // SCENE_GRAPH_DTOS_H
