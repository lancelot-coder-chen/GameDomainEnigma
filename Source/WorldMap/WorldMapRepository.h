﻿/*****************************************************************
 * \file   WorldMapRepository.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   April 2024
 ******************************************************************/
#ifndef WORLD_MAP_REPOSITORY_H
#define WORLD_MAP_REPOSITORY_H

#include "QuadTreeRootId.h"
#include "Frameworks/SystemService.h"
#include <memory>
#include <unordered_map>
#include <mutex>

namespace Enigma::WorldMap
{
    class QuadTreeRoot;
    class WorldMapStoreMapper;

    class WorldMapRepository : public Frameworks::ISystemService
    {
        DECLARE_EN_RTTI;
    public:
        WorldMapRepository(Frameworks::ServiceManager* srv_manager, const std::shared_ptr<WorldMapStoreMapper>& world_map_store_mapper);
        WorldMapRepository(const WorldMapRepository&) = delete;
        WorldMapRepository(WorldMapRepository&&) = delete;
        WorldMapRepository& operator=(const WorldMapRepository&) = delete;
        WorldMapRepository& operator=(WorldMapRepository&&) = delete;
        ~WorldMapRepository() override;

        bool hasQuadTreeRoot(const QuadTreeRootId& id);
        std::shared_ptr<QuadTreeRoot> queryQuadTreeRoot(const QuadTreeRootId& id);

    protected:
        std::shared_ptr<WorldMapStoreMapper> m_storeMapper;
        std::unordered_map<QuadTreeRootId, std::shared_ptr<QuadTreeRoot>, QuadTreeRootId::hash> m_quadTreeRoots;
        std::recursive_mutex m_quadTreeRootLock;
    };
}

#endif // WORLD_MAP_REPOSITORY_H