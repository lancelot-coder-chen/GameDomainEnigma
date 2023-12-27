﻿#include "EffectTextureMapDto.h"
#include "GameEngine/TexturePolicies.h"

using namespace Enigma::Engine;

static std::string TOKEN_TEXTURE_ID_NAME = "TextureId.Name";
static std::string TOKEN_SEMANTIC = "Semantic";
static std::string TOKEN_ARRAY_INDEX = "ArrayIndex";
static std::string TOKEN_TEXTURE_MAPPINGS = "TextureMappings";

TextureMappingDto TextureMappingDto::fromGenericDto(const GenericDto& dto)
{
    TextureMappingDto tex;
    if (const auto v = dto.TryGetValue<std::string>(TOKEN_TEXTURE_ID_NAME)) tex.textureId() = v.value();
    if (const auto v = dto.TryGetValue<std::string>(TOKEN_SEMANTIC)) tex.semantic() = v.value();
    if (const auto v = dto.TryGetValue<unsigned>(TOKEN_ARRAY_INDEX)) tex.arrayIndex() = v.value();
    return tex;
}

GenericDto TextureMappingDto::toGenericDto() const
{
    GenericDto dto;
    dto.AddOrUpdate(TOKEN_TEXTURE_ID_NAME, m_textureId.name());
    dto.AddOrUpdate(TOKEN_SEMANTIC, m_semantic);
    if (m_arrayIndex)
    {
        dto.AddOrUpdate(TOKEN_ARRAY_INDEX, m_arrayIndex.value());
    }
    return dto;
}

EffectTextureMapDto EffectTextureMapDto::fromGenericDto(const GenericDto& dto)
{
    EffectTextureMapDto effect;
    if (const auto v = dto.TryGetValue<GenericDtoCollection>(TOKEN_TEXTURE_MAPPINGS))
    {
        for (auto& mapping_dto : v.value())
        {
            effect.textureMappings().emplace_back(TextureMappingDto::fromGenericDto(mapping_dto));
        }
    }
    return effect;
}

GenericDto EffectTextureMapDto::toGenericDto()
{
    GenericDtoCollection mappings;
    for (auto& tex : m_textureMappings)
    {
        mappings.emplace_back(tex.toGenericDto());
    }
    GenericDto dto;
    dto.AddOrUpdate(TOKEN_TEXTURE_MAPPINGS, mappings);
    return dto;
}
