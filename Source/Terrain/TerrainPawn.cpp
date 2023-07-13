﻿#include "TerrainPawn.h"
#include "TerrainPawnDto.h"

using namespace Enigma::Terrain;
using namespace Enigma::SceneGraph;
using namespace Enigma::Engine;

DEFINE_RTTI(Terrain, TerrainPawn, Pawn);

TerrainPawn::TerrainPawn(const std::string& name) : Pawn(name)
{

}

TerrainPawn::TerrainPawn(const GenericDto& o) : Pawn(o)
{

}

TerrainPawn::~TerrainPawn()
{

}

GenericDto TerrainPawn::SerializeDto()
{
    TerrainPawnDto dto(SerializePawnDto());
    return dto.ToGenericDto();
}
