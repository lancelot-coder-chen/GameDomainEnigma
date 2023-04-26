﻿/*********************************************************************
 * \file   SceneGraphCommands.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   October 2022
 *********************************************************************/
#ifndef _SCENE_GRAPH_COMMANDS_H
#define _SCENE_GRAPH_COMMANDS_H

#include "Frameworks/Command.h"
#include "GameEngine/GenericDto.h"
#include <string>
#include <vector>

namespace Enigma::SceneGraph
{
    class Node;
    class LazyNode;
    class Spatial;

    class BuildSceneGraph : public Frameworks::ICommand
    {
    public:
        BuildSceneGraph(const std::string& scene_graph_id, const std::vector<Engine::GenericDto>& dtos)
            : m_id(scene_graph_id), m_dtos(dtos) {}

        const std::string& GetSceneGraphId() { return m_id; }
        const std::vector<Engine::GenericDto>& GetDtos() { return m_dtos; }

    protected:
        std::string m_id;
        std::vector<Engine::GenericDto> m_dtos;
    };

    class InstanceLazyNode : public Frameworks::ICommand
    {
    public:
        InstanceLazyNode(const std::shared_ptr<LazyNode>& node) : m_node(node) {}

        const std::shared_ptr<LazyNode>& GetNode() { return m_node; }

    protected:
        std::shared_ptr<LazyNode> m_node;
    };

    class InPlaceBuildSceneGraph : public Frameworks::ICommand
    {
    public:
        InPlaceBuildSceneGraph(const std::shared_ptr<Node>& owner_node, const std::vector<Engine::GenericDto>& dtos)
            : m_ownerNode(owner_node), m_dtos(dtos) {}

        const std::shared_ptr<Node>& GetOwnerNode() { return m_ownerNode; }
        const std::vector<Engine::GenericDto>& GetDtos() { return m_dtos; }

    protected:
        std::shared_ptr<Node> m_ownerNode;
        std::vector<Engine::GenericDto> m_dtos;
    };
    //--------------------------------------------------------------------------------
    using SpatialDtoFactory = std::function<Spatial* (const Engine::GenericDto& dto)>;

    class RegisterSpatialDtoFactory : public Frameworks::ICommand
    {
    public:
        RegisterSpatialDtoFactory(const std::string& rtti, const SpatialDtoFactory& factory)
            : m_rtti(rtti), m_factory(factory) {}

        const std::string& GetRtti() const { return m_rtti; }
        const SpatialDtoFactory& GetFactory() { return m_factory; }

    private:
        std::string m_rtti;
        SpatialDtoFactory m_factory;
    };
    class UnRegisterSpatialDtoFactory : public Frameworks::ICommand
    {
    public:
        UnRegisterSpatialDtoFactory(const std::string& rtti) : m_rtti(rtti) {}

        const std::string& GetRtti() const { return m_rtti; }

    private:
        std::string m_rtti;
    };
    class InvokeSpatialDtoFactory : public Frameworks::ICommand
    {
    public:
        InvokeSpatialDtoFactory(const Engine::GenericDto& dto) : m_dto(dto) {}

        const Engine::GenericDto& GetDto() { return m_dto; }

    private:
        Engine::GenericDto m_dto;
    };
}

#endif // _SCENE_GRAPH_COMMANDS_H
