﻿#include "AnimatedPawnDto.h"
#include "AnimatedPawn.h"
#include <cassert>

using namespace Enigma::GameCommon;
using namespace Enigma::SceneGraph;

static std::string TOKEN_ANIMATION_CLIP_MAP = "AnimClipMap";
static std::string TOKEN_AVATAR_RECIPES = "AvatarRecipes";

AnimatedPawnDto::AnimatedPawnDto() : PawnDto()
{
    m_factoryDesc = Engine::FactoryDesc(AnimatedPawn::TYPE_RTTI.getName());
}

AnimatedPawnDto::AnimatedPawnDto(const SceneGraph::PawnDto& dto) : PawnDto(dto)
{
    assert(Frameworks::Rtti::isExactlyOrDerivedFrom(m_factoryDesc.GetRttiName(), AnimatedPawn::TYPE_RTTI.getName()));
}

AnimatedPawnDto AnimatedPawnDto::FromGenericDto(const Engine::GenericDto& dto)
{
    AnimatedPawnDto pawn_dto(PawnDto::FromGenericDto(dto));
    if (auto v = dto.TryGetValue<Engine::GenericDto>(TOKEN_ANIMATION_CLIP_MAP)) pawn_dto.TheAnimationClipMapDto() = v.value();
    if (auto v = dto.TryGetValue<Engine::GenericDtoCollection>(TOKEN_AVATAR_RECIPES)) pawn_dto.AvatarRecipeDtos() = v.value();
    return pawn_dto;
}

Enigma::Engine::GenericDto AnimatedPawnDto::ToGenericDto() const
{
    Engine::GenericDto dto = PawnDto::ToGenericDto();
    if (m_animationClipMapDto) dto.AddOrUpdate(TOKEN_ANIMATION_CLIP_MAP, m_animationClipMapDto.value());
    dto.AddOrUpdate(TOKEN_AVATAR_RECIPES, m_avatarRecipeDtos);
    return dto;
}
