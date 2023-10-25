﻿/*********************************************************************
 * \file   WorldMapEvents.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   July 2023
 *********************************************************************/
#ifndef WORLD_MAP_EVENTS_H
#define WORLD_MAP_EVENTS_H

#include "Frameworks/Event.h"

namespace Enigma::WorldMap
{
    class WorldMap;
    class WorldMapCreated : public Frameworks::IEvent
    {
    public:
        WorldMapCreated(const std::string& name, std::shared_ptr<WorldMap> world) : m_name(name), m_world(world) {}

        const std::string& getName() const { return m_name; }
        const std::shared_ptr<WorldMap>& getWorld() const { return m_world; }

    protected:
        std::string m_name;
        std::shared_ptr<WorldMap> m_world;
    };
    class WorldMapDeserialized : public Frameworks::IEvent
    {
    public:
        WorldMapDeserialized(const std::string& name, std::shared_ptr<WorldMap> world) : m_name(name), m_world(world) {}

        const std::string& getName() const { return m_name; }
        const std::shared_ptr<WorldMap>& getWorld() const { return m_world; }

    protected:
        std::string m_name;
        std::shared_ptr<WorldMap> m_world;
    };
}

#endif // WORLD_MAP_EVENTS_H
