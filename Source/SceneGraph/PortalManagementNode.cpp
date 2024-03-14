﻿#include "PortalManagementNode.h"
#include "PortalDtos.h"
#include "SceneGraphErrors.h"
#include "PortalZoneNode.h"
#include "Culler.h"
#include "Camera.h"
#include "ContainingPortalZoneFinder.h"
#include "GameEngine/LinkageResolver.h"
#include "Platforms/PlatformLayer.h"
#include "PortalCommands.h"
#include "Frameworks/CommandBus.h"

using namespace Enigma::SceneGraph;
using namespace Enigma::MathLib;
using namespace Enigma::Engine;

DEFINE_RTTI(SceneGraph, PortalManagementNode, Node);

PortalManagementNode::PortalManagementNode(const SpatialId& id) : Node(id)
{
    m_factoryDesc = FactoryDesc(TYPE_RTTI.getName());
    m_attachOutsideZone = std::make_shared<Frameworks::CommandSubscriber>([=](const Frameworks::ICommandPtr& c) { attachOutsideZone(c); });
    Frameworks::CommandBus::subscribe(typeid(AttachPortalOutsideZone), m_attachOutsideZone);
}

PortalManagementNode::PortalManagementNode(const SpatialId& id, const Engine::GenericDto& o) : Node(id, o)
{
    PortalManagementNodeDto dto{ o };
    m_outsideZoneId = dto.outsideZoneNodeId();
    m_attachOutsideZone = std::make_shared<Frameworks::CommandSubscriber>([=](const Frameworks::ICommandPtr& c) { attachOutsideZone(c); });
    Frameworks::CommandBus::subscribe(typeid(AttachPortalOutsideZone), m_attachOutsideZone);
}

/*PortalManagementNode::PortalManagementNode(const std::string& name) : Node(name)
{
    m_factoryDesc = FactoryDesc(TYPE_RTTI.getName());
    m_attachOutsideZone = std::make_shared<Frameworks::CommandSubscriber>([=](const Frameworks::ICommandPtr& c) { attachOutsideZone(c); });
    Frameworks::CommandBus::subscribe(typeid(AttachPortalOutsideZone), m_attachOutsideZone);
}

PortalManagementNode::PortalManagementNode(const GenericDto& dto) : Node(dto)
{
    m_attachOutsideZone = std::make_shared<Frameworks::CommandSubscriber>([=](const Frameworks::ICommandPtr& c) { attachOutsideZone(c); });
    Frameworks::CommandBus::subscribe(typeid(AttachPortalOutsideZone), m_attachOutsideZone);
}*/

PortalManagementNode::~PortalManagementNode()
{
    Frameworks::CommandBus::unsubscribe(typeid(AttachPortalOutsideZone), m_attachOutsideZone);
    m_attachOutsideZone = nullptr;
}

std::shared_ptr<PortalManagementNode> PortalManagementNode::create(const SpatialId& id)
{
    return std::make_shared<PortalManagementNode>(id);
}

std::shared_ptr<PortalManagementNode> PortalManagementNode::constitute(const SpatialId& id, const Engine::GenericDto& dto)
{
    return std::make_shared<PortalManagementNode>(id, dto);
}

GenericDto PortalManagementNode::serializeDto()
{
    PortalManagementNodeDto dto(serializeNodeDto());
    dto.outsideZoneNodeId() = m_outsideZoneId;
    return dto.toGenericDto();
}

/*void PortalManagementNode::resolveFactoryLinkage(const GenericDto& dto, FactoryLinkageResolver<Spatial>& resolver)
{
    PortalManagementNodeDto nodeDto{ dto };
    resolver.tryResolveLinkage(nodeDto.outsideZoneNodeId().name(), [lifetime = weak_from_this()](auto sp)
        {
            if (!lifetime.expired())
                std::dynamic_pointer_cast<PortalManagementNode, Spatial>(lifetime.lock())->
                attachOutsideZone(std::dynamic_pointer_cast<PortalZoneNode, Spatial>(sp));
        });
}*/

void PortalManagementNode::attachOutsideZone(const std::shared_ptr<PortalZoneNode>& node)
{
    if (!node) return;
    m_outsideZoneId = node->id();
    node->setPortalParent(m_id);
}

error PortalManagementNode::onCullingVisible(Culler* culler, bool noCull)
{
    if (FATAL_LOG_EXPR((!culler) || (!culler->GetCamera()))) return ErrorCode::nullCullerCamera;

    error er = ErrorCode::ok;
    if (!noCull)
    {
        culler->Insert(thisSpatial());
        PortalZoneNodePtr startZone;
        Vector3 camPos = culler->GetCamera()->location();
        if ((m_cachedStartZone.lock()) && (m_cachedStartZone.lock()->getWorldBound().PointInside(camPos)))
        {
            startZone = m_cachedStartZone.lock();
        }
        else
        {
            ContainingPortalZoneFinder zone_finder(camPos);
            //CSceneTraveler::TravelResult result=region_finder.travelTo(this);
            SceneTraveler::TravelResult result = visitBy(&zone_finder);
            if (result == SceneTraveler::TravelResult::InterruptError) return ErrorCode::ok;

            startZone = zone_finder.GetContainingZone();
            if (startZone) m_cachedStartZone = startZone;
        }
        if (!startZone) startZone = std::dynamic_pointer_cast<PortalZoneNode>(Node::queryNode(m_outsideZoneId));
        if (startZone)
        {
            er = startZone->cullVisibleSet(culler, noCull);
            if (er) return er;
        }
    }
    else
    {
        er = Node::onCullingVisible(culler, noCull);
    }
    return er;
}

void PortalManagementNode::attachOutsideZone(const Frameworks::ICommandPtr& c)
{
    if (!c) return;
    const auto cmd = std::dynamic_pointer_cast<AttachPortalOutsideZone, Frameworks::ICommand>(c);
    if (!cmd) return;
    attachChild(cmd->GetZone(), Matrix4::IDENTITY);
    attachOutsideZone(cmd->GetZone());
}
