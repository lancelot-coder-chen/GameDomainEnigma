﻿#include "WorldMap.h"
#include "SceneGraph/PortalManagementNode.h"
#include "SceneGraph/PortalZoneNode.h"
#include "WorldMapDto.h"
#include <cassert>

using namespace Enigma::WorldMap;
using namespace Enigma::SceneGraph;
using namespace Enigma::MathLib;
using namespace Enigma::Engine;

DEFINE_RTTI(WorldMap, WorldMap, PortalZoneNode);

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
}

GenericDto WorldMap::serializeDto()
{
    return PortalZoneNode::serializeDto();
}

void WorldMap::setPortalRootNode(const std::shared_ptr<PortalManagementNode>& root)
{
    assert(root);
    m_portalRootNode = root;
}
