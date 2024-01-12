﻿/*********************************************************************
 * \file   AnimatorDtos.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   January 2023
 *********************************************************************/
#ifndef ANIMATOR_DTOS_H
#define ANIMATOR_DTOS_H

#include "GameEngine/DtoDeserializer.h"
#include "GameEngine/GenericDto.h"
#include "Renderer/ModelPrimitive.h"

namespace Enigma::Animators
{
    class ModelAnimatorPolicy;

    class ModelAnimatorDto
    {
    public:
        ModelAnimatorDto();

        [[nodiscard]] const std::string& AssetName() const { return m_assetName; }
        std::string& AssetName() { return m_assetName; }
        [[nodiscard]] const std::optional<Engine::GenericDto>& AnimationAssetDto() const { return m_animationAssetDto; }
        std::optional<Engine::GenericDto>& AnimationAssetDto() { return m_animationAssetDto; }
        [[nodiscard]] const Engine::FactoryDesc& AssetFactoryDesc() const { return m_assetFactory; }
        Engine::FactoryDesc& AssetFactoryDesc() { return m_assetFactory; }
        [[nodiscard]] const Engine::GenericDtoCollection& SkinOperators() const { return m_skinOperators; }
        Engine::GenericDtoCollection& SkinOperators() { return m_skinOperators; }

        [[nodiscard]] const Engine::FactoryDesc& factoryDesc() const { return m_factoryDesc; }
        Engine::FactoryDesc& factoryDesc() { return m_factoryDesc; }

        static ModelAnimatorDto fromGenericDto(const Engine::GenericDto& dto);
        Engine::GenericDto toGenericDto();

        std::shared_ptr<ModelAnimatorPolicy> convertToPolicy(const std::shared_ptr<Renderer::ModelPrimitive>& controlled,
            const std::shared_ptr<Engine::IDtoDeserializer>& deserializer);

    private:
        std::string m_assetName;
        std::optional<Engine::GenericDto> m_animationAssetDto;
        Engine::FactoryDesc m_assetFactory;
        Engine::GenericDtoCollection m_skinOperators;
        Engine::FactoryDesc m_factoryDesc;
    };

    class SkinOperatorDto
    {
    public:
        SkinOperatorDto();

        [[nodiscard]] const Engine::FactoryDesc& factoryDesc() const { return m_factoryDesc; }
        Engine::FactoryDesc& factoryDesc() { return m_factoryDesc; }

        [[nodiscard]] const std::optional<std::string>& SkinMeshName() const { return m_skinMeshName; }
        std::optional<std::string>& SkinMeshName() { return m_skinMeshName; }
        [[nodiscard]] const std::optional<std::string>& SkinMeshNodeName() const { return m_skinMeshNodeName; }
        std::optional<std::string>& SkinMeshNodeName() { return m_skinMeshNodeName; }
        [[nodiscard]] const std::vector<std::string>& BoneNodeNames() const { return m_boneNodeNames; }
        std::vector<std::string>& BoneNodeNames() { return m_boneNodeNames; }
        [[nodiscard]] const std::optional<std::vector<MathLib::Matrix4>>& NodeOffsets() const { return m_nodeOffsets; }
        std::optional<std::vector<MathLib::Matrix4>>& NodeOffsets() { return m_nodeOffsets; }

        static SkinOperatorDto fromGenericDto(const Engine::GenericDto& dto);
        Engine::GenericDto toGenericDto();

    private:
        Engine::FactoryDesc m_factoryDesc;
        std::optional<std::string> m_skinMeshName;
        std::optional<std::string> m_skinMeshNodeName;
        std::vector<std::string> m_boneNodeNames;
        std::optional<std::vector<MathLib::Matrix4>> m_nodeOffsets;
    };
}

#endif // ANIMATOR_DTOS_H
