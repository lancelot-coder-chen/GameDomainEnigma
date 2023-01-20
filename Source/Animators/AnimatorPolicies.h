﻿/*********************************************************************
 * \file   AnimatorPolicies.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   January 2023
 *********************************************************************/
#ifndef _ANIMATOR_POLICIES_H
#define _ANIMATOR_POLICIES_H

#include "GameEngine/DtoDeserializer.h"
#include "GameEngine/GenericDto.h"
#include "Renderer/ModelPrimitive.h"
#include <memory>

namespace Enigma::Animators
{
    class AnimationAssetPolicy;

    class ModelAnimatorPolicy
    {
    public:
        ModelAnimatorPolicy() : m_ruid(Frameworks::Ruid::Generate()) {}
        ModelAnimatorPolicy(const std::shared_ptr<Renderer::ModelPrimitive>& controlled,
            const std::shared_ptr<AnimationAssetPolicy>& asset_policy) : m_controlledPrimitive(controlled), m_assetPolicy(asset_policy) {}

        [[nodiscard]] const Frameworks::Ruid& GetRuid() const { return m_ruid; }
        [[nodiscard]] const std::shared_ptr<Renderer::ModelPrimitive>& ControlledPrimitive() const { return m_controlledPrimitive; }
        [[nodiscard]] const std::shared_ptr<AnimationAssetPolicy>& GetAssetPolicy() const { return m_assetPolicy; }

    private:
        Frameworks::Ruid m_ruid;
        std::shared_ptr<Renderer::ModelPrimitive> m_controlledPrimitive;
        std::shared_ptr<AnimationAssetPolicy> m_assetPolicy;
    };
}

#endif // _ANIMATOR_POLICIES_H