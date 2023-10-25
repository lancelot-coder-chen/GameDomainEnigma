﻿#include "VisibilityManagedNode.h"
#include "SceneGraphDtos.h"
#include "SceneGraphCommands.h"
#include "Frameworks/CommandBus.h"
#include "Frameworks/EventPublisher.h"
#include "SceneGraphErrors.h"
#include "SceneGraphEvents.h"
#include "Culler.h"

using namespace Enigma::SceneGraph;
using namespace Enigma::Engine;
using namespace Enigma::Frameworks;

DEFINE_RTTI(SceneGraph, VisibilityManagedNode, LazyNode);

VisibilityManagedNode::VisibilityManagedNode(const std::string& name)
    : LazyNode(name)
{
    m_factoryDesc = Engine::FactoryDesc(VisibilityManagedNode::TYPE_RTTI.getName());
}

VisibilityManagedNode::VisibilityManagedNode(const GenericDto& dto)
    : LazyNode(dto)
{
}

VisibilityManagedNode::~VisibilityManagedNode()
{

}

GenericDto VisibilityManagedNode::SerializeDto()
{
    return LazyNode::SerializeDto();
}

error VisibilityManagedNode::OnCullingVisible(Culler* culler, bool noCull)
{
    // 需要讀取
    if (m_lazyStatus.isGhost())
    {
        CommandBus::post(std::make_shared<InstanceLazyNode>(std::dynamic_pointer_cast<LazyNode, Spatial>(shared_from_this())));
        return ErrorCode::ok;
    }
    if (!m_lazyStatus.isReady())
    {
        return ErrorCode::dataNotReady;
    }
    EventPublisher::post(std::make_shared<VisibilityChanged>(std::dynamic_pointer_cast<LazyNode, Spatial>(shared_from_this()), true));
    return Node::OnCullingVisible(culler, noCull);
}

void VisibilityManagedNode::OnCullingCompleteNotVisible(Culler* culler)
{
    if (!m_lazyStatus.isReady()) return;

    // let me check first
    if (!culler) return;
    if (!culler->IsOuterClippingEnable()) return;
    EventPublisher::post(std::make_shared<VisibilityChanged>(std::dynamic_pointer_cast<LazyNode, Spatial>(shared_from_this()), false));
}

void VisibilityManagedNode::ReleaseDeferredContent()
{
    while (!m_childList.empty())
    {
        DetachChild(*(m_childList.begin()));
    }

    m_lazyStatus.changeStatus(LazyStatus::Status::Ghost);
}
