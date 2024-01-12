﻿#include "LightingPawnDto.h"
#include "LightingPawn.h"

using namespace Enigma::GameCommon;
using namespace Enigma::SceneGraph;

static std::string TOKEN_HOST_LIGHT_NAME = "HostLightName";

LightingPawnDto::LightingPawnDto() : PawnDto()
{
    m_factoryDesc = Engine::FactoryDesc(LightingPawn::TYPE_RTTI.getName());
}

LightingPawnDto::LightingPawnDto(const SceneGraph::PawnDto& dto) : PawnDto(dto)
{
    assert(Frameworks::Rtti::isExactlyOrDerivedFrom(m_factoryDesc.GetRttiName(), LightingPawn::TYPE_RTTI.getName()));
}

LightingPawnDto LightingPawnDto::fromGenericDto(const Engine::GenericDto& dto)
{
    LightingPawnDto pawn_dto{ PawnDto(dto) };
    if (auto v = dto.tryGetValue<std::string>(TOKEN_HOST_LIGHT_NAME)) pawn_dto.HostLightName() = v.value();
    return pawn_dto;
}

Enigma::Engine::GenericDto LightingPawnDto::toGenericDto() const
{
    Engine::GenericDto dto = PawnDto::toGenericDto();
    if (!m_hostLightName.empty()) dto.addOrUpdate(TOKEN_HOST_LIGHT_NAME, m_hostLightName);
    return dto;
}


