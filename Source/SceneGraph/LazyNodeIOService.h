﻿/*********************************************************************
 * \file   LazyNodeIOService.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   February 2023
 *********************************************************************/
#ifndef _LAZY_NODE_IO_SERVICE_H
#define _LAZY_NODE_IO_SERVICE_H

#include "Frameworks/SystemService.h"
#include "Frameworks/CommandSubscriber.h"
#include "GameEngine/DtoDeserializer.h"
#include "Frameworks/EventSubscriber.h"
#include <memory>
#include <deque>
#include <mutex>

namespace Enigma::SceneGraph
{
    class LazyNode;
    class LazyNodeIOService : public Frameworks::ISystemService
    {
        DECLARE_EN_RTTI;
    public:
        LazyNodeIOService(Frameworks::ServiceManager* mngr, const std::shared_ptr<Engine::IDtoDeserializer>& dto_deserializer);
        LazyNodeIOService(const LazyNodeIOService&) = delete;
        LazyNodeIOService(LazyNodeIOService&&) = delete;
        virtual ~LazyNodeIOService() override;
        LazyNodeIOService& operator=(const LazyNodeIOService&) = delete;
        LazyNodeIOService& operator=(LazyNodeIOService&&) = delete;

        virtual Frameworks::ServiceResult OnTick() override;

    private:
        void DoInstancingLazyNode(const Frameworks::ICommandPtr& c);
        void OnDtoDeserialized(const Frameworks::IEventPtr& e);
        void OnDeserializingDtoFailed(const Frameworks::IEventPtr& e);
        void OnInPlaceSceneGraphBuilt(const Frameworks::IEventPtr& e);

        void InstanceNextLazyNode();

    private:
        Frameworks::CommandSubscriberPtr m_doInstancingLazyNode;
        std::shared_ptr<Engine::IDtoDeserializer> m_dtoDeserializer;

        Frameworks::Ruid m_ruidDeserializing;
        std::shared_ptr<LazyNode> m_in_placeNode;

        Frameworks::EventSubscriberPtr m_onDtoDeserialized;
        Frameworks::EventSubscriberPtr m_onDeserializingDtoFailed;
        Frameworks::EventSubscriberPtr m_onInPlaceSceneGraphBuilt;

        std::deque<Frameworks::ICommandPtr> m_InstanceCommands;
        std::mutex m_InstanceCommandsLock;
        std::atomic_bool m_isCurrentInstancing;
    };
}

#endif // _LAZY_NODE_IO_SERVICE_H
