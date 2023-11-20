﻿#include "WorldMap.h"
#include "SceneGraph/PortalManagementNode.h"
#include "SceneGraph/PortalZoneNode.h"
#include "WorldMapDto.h"
#include "SceneQuadTreeRoot.h"
#include "SceneGraph/VisibilityManagedNode.h"
#include "SceneGraph/EnumNonDerivedSpatials.h"
#include "Frameworks/LazyStatus.h"
#include "Platforms/PlatformLayer.h"
#include <cassert>

using namespace Enigma::WorldMap;
using namespace Enigma::SceneGraph;
using namespace Enigma::MathLib;
using namespace Enigma::Engine;
using namespace Enigma::Terrain;
using namespace Enigma::Frameworks;

DEFINE_RTTI(WorldMap, WorldMap, PortalZoneNode);

std::string QUADROOT_POSTFIX = "_qroot";

WorldMap::WorldMap(const std::string& name) : PortalZoneNode(name)
{
    m_factoryDesc = FactoryDesc(WorldMap::TYPE_RTTI.getName()).ClaimAsInstanced(name + ".wld");
}

WorldMap::WorldMap(const Engine::GenericDto& o) : PortalZoneNode(o)
{
    //WorldMapDto world = WorldMapDto::FromGenericDto(o);
    //m_factoryDesc = FactoryDesc(WorldMap::TYPE_RTTI.getName()).ClaimAsInstanced(world.Name() + ".wld");
}

WorldMap::~WorldMap()
{
    m_listQuadRoot.clear();
}

GenericDto WorldMap::serializeDto()
{
    WorldMapDto world_dto(PortalZoneNodeDto::FromGenericDto(PortalZoneNode::serializeDto()));
    world_dto.IsTopLevel() = true;

    for (auto& root : m_listQuadRoot)
    {
        if (!root) continue;
        world_dto.quadTreeRoots().emplace_back(root->serializeDto());
    }
    for (const auto& child : GetChildList())
    {
        if (!child) continue;
        EnumNonDerivedSpatials enumSpatials(LazyNode::TYPE_RTTI);
        child->visitBy(&enumSpatials);
        for (const auto& spatial : enumSpatials.GetSpatials())
        {
            if (!spatial) continue;
            world_dto.nonLazyChildren().emplace_back(spatial->serializeDto());
        }
    }
    return world_dto.toGenericDto();
}

std::vector<GenericDtoCollection> WorldMap::serializeQuadGraphs()
{
    std::vector<Engine::GenericDtoCollection> collections;
    if (m_listQuadRoot.empty()) return collections;
    for (auto& root : m_listQuadRoot)
    {
        if (!root) continue;
        auto root_collection = root->serializeTreeGraphs();
        collections.insert(collections.end(), root_collection.begin(), root_collection.end());
    }
    return collections;
}

void WorldMap::setPortalRootNode(const std::shared_ptr<PortalManagementNode>& root)
{
    assert(root);
    m_portalRootNode = root;
}

void WorldMap::attachTerrain(const std::shared_ptr<SceneGraph::SceneGraphRepository>& repository, const std::shared_ptr<Terrain::TerrainPawn>& terrain, const MathLib::Matrix4& local_transform)
{
    assert(terrain);
    assert(repository);
    std::string node_name = terrain->getSpatialName() + QUADROOT_POSTFIX; // +NODE_FILE_EXT;
    auto quadRootNode = std::dynamic_pointer_cast<VisibilityManagedNode, Node>(repository->createNode(node_name, VisibilityManagedNode::TYPE_RTTI));
    quadRootNode->lazyStatus().changeStatus(LazyStatus::Status::Ready);
    quadRootNode->factoryDesc().ClaimAsInstanced(node_name + ".node");
    quadRootNode->AttachChild(terrain, Matrix4::IDENTITY);
    AttachChild(quadRootNode, local_transform);
    auto treeRoot = std::make_shared<SceneQuadTreeRoot>(node_name, quadRootNode);
    m_listQuadRoot.push_back(treeRoot);
}

std::shared_ptr<Node> WorldMap::queryFittingNode(const Engine::BoundingVolume& bv_in_world) const
{
    if (m_listQuadRoot.empty()) return getPortalRootNode();
    for (auto root : m_listQuadRoot)
    {
        if (!root) continue;
        if (auto node = root->queryFittingNode(bv_in_world)) return node;
    }
    return getPortalRootNode();
}
