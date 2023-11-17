﻿/*********************************************************************
 * \file   SceneGraphRepository.h
 * \brief  scene graph objects repository, maintain objects lifecycle
 *      and some global settings
 * \author Lancelot 'Robin' Chen
 * \date   September 2022
 *********************************************************************/
#ifndef SCENE_GRAPH_REPOSITORY_H
#define SCENE_GRAPH_REPOSITORY_H

#include "Frameworks/SystemService.h"
#include "GameEngine/DtoDeserializer.h"
#include "SceneGraphDefines.h"
#include "Frustum.h"
#include "Renderer/RenderablePrimitivePolicies.h"
#include "Frameworks/CommandSubscriber.h"
#include "Frameworks/QuerySubscriber.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

namespace Enigma::SceneGraph
{
    class Spatial;
    class Camera;
    class Frustum;
    class Node;
    class Pawn;
    class Portal;
    class LightInfo;
    class Light;
    class CameraDto;
    class NodeDto;
    class LazyNodeDto;
    class VisibilityManagedNodeDto;
    class LightDto;
    class PawnDto;
    class PortalDto;
    class PortalZoneNodeDto;
    class PortalManagementNodeDto;
    class SceneGraphBuilder;
    class SceneQuadTreeRoot;

    class SceneGraphRepository : public Frameworks::ISystemService
    {
        DECLARE_EN_RTTI;
    public:
        SceneGraphRepository(Frameworks::ServiceManager* srv_mngr, const std::shared_ptr<Engine::IDtoDeserializer>& dto_deserializer);
        SceneGraphRepository(const SceneGraphRepository&) = delete;
        SceneGraphRepository(SceneGraphRepository&&) = delete;
        virtual ~SceneGraphRepository() override;
        SceneGraphRepository& operator=(const SceneGraphRepository&) = delete;
        SceneGraphRepository& operator=(SceneGraphRepository&&) = delete;

        virtual Frameworks::ServiceResult onInit() override;
        virtual Frameworks::ServiceResult onTerm() override;

        void setCoordinateSystem(GraphicCoordSys hand);
        GraphicCoordSys getCoordinateSystem();

        std::shared_ptr<Camera> createCamera(const std::string& name);
        std::shared_ptr<Camera> createCamera(const Engine::GenericDto& dto);

        std::shared_ptr<Node> CreateNode(const std::string& name, const Frameworks::Rtti& rtti);
        bool HasNode(const std::string& name);
        std::shared_ptr<Node> QueryNode(const std::string& name);

        std::shared_ptr<Pawn> CreatePawn(const std::string& name);
        bool HasPawn(const std::string& name);
        std::shared_ptr<Pawn> QueryPawn(const std::string& name);

        std::shared_ptr<Light> CreateLight(const std::string& name, const LightInfo& info);
        bool HasLight(const std::string& name);
        std::shared_ptr<Light> QueryLight(const std::string& name);

        std::shared_ptr<Portal> CreatePortal(const std::string& name);
        bool HasPortal(const std::string& name);
        std::shared_ptr<Portal> QueryPortal(const std::string& name);

        std::shared_ptr<Spatial> QuerySpatial(const std::string& name);
        std::shared_ptr<Spatial> AddNewSpatial(Spatial* spatial);

    private:
        bool hasCamera(const std::string& name);
        std::shared_ptr<Camera> queryCamera(const std::string& name);
        bool hasQuadTreeRoot(const std::string& name);
        std::shared_ptr<SceneQuadTreeRoot> queryQuadTreeRoot(const std::string& name);

        void queryCamera(const Frameworks::IQueryPtr& q);
        void queryQuadTreeRoot(const Frameworks::IQueryPtr& q);
        void createCamera(const Frameworks::ICommandPtr& c);

    private:
        GraphicCoordSys m_handSystem;

        std::unordered_map<std::string, std::weak_ptr<Camera>> m_cameras;
        std::recursive_mutex m_cameraMapLock;

        std::unordered_map<std::string, std::weak_ptr<Node>> m_nodes;
        std::recursive_mutex m_nodeMapLock;
        std::unordered_map<std::string, std::weak_ptr<Pawn>> m_pawns;
        std::recursive_mutex m_pawnMapLock;

        std::unordered_map<std::string, std::weak_ptr<Light>> m_lights;
        std::recursive_mutex m_lightMapLock;

        std::unordered_map<std::string, std::weak_ptr<Portal>> m_portals;
        std::recursive_mutex m_portalMapLock;

        std::unordered_map<std::string, std::weak_ptr<SceneQuadTreeRoot>> m_quadTreeRoots;
        std::recursive_mutex m_quadTreeRootMapLock;

        SceneGraphBuilder* m_builder;

        Frameworks::QuerySubscriberPtr m_queryCamera;
        Frameworks::QuerySubscriberPtr m_queryQuadTreeRoot;
        Frameworks::CommandSubscriberPtr m_createCamera;
    };
}

#endif // SCENE_GRAPH_REPOSITORY_H
