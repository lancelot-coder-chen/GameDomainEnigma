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
    m_doCreatingCamera = std::make_shared<CommandSubscriber>([=](auto c) { DoCreatingCamera(c); });
    CommandBus::subscribe(typeid(SceneGraph::CreateCamera), m_doCreatingCamera);

    return ServiceResult::Complete;
}
ServiceResult SceneGraphRepository::onTerm()
{
    QueryDispatcher::unsubscribe(typeid(QueryCamera), m_queryCamera);
    m_queryCamera = nullptr;
    CommandBus::unsubscribe(typeid(SceneGraph::CreateCamera), m_doCreatingCamera);
    m_doCreatingCamera = nullptr;

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

std::shared_ptr<Camera> SceneGraphRepository::CreateCamera(const std::string& name)
{
    assert(!hasCamera(name));
    auto camera = std::make_shared<Camera>(name, m_handSystem);
    std::lock_guard locker{ m_cameraMapLock };
    m_cameras.insert_or_assign(name, camera);
    return camera;
}

std::shared_ptr<Camera> SceneGraphRepository::CreateCamera(const GenericDto& dto)
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

std::shared_ptr<Node> SceneGraphRepository::CreateNode(const std::string& name, const Rtti& rtti)
{
    assert(!HasNode(name));
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

bool SceneGraphRepository::HasNode(const std::string& name)
{
    std::lock_guard locker{ m_nodeMapLock };
    auto it = m_nodes.find(name);
    return ((it != m_nodes.end()) && (!it->second.expired()));
}

std::shared_ptr<Node> SceneGraphRepository::QueryNode(const std::string& name)
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
    if (auto node = QueryNode(name)) return node;
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
        assert(!HasNode(node->getSpatialName()));
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

void SceneGraphRepository::DoCreatingCamera(const Frameworks::ICommandPtr& c)
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
            camera = CreateCamera(cmd->GetDto());
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
