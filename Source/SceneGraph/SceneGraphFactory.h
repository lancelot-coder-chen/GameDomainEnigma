﻿/*********************************************************************
 * \file   SceneGraphFactory.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   November 2023
 *********************************************************************/
#ifndef SCENE_GRAPH_FACTORY_H
#define SCENE_GRAPH_FACTORY_H

#include "SpatialId.h"
#include "Frameworks/EventSubscriber.h"
#include "Frameworks/CommandSubscriber.h"
#include "GameEngine/GenericDto.h"
#include <mutex>

namespace Enigma::SceneGraph
{
    class Camera;
    class Spatial;
    using SpatialCreator = std::function<std::shared_ptr<Spatial>(const SpatialId& id)>;
    using SpatialConstitutor = std::function<std::shared_ptr<Spatial>(const SpatialId& id, const Engine::GenericDto& dto)>;

    class SceneGraphFactory
    {
    public:
        SceneGraphFactory();
        ~SceneGraphFactory();

        void registerHandlers();
        void unregisterHandlers();

        std::shared_ptr<Camera> createCamera(const SpatialId& id);
        std::shared_ptr<Camera> constituteCamera(const SpatialId& id, const Engine::GenericDto& dto, bool is_persisted);
        std::shared_ptr<Spatial> createSpatial(const SpatialId& id);
        std::shared_ptr<Spatial> constituteSpatial(const SpatialId& id, const Engine::GenericDto& dto, bool is_persisted);

        void registerSpatialFactory(const std::string& rtti, const SpatialCreator& creator, const SpatialConstitutor& constitutor);
        void unregisterSpatialFactory(const std::string& rtti);

    protected:
        void registerSpatialFactory(const Frameworks::ICommandPtr& c);
        void unregisterSpatialFactory(const Frameworks::ICommandPtr& c);
        void createCamera(const Frameworks::ICommandPtr& c);
        void constituteCamera(const Frameworks::ICommandPtr& c);
        void createSpatial(const Frameworks::ICommandPtr& c);
        void constituteSpatial(const Frameworks::ICommandPtr& c);

    protected:
        std::unordered_map<std::string, SpatialCreator> m_creators; // rtti name -> creator
        std::unordered_map<std::string, SpatialConstitutor> m_constitutors; // rtti name -> constitutor

        Frameworks::CommandSubscriberPtr m_registerSpatialFactory;
        Frameworks::CommandSubscriberPtr m_unregisterSpatialFactory;
        Frameworks::CommandSubscriberPtr m_createCamera;
        Frameworks::CommandSubscriberPtr m_constituteCamera;
        Frameworks::CommandSubscriberPtr m_createSpatial;
        Frameworks::CommandSubscriberPtr m_constituteSpatial;
    };
}

#endif // SCENE_GRAPH_FACTORY_H
