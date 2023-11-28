﻿#include "QuadTreeRootDto.h"
#include "SceneQuadTreeRoot.h"

using namespace Enigma::WorldMap;
using namespace Enigma::Engine;

static std::string TOKEN_NAME = "Name";
static std::string TOKEN_ROOT = "Root";

QuadTreeRootDto::QuadTreeRootDto() : m_factoryDesc(SceneQuadTreeRoot::TYPE_RTTI.getName())
{
}

QuadTreeRootDto QuadTreeRootDto::fromGenericDto(const Engine::GenericDto& dto)
{
    QuadTreeRootDto quad_dto;
    quad_dto.factoryDesc() = dto.GetRtti();
    if (auto v = dto.TryGetValue<std::string>(TOKEN_NAME)) quad_dto.name() = v.value();
    if (auto v = dto.TryGetValue<GenericDto>(TOKEN_ROOT)) quad_dto.root() = v.value();
    return quad_dto;
}

GenericDto QuadTreeRootDto::toGenericDto() const
{
    GenericDto dto;
    dto.AddRtti(m_factoryDesc);
    dto.AddOrUpdate(TOKEN_NAME, name());
    dto.AddOrUpdate(TOKEN_ROOT, root());
    return dto;
}
