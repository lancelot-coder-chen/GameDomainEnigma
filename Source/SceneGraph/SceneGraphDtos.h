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
#include "GameEngine/BoundingVolumeDto.h"
#include <string>
#include <vector>

namespace Enigma::SceneGraph
{
    class SpatialDto
    {
    public:
        SpatialDto() = default;

        [[nodiscard]] bool IsTopLevel() const { return m_isTopLevel; }
        bool& IsTopLevel() { return m_isTopLevel; }
        const std::string& Name() const { return m_name; }
        std::string& Name() { return m_name; }
        [[nodiscard]] MathLib::Matrix4 LocalTransform() const { return m_localTransform; }
        MathLib::Matrix4& LocalTransform() { return m_localTransform; }
        [[nodiscard]] MathLib::Matrix4 WorldTransform() const { return m_worldTransform; }
        MathLib::Matrix4& WorldTransform() { return m_worldTransform; }
        [[nodiscard]] unsigned int GraphDepth() const { return m_graphDepth; }
        unsigned int& GraphDepth() { return m_graphDepth; }
        [[nodiscard]] Engine::GenericDto ModelBound() const { return m_modelBound; }
        Engine::GenericDto& ModelBound() { return m_modelBound; }
        [[nodiscard]] Engine::GenericDto WorldBound() const { return m_worldBound; }
        Engine::GenericDto& WorldBound() { return m_worldBound; }
        [[nodiscard]] unsigned int CullingMode() const { return m_cullingMode; }
        unsigned int& CullingMode() { return m_cullingMode; }
        [[nodiscard]] unsigned int SpatialFlag() const { return m_spatialFlag; }
        unsigned int& SpatialFlag() { return m_spatialFlag; }
        [[nodiscard]] unsigned int NotifyFlag() const { return m_notifyFlag; }
        unsigned int& NotifyFlag() { return m_notifyFlag; }

        static SpatialDto FromGenericDto(const Engine::GenericDto& dto);
        Engine::GenericDto ToGenericDto();

    protected:
        bool m_isTopLevel;
        std::string m_name;
        MathLib::Matrix4 m_localTransform;
        MathLib::Matrix4 m_worldTransform;
        unsigned int m_graphDepth;
        Engine::GenericDto m_modelBound;
        Engine::GenericDto m_worldBound;
        unsigned int m_cullingMode;
        unsigned int m_spatialFlag;
        unsigned int m_notifyFlag;
    };

    class NodeDto : public SpatialDto
    {
    public:
        NodeDto() = default;
        NodeDto(const SpatialDto& spatial_dto);

        const std::vector<std::string>& ChildNames() const { return m_childNames; }
        std::vector<std::string>& ChildNames() { return m_childNames; }

        static NodeDto FromGenericDto(const Engine::GenericDto& dto);
        Engine::GenericDto ToGenericDto();

    protected:
        std::vector<std::string> m_childNames;
    };

    class LightDto : public SpatialDto
    {
    public:
        LightDto() = default;
        LightDto(const SpatialDto& spatial_dto);

        [[nodiscard]] Engine::GenericDto LightInfo() const { return m_lightInfo; }
        Engine::GenericDto& LightInfo() { return m_lightInfo; }

        static LightDto FromGenericDto(const Engine::GenericDto& dto);
        Engine::GenericDto ToGenericDto();

    protected:
        Engine::GenericDto m_lightInfo;
    };
}

#endif // SCENE_GRAPH_DTOS_H