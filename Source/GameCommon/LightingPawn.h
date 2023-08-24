﻿/*********************************************************************
 * \file   LightingPawn.h
 * \brief  base class for light quad & light volume
 *
 * \author Lancelot 'Robin' Chen
 * \date   August 2023
 *********************************************************************/
#ifndef LIGHTING_PAWN_H
#define LIGHTING_PAWN_H

#include "SceneGraph/Pawn.h"
#include "SceneGraph/Light.h"

namespace Enigma::GameCommon
{
    using error = std::error_code;
    class LightingPawn : public SceneGraph::Pawn
    {
        DECLARE_EN_RTTI;
    public:
        LightingPawn(const std::string& name);
        LightingPawn(const Engine::GenericDto& o);
        LightingPawn(const LightingPawn&) = delete;
        LightingPawn(LightingPawn&&) = delete;
        virtual ~LightingPawn() override;
        LightingPawn& operator=(const LightingPawn&) = delete;
        LightingPawn& operator=(LightingPawn&&) = delete;

        virtual void ResolveFactoryLinkage(const Engine::GenericDto& dto, Engine::FactoryLinkageResolver<Spatial>& resolver) override;

        void SetHostLight(const std::shared_ptr<SceneGraph::Light>& light);
        std::string GetHostLightName() const { return m_hostLight.expired() ? "" : m_hostLight.lock()->GetSpatialName(); }

    protected:
        std::weak_ptr<SceneGraph::Light> m_hostLight;
    };
}


#endif // LIGHTING_PAWN_H