﻿#include "GeometryBuilder.h"
#include "GeometryDataEvents.h"
#include "GeometryRepository.h"
#include "GeometryDataPolicy.h"
#include "GeometryErrors.h"
#include "Frameworks/EventPublisher.h"
#include "Frameworks/EventSubscriber.h"
#include "Frameworks/CommandBus.h"
#include "GameEngine/DtoDeserializer.h"
#include "GameEngine/DtoEvents.h"
#include "GameEngine/FactoryCommands.h"
#include "Platforms/PlatformLayer.h"
#include <cassert>
#include <memory>

using namespace Enigma::Geometries;
using namespace Enigma::Engine;
using namespace Enigma::Frameworks;

/*GeometryBuilder::GeometryBuilder(GeometryRepository* host) : m_ruidDeserializing(), m_ruidInstancing()
{
    m_hostRepository = host;
    m_onDtoDeserialized = std::make_shared<EventSubscriber>([=](auto e) { this->OnDtoDeserialized(e); });
    EventPublisher::subscribe(typeid(GenericDtoDeserialized), m_onDtoDeserialized);
    m_onDeserializeDtoFailed = std::make_shared<EventSubscriber>([=](auto e) { this->OnDeserializeDtoFailed(e); });
    EventPublisher::subscribe(typeid(DeserializeDtoFailed), m_onDeserializeDtoFailed);
    //m_onFactoryGeometryCreated = std::make_shared<EventSubscriber>([=](auto e) { this->OnFactoryGeometryCreated(e); });
    //EventPublisher::subscribe(typeid(FactoryGeometryCreated), m_onFactoryGeometryCreated);

    //m_doRegisteringGeometryFactory =
      //  std::make_shared<CommandSubscriber>([=](auto c) { this->DoRegisteringGeometryFactory(c); });
    //CommandBus::subscribe(typeid(RegisterGeometryDtoFactory), m_doRegisteringGeometryFactory);
    //m_doUnRegisteringGeometryFactory =
      //  std::make_shared<CommandSubscriber>([=](auto c) { this->DoUnRegisteringGeometryFactory(c); });
    //CommandBus::subscribe(typeid(UnRegisterGeometryDtoFactory), m_doUnRegisteringGeometryFactory);
}

GeometryBuilder::~GeometryBuilder()
{
    EventPublisher::unsubscribe(typeid(GenericDtoDeserialized), m_onDtoDeserialized);
    m_onDtoDeserialized = nullptr;
    EventPublisher::unsubscribe(typeid(DeserializeDtoFailed), m_onDeserializeDtoFailed);
    m_onDeserializeDtoFailed = nullptr;
    //EventPublisher::unsubscribe(typeid(FactoryGeometryCreated), m_onFactoryGeometryCreated);
    //m_onFactoryGeometryCreated = nullptr;
    //CommandBus::unsubscribe(typeid(RegisterGeometryDtoFactory), m_doRegisteringGeometryFactory);
    //m_doRegisteringGeometryFactory = nullptr;
    //CommandBus::unsubscribe(typeid(UnRegisterGeometryDtoFactory), m_doUnRegisteringGeometryFactory);
    //m_doUnRegisteringGeometryFactory = nullptr;
}

void GeometryBuilder::BuildGeometry(const GeometryDataPolicy& policy)
{
    assert(m_hostRepository);
    m_policy = policy;
    /*if (m_hostRepository->hasGeometryData(policy.id()))
    {
        EventPublisher::post(std::make_shared<GeometryDataBuilt>(policy.id(),
            m_hostRepository->queryGeometryData(policy.id())));
    }
    else if (auto dto = policy.getDto())
    {
        CreateFromDto(policy.id(), dto.value());
    }
    else if (policy.getDeserializer())
    {
        m_ruidDeserializing = Ruid::generate();
        policy.getDeserializer()->InvokeDeserialize(m_ruidDeserializing, policy.parameter());
    }
    else
    {
        EventPublisher::post(std::make_shared<BuildGeometryDataFailed>(policy.id(), ErrorCode::policyIncomplete));
    }*/
    /*}

    void GeometryBuilder::CreateFromDto(const GeometryId& id, const Engine::GenericDto& dto)
    {
        assert(m_hostRepository);
        m_ruidInstancing = dto.ruid();
        GeometryFactory(id, dto);
        //CommandBus::post(std::make_shared<InvokeDtoFactory>(dto));
    }

    void GeometryBuilder::GeometryFactory(const GeometryId& id, const Engine::GenericDto& dto)
    {
        /*auto factory = m_factories.find(dto.GetRtti().GetRttiName());
        if (factory == m_factories.end())
        {
            Platforms::Debug::Printf("Can't find dto factory of %s\n", dto.GetRtti().GetRttiName().c_str());
            return;
        }
        EventPublisher::post(std::make_shared<FactoryGeometryCreated>(dto, factory->second(id, dto)));*/
        //}

        //void GeometryBuilder::OnDtoDeserialized(const Frameworks::IEventPtr& e)
        //{
            /*if (!e) return;
            auto ev = std::dynamic_pointer_cast<GenericDtoDeserialized, IEvent>(e);
            if (!ev) return;
            if (ev->getRuid() != m_ruidDeserializing) return;
            if (ev->GetDtos().empty())
            {
                EventPublisher::post(std::make_shared<BuildGeometryDataFailed>(m_policy.id(), ErrorCode::deserializeFail));
                return;
            }
            CreateFromDto(m_policy.id(), ev->GetDtos()[0]);*/
            //}

            //void GeometryBuilder::OnDeserializeDtoFailed(const Frameworks::IEventPtr& e)
            //{
                /*if (!e) return;
                auto ev = std::dynamic_pointer_cast<DeserializeDtoFailed, IEvent>(e);
                if (!ev) return;
                if (ev->getRuid() != m_ruidDeserializing) return;
                EventPublisher::post(std::make_shared<BuildGeometryDataFailed>(m_policy.id(), ev->GetErrorCode()));*/
                //}

                //void GeometryBuilder::OnFactoryGeometryCreated(const Frameworks::IEventPtr& e)
                //{
                    /*if (!e) return;
                    auto ev = std::dynamic_pointer_cast<FactoryGeometryCreated, IEvent>(e);
                    if (!ev) return;
                    if (ev->GetDto().ruid() != m_ruidInstancing) return;
                    EventPublisher::post(std::make_shared<GeometryDataBuilt>(m_policy.id(), ev->GetGeometryData()));*/
                    //}

                    //void GeometryBuilder::DoRegisteringGeometryFactory(const Frameworks::ICommandPtr& c)
                    //{
                        /*if (!c) return;
                        auto cmd = std::dynamic_pointer_cast<RegisterGeometryDtoFactory, ICommand>(c);
                        if (!cmd) return;
                        m_factories.insert_or_assign(cmd->GetRtti(), cmd->GetFactory());*/
                        //}

                        //void GeometryBuilder::DoUnRegisteringGeometryFactory(const Frameworks::ICommandPtr& c)
                        //{
                            /*if (!c) return;
                            auto cmd = std::dynamic_pointer_cast<UnRegisterGeometryDtoFactory, ICommand>(c);
                            if (!cmd) return;
                            if (m_factories.find(cmd->GetRtti()) != m_factories.end()) m_factories.erase(cmd->GetRtti());*/
                            //}
