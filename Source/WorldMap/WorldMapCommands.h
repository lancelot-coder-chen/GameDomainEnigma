﻿/*********************************************************************
 * \file   WorldMapCommands.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   July 2023
 *********************************************************************/
#ifndef WORLD_MAP_COMMANDS_H
#define WORLD_MAP_COMMANDS_H

#include "Frameworks/Command.h"
#include "GameEngine/GenericDto.h"
#include "Terrain/TerrainPawn.h"
#include "MathLib/Matrix4.h"

namespace Enigma::WorldMap
{
    class CreateEmptyWorldMap : public Frameworks::ICommand
    {
    public:
        CreateEmptyWorldMap(const Engine::GenericDto& dto) : m_dto(dto) {}

        const Engine::GenericDto& GetDto() const { return m_dto; }

    protected:
        Engine::GenericDto m_dto;
    };
    class AttachTerrainToWorldMap : public Frameworks::ICommand
    {
    public:
        AttachTerrainToWorldMap(const std::shared_ptr<Terrain::TerrainPawn>& terrain, const MathLib::Matrix4& local_transform) : m_terrain(terrain), m_local(local_transform) {}

        const std::shared_ptr<Terrain::TerrainPawn>& GetTerrain() const { return m_terrain; }
        const MathLib::Matrix4& GetLocalTransform() const { return m_local; }

    protected:
        std::shared_ptr<Terrain::TerrainPawn> m_terrain;
        MathLib::Matrix4 m_local;
    };
}

#endif // WORLD_MAP_COMMANDS_H
