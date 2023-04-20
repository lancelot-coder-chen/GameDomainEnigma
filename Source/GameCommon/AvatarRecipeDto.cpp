﻿#include "AvatarRecipeDto.h"
#include "AvatarRecipes.h"

using namespace Enigma::GameCommon;
using namespace Enigma::Engine;

static std::string TOKEN_OLD_MATERIAL_NAME = "OldMaterialName";
static std::string TOKEN_NEW_MATERIAL_DTO = "NewMaterialDto";
static std::string TOKEN_MESH_NAME = "MeshName";
static std::string TOKEN_TEXTURE_MAPPING_DTO = "TextureMappingDto";

AvatarRecipeReplaceMaterialDto AvatarRecipeReplaceMaterialDto::FromGenericDto(const Engine::GenericDto& dto)
{
    AvatarRecipeReplaceMaterialDto recipe;
    if (auto v = dto.TryGetValue<std::string>(TOKEN_OLD_MATERIAL_NAME)) recipe.OldMaterialName() = v.value();
    if (auto v = dto.TryGetValue<GenericDto>(TOKEN_NEW_MATERIAL_DTO)) recipe.NewMaterialDto() = EffectMaterialDto::FromGenericDto(v.value());
    return recipe;
}

GenericDto AvatarRecipeReplaceMaterialDto::ToGenericDto() const
{
    GenericDto dto;
    dto.AddRtti(FactoryDesc(ReplaceAvatarMaterial::TYPE_RTTI.GetName()));
    dto.AddOrUpdate(TOKEN_OLD_MATERIAL_NAME, m_oldMaterialName);
    dto.AddOrUpdate(TOKEN_NEW_MATERIAL_DTO, m_newMaterialDto.ToGenericDto());
    return dto;
}

AvatarRecipeChangeTextureDto AvatarRecipeChangeTextureDto::FromGenericDto(const Engine::GenericDto& dto)
{
    AvatarRecipeChangeTextureDto recipe;
    if (auto v = dto.TryGetValue<std::string>(TOKEN_MESH_NAME)) recipe.MeshName() = v.value();
    if (auto v = dto.TryGetValue<GenericDto>(TOKEN_TEXTURE_MAPPING_DTO)) recipe.TextureDto() = TextureMappingDto::FromGenericDto(v.value());
    return recipe;
}

GenericDto AvatarRecipeChangeTextureDto::ToGenericDto() const
{
    GenericDto dto;
    dto.AddRtti(FactoryDesc(ChangeAvatarTexture::TYPE_RTTI.GetName()));
    dto.AddOrUpdate(TOKEN_MESH_NAME, m_meshName);
    dto.AddOrUpdate(TOKEN_TEXTURE_MAPPING_DTO, m_textureDto.ToGenericDto());
    return dto;
}