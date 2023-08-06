﻿/*********************************************************************
 * \file   LazyNode.h
 * \brief  延遲載入的 Scene Graph Node, 先建立空間相關數據, 等可見時才載入內容
 * VisibilityManagedNode, PortalZoneNode 繼承自此
 * \author Lancelot 'Robin' Chen
 * \date   February 2023
 *********************************************************************/
#ifndef _LAZY_NODE_H
#define _LAZY_NODE_H

#include "Node.h"
#include "Frameworks/LazyStatus.h"

namespace Enigma::SceneGraph
{
    class LazyNodeDto;

    class LazyNode : public Node
    {
        DECLARE_EN_RTTI;
    public:
        LazyNode(const std::string& name);
        LazyNode(const Engine::GenericDto& dto);
        LazyNode(const LazyNode&) = delete;
        LazyNode(LazyNode&&) = delete;
        virtual ~LazyNode() override;
        LazyNode& operator=(const LazyNode&) = delete;
        LazyNode& operator=(LazyNode&&) = delete;

        virtual Engine::GenericDto SerializeDto() override;
        virtual Engine::GenericDto SerializeAsLaziness();

        virtual bool CanVisited() override;

        virtual SceneTraveler::TravelResult VisitBy(SceneTraveler* traveler) override;

        const Frameworks::LazyStatus& TheLazyStatus() const { return m_lazyStatus; }
        Frameworks::LazyStatus& TheLazyStatus() { return m_lazyStatus; }

    protected:
        Frameworks::LazyStatus m_lazyStatus;
    };
}

#endif // _LAZY_NODE_H
