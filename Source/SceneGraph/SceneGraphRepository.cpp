﻿#include "SceneGraphRepository.h"
#include "Camera.h"
#include "Frustum.h"
#include "Node.h"
#include "Pawn.h"
#include "Light.h"
#include "LazyNode.h"
#include "VisibilityManagedNode.h"
#include "SceneGraphEvents.h"
#include "SceneGraphDtos.h"
#include "SceneGraphCommands.h"
#include "SceneGraphErrors.h"
#include "PortalZoneNode.h"
#include "Portal.h"
#include "PortalManagementNode.h"
#include "SceneQuadTreeRoot.h"
#include "Frameworks/EventPublisher.h"
#include "Frameworks/CommandBus.h"
#include "Frameworks/QueryDispatcher.h"
#include "SceneGraphBuilder.h"
#include "Platforms/PlatformLayerUtilities.h"
#include "Platforms/MemoryAllocMacro.h"
#include "Renderer/ModelPrimitive.h"
#include "Platforms/MemoryMacro.h"
#include "CameraFrustumCommands.h"
#include "CameraFrustumEvents.h"
#include "SceneGraphQueries.h"
#include <cassert>

using namespace Enigma::SceneGraph;
using namespace Enigma::Frameworks;
using namespace Enigma::Engine;
using namespace Enigma::Platforms;
using namespace Enigma::Renderer;

DEFINE_RTTI(SceneGraph, SceneGraphRepository, ISystemService);

SceneGraphRepository::SceneGraphRepository(Frameworks::ServiceManager* srv_mngr,
    const std::shared_ptr<Engine::IDtoDeserializer>& dto_deserializer) : ISystemService(srv_mngr)
{
    m_handSystem = GraphicCoordSys::LeftHand;

    m_needTick = false;
    m_builder = menew SceneGraphBuilder(this, dto_deserializer);
}

SceneGraphRepository::~SceneGraphRepository()
{
    SAFE_DELETE(m_builder);
}

ServiceResult SceneGraphRepository::onInit()
{
    m_queryCamera = std::make_shared<QuerySubscriber>([=](const IQueryPtr& q) { queryCamera(q); });
    QueryDispatcher::subscribe(typeid(QueryCamera), m_queryCamera);
    m_queryNode = std::make_shared<QuerySubscriber>([=](const IQueryPtr& q) { queryNode(q); });
    QueryDispatcher::subscribe(typeid(QueryNode), m_queryNode);
    m_queryQuadTreeRoot = std::make_shared<QuerySubscriber>([=](const IQueryPtr& q) { queryQuadTreeRoot(q); });
    QueryDispatcher::subscribe(typeid(QuerySceneQuadTreeRoot), m_queryQuadTreeRoot);
    m_createCamera = std::make_shared<CommandSubscriber>([=](const ICommandPtr& c) { createCamera(c); });
    CommandBus::subscribe(typeid(CreateCamera), m_createCamera);
    m_createNode = std::make_shared<CommandSubscriber>([=](const ICommandPtr& c) { createNode(c); });
    CommandBus::subscribe(typeid(CreateNode), m_createNode);
    m_createQuadTreeRoot = std::make_shared<CommandSubscriber>([=](const ICommandPtr& c) { createQuadTreeRoot(c); });
    CommandBus::subscribe(typeid(CreateSceneQuadTreeRoot), m_createQuadTreeRoot);

    return ServiceResult::Complete;
}
ServiceResult SceneGraphRepository::onTerm()
{
    QueryDispatcher::unsubscribe(typeid(QueryCamera), m_queryCamera);
    m_queryCamera = nullptr;
    QueryDispatcher::unsubscribe(typeid(QueryNode), m_queryNode);
    m_queryNode = nullptr;
    QueryDispatcher::unsubscribe(typeid(QuerySceneQuadTreeRoot), m_queryQuadTreeRoot);
    m_queryQuadTreeRoot = nullptr;
    CommandBus::unsubscribe(typeid(CreateCamera), m_createCamera);
    m_createCamera = nullptr;
    CommandBus::unsubscribe(typeid(CreateNode), m_createNode);
    m_createNode = nullptr;
    CommandBus::unsubscribe(typeid(CreateSceneQuadTreeRoot), m_createQuadTreeRoot);
    m_createQuadTreeRoot = nullptr;

    return ServiceResult::Complete;
}

void SceneGraphRepository::setCoordinateSystem(GraphicCoordSys hand)
{
    m_handSystem = hand;
}

GraphicCoordSys SceneGraphRepository::getCoordinateSystem()
{
    return m_handSystem;
}

std::shared_ptr<Camera> SceneGraphRepository::createCamera(const std::string& name)
{
    assert(!hasCamera(name));
    auto camera = std::make_shared<Camera>(name, m_handSystem);
    std::lock_guard locker{ m_cameraMapLock };
    m_cameras.insert_or_assign(name, camera);
    return camera;
}

std::shared_ptr<Camera> SceneGraphRepository::createCamera(const GenericDto& dto)
{
    assert(!hasCamera(dto.getName()));
    auto camera = std::make_shared<Camera>(dto);
    std::lock_guard locker{ m_cameraMapLock };
    m_cameras.insert_or_assign(dto.getName(), camera);
    return camera;
}

bool SceneGraphRepository::hasCamera(const std::string& name)
{
    std::lock_guard locker{ m_cameraMapLock };
    auto it = m_cameras.find(name);
    return ((it != m_cameras.end()) && (!it->second.expired()));
}

std::shared_ptr<Camera> SceneGraphRepository::queryCamera(const std::string& name)
{
    std::lock_guard locker{ m_cameraMapLock };
    auto it = m_cameras.find(name);
    if (it == m_cameras.end()) return nullptr;
    if (it->second.expired()) return nullptr;
    return it->second.lock();
}

std::shared_ptr<Node> SceneGraphRepository::createNode(const std::string& name, const Rtti& rtti)
{
    assert(!hasNode(name));
    std::shared_ptr<Node> node = nullptr;
    if (rtti == Node::TYPE_RTTI)
    {
        node = std::make_shared<Node>(name);
    }
    else if (rtti == LazyNode::TYPE_RTTI)
    {
        node = std::make_shared<LazyNode>(name);
    }
    else if (rtti == VisibilityManagedNode::TYPE_RTTI)
    {
        node = std::make_shared<VisibilityManagedNode>(name);
    }
    else if (rtti == PortalZoneNode::TYPE_RTTI)
    {
        node = std::make_shared<PortalZoneNode>(name);
    }
    else if (rtti == PortalManagementNode::TYPE_RTTI)
    {
        node = std::make_shared<PortalManagementNode>(name);
    }
    assert(node);
    std::lock_guard locker{ m_nodeMapLock };
    m_nodes.insert_or_assign(name, node);
    return node;
}

bool SceneGraphRepository::hasNode(const std::string& name)
{
    std::lock_guard locker{ m_nodeMapLock };
    auto it = m_nodes.find(name);
    return ((it != m_nodes.end()) && (!it->second.expired()));
}

std::shared_ptr<Node> SceneGraphRepository::queryNode(const std::string& name)
{
    std::lock_guard locker{ m_nodeMapLock };
    auto it = m_nodes.find(name);
    if (it == m_nodes.end()) return nullptr;
    if (it->second.expired()) return nullptr;
    return it->second.lock();
}

std::shared_ptr<Pawn> SceneGraphRepository::CreatePawn(const std::string& name)
{
    assert(!HasPawn(name));
    auto pawn = std::make_shared<Pawn>(name);
    std::lock_guard locker{ m_pawnMapLock };
    m_pawns.insert_or_assign(name, pawn);
    return pawn;
}

bool SceneGraphRepository::HasPawn(const std::string& name)
{
    std::lock_guard locker{ m_pawnMapLock };
    auto it = m_pawns.find(name);
    return ((it != m_pawns.end()) && (!it->second.expired()));
}

std::shared_ptr<Pawn> SceneGraphRepository::QueryPawn(const std::string& name)
{
    std::lock_guard locker{ m_pawnMapLock };
    auto it = m_pawns.find(name);
    if (it == m_pawns.end()) return nullptr;
    if (it->second.expired()) return nullptr;
    return it->second.lock();
}

std::shared_ptr<Light> SceneGraphRepository::CreateLight(const std::string& name, const LightInfo& info)
{
    assert(!HasLight(name));
    auto light = std::make_shared<Light>(name, info);
    std::lock_guard locker{ m_lightMapLock };
    m_lights.insert_or_assign(name, light);
    Frameworks::EventPublisher::post(std::make_shared<LightInfoCreated>(light));
    return light;
}

bool SceneGraphRepository::HasLight(const std::string& name)
{
    std::lock_guard locker{ m_lightMapLock };
    auto it = m_lights.find(name);
    return ((it != m_lights.end()) && (!it->second.expired()));
}

std::shared_ptr<Light> SceneGraphRepository::QueryLight(const std::string& name)
{
    std::lock_guard locker{ m_lightMapLock };
    auto it = m_lights.find(name);
    if (it == m_lights.end()) return nullptr;
    if (it->second.expired()) return nullptr;
    return it->second.lock();
}

std::shared_ptr<Portal> SceneGraphRepository::CreatePortal(const std::string& name)
{
    assert(!HasPortal(name));
    auto portal = std::make_shared<Portal>(name);
    std::lock_guard locker{ m_portalMapLock };
    m_portals.insert_or_assign(name, portal);
    return portal;
}

bool SceneGraphRepository::HasPortal(const std::string& name)
{
    std::lock_guard locker{ m_portalMapLock };
    auto it = m_portals.find(name);
    return ((it != m_portals.end()) && (!it->second.expired()));
}

std::shared_ptr<Portal> SceneGraphRepository::QueryPortal(const std::string& name)
{
    std::lock_guard locker{ m_portalMapLock };
    auto it = m_portals.find(name);
    if (it == m_portals.end()) return nullptr;
    if (it->second.expired()) return nullptr;
    return it->second.lock();
}

std::shared_ptr<Spatial> SceneGraphRepository::QuerySpatial(const std::string& name)
{
    if (auto node = queryNode(name)) return node;
    if (auto pawn = QueryPawn(name)) return pawn;
    if (auto light = QueryLight(name)) return light;
    if (auto portal = QueryPortal(name)) return portal;
    return nullptr;
}

std::shared_ptr<Spatial> SceneGraphRepository::AddNewSpatial(Spatial* spatial)
{
    assert(spatial);
    if (auto pawn = std::shared_ptr<Pawn>(dynamic_cast<Pawn*>(spatial)))
    {
        assert(!HasPawn(pawn->getSpatialName()));
        std::lock_guard locker{ m_pawnMapLock };
        m_pawns.insert_or_assign(pawn->getSpatialName(), pawn);
        return pawn;
    }
    else if (auto node = std::shared_ptr<Node>(dynamic_cast<Node*>(spatial)))
    {
        assert(!hasNode(node->getSpatialName()));
        std::lock_guard locker{ m_nodeMapLock };
        m_nodes.insert_or_assign(node->getSpatialName(), node);
        return node;
    }
    else if (auto portal = std::shared_ptr<Portal>(dynamic_cast<Portal*>(spatial)))
    {
        assert(!HasPortal(portal->getSpatialName()));
        std::lock_guard locker{ m_portalMapLock };
        m_portals.insert_or_assign(portal->getSpatialName(), portal);
        return portal;
    }
    else if (auto light = std::shared_ptr<Light>(dynamic_cast<Light*>(spatial)))
    {
        assert(!HasLight(light->getSpatialName()));
        std::lock_guard locker{ m_lightMapLock };
        m_lights.insert_or_assign(light->getSpatialName(), light);
        Frameworks::EventPublisher::post(std::make_shared<LightInfoCreated>(light));
        return light;
    }
    else
    {
        assert(false);
        return nullptr;
    }
}

std::shared_ptr<SceneQuadTreeRoot> SceneGraphRepository::createQuadTreeRoot(const std::string& name, const std::shared_ptr<LazyNode> root)
{
    assert(!hasQuadTreeRoot(name));
    auto quadTreeRoot = std::make_shared<SceneQuadTreeRoot>(name, root);
    std::lock_guard locker{ m_quadTreeRootMapLock };
    m_quadTreeRoots.insert_or_assign(name, quadTreeRoot);
    return quadTreeRoot;
}

bool SceneGraphRepository::hasQuadTreeRoot(const std::string& name)
{
    std::lock_guard locker{ m_quadTreeRootMapLock };
    auto it = m_quadTreeRoots.find(name);
    return ((it != m_quadTreeRoots.end()) && (!it->second.expired()));
}

std::shared_ptr<SceneQuadTreeRoot> SceneGraphRepository::queryQuadTreeRoot(const std::string& name)
{
    std::lock_guard locker{ m_quadTreeRootMapLock };
    auto it = m_quadTreeRoots.find(name);
    if (it == m_quadTreeRoots.end()) return nullptr;
    if (it->second.expired()) return nullptr;
    return it->second.lock();
}

void SceneGraphRepository::queryCamera(const IQueryPtr& q)
{
    if (!q) return;
    const auto query = std::dynamic_pointer_cast<QueryCamera>(q);
    assert(query);
    query->setResult(queryCamera(query->cameraName()));
}

void SceneGraphRepository::queryNode(const Frameworks::IQueryPtr& q)
{
    if (!q) return;
    const auto query = std::dynamic_pointer_cast<QueryNode>(q);
    assert(query);
    query->setResult(queryNode(query->nodeName()));
}

void SceneGraphRepository::queryQuadTreeRoot(const IQueryPtr& q)
{
    if (!q) return;
    const auto query = std::dynamic_pointer_cast<QuerySceneQuadTreeRoot>(q);
    assert(query);
    query->setResult(queryQuadTreeRoot(query->sceneQuadTreeRootName()));
}

void SceneGraphRepository::createCamera(const Frameworks::ICommandPtr& c)
{
    if (!c) return;
    if (const auto cmd = std::dynamic_pointer_cast<SceneGraph::CreateCamera>(c))
    {
        std::shared_ptr<Camera> camera = nullptr;
        if (hasCamera(cmd->GetDto().getName()))
        {
            camera = queryCamera(cmd->GetDto().getName());
        }
        else
        {
            camera = createCamera(cmd->GetDto());
        }
        if (camera)
        {
            EventPublisher::post(std::make_shared<SceneGraph::CameraCreated>(camera->getName(), camera));
        }
        else
        {
            EventPublisher::post(std::make_shared<SceneGraph::CreateCameraFailed>(cmd->GetDto().getName(), ErrorCode::sceneRepositoryFailed));
        }
    }
    else
    {
        assert(false);
    }
}

void SceneGraphRepository::createNode(const Frameworks::ICommandPtr& c)
{
    if (!c) return;
    const auto cmd = std::dynamic_pointer_cast<CreateNode>(c);
    if (!cmd) return;
    if (hasNode(cmd->name()))
    {
        EventPublisher::post(std::make_shared<CreateNodeFailed>(cmd->getRuid(), ErrorCode::entityAlreadyExists));
    }
    else
    {
        auto node = createNode(cmd->name(), *cmd->rtti());
        if (node)
        {
            EventPublisher::post(std::make_shared<NodeCreated>(cmd->getRuid(), node));
        }
        else
        {
            EventPublisher::post(std::make_shared<CreateNodeFailed>(cmd->getRuid(), ErrorCode::sceneRepositoryFailed));
        }
    }
}

void SceneGraphRepository::createQuadTreeRoot(const Frameworks::ICommandPtr& c)
{
    if (!c) return;
    const auto cmd = std::dynamic_pointer_cast<CreateSceneQuadTreeRoot>(c);
    if (!cmd) return;
    if (hasQuadTreeRoot(cmd->name()))
    {
        EventPublisher::post(std::make_shared<CreateSceneQuadTreeRootFailed>(cmd->getRuid(), ErrorCode::entityAlreadyExists));
    }
    else
    {
        auto quadTreeRoot = createQuadTreeRoot(cmd->name(), cmd->root());
        if (quadTreeRoot)
        {
            EventPublisher::post(std::make_shared<SceneQuadTreeRootCreated>(cmd->getRuid(), quadTreeRoot));
        }
        else
        {
            EventPublisher::post(std::make_shared<CreateSceneQuadTreeRootFailed>(cmd->getRuid(), ErrorCode::sceneRepositoryFailed));
        }
    }
}
