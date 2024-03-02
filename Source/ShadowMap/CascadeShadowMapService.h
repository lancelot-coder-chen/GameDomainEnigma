﻿/*********************************************************************
 * \file   CascadeShadowMapService.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   June 2023
 *********************************************************************/
#ifndef CASCADE_SHADOW_MAP_SERVICE_H
#define CASCADE_SHADOW_MAP_SERVICE_H

#include "ShadowMapService.h"
#include "CascadeShadowMapServiceConfiguration.h"
#include "GraphicKernel/IDeviceRasterizerState.h"
#include "GameEngine/EffectVariable.h"

namespace Enigma::ShadowMap
{
    class CSMSunLightCamera;

    class CascadeShadowMapService : public ShadowMapService
    {
        DECLARE_EN_RTTI;
    public:
        CascadeShadowMapService(Frameworks::ServiceManager* manager, const std::shared_ptr<GameCommon::GameSceneService>& scene_service,
            const std::shared_ptr<GameCommon::GameCameraService>& camera_service,
            const std::shared_ptr<Renderer::RendererManager>& renderer_manager,
            const std::shared_ptr<CascadeShadowMapServiceConfiguration>& configuration);
        virtual ~CascadeShadowMapService() override;
        CascadeShadowMapService(const CascadeShadowMapService& other) = delete;
        CascadeShadowMapService(CascadeShadowMapService&& other) noexcept = delete;
        CascadeShadowMapService& operator=(const CascadeShadowMapService& other) = delete;
        CascadeShadowMapService& operator=(CascadeShadowMapService&& other) noexcept = delete;

        virtual Frameworks::ServiceResult onInit() override;
        virtual Frameworks::ServiceResult onTick() override;
        virtual Frameworks::ServiceResult onTerm() override;

        virtual void createShadowRenderSystem(const std::string& renderer_name, const std::string& target_name) override;
        virtual void destroyShadowRenderSystem(const std::string& renderer_name, const std::string& target_name) override;

    protected:
        virtual void createSunLightCamera(const std::shared_ptr<SceneGraph::Light>& lit) override;
        virtual void deleteSunLightCamera() override;
        virtual void updateSunLightDirection(const MathLib::Vector3& dir) override;

    private:
        static void assignLightViewProjectionTransforms(Engine::EffectVariable& var);
        static void assignCascadeDistances(Engine::EffectVariable& var);
        static void assignCascadeTextureCoordTransforms(Engine::EffectVariable& var);
        static void assignSliceCount(Engine::EffectVariable& var);
        static void assignSliceDimension(Engine::EffectVariable& var);
        static void assignFaceLightThreshold(Engine::EffectVariable& var);

    private:
        std::shared_ptr<CascadeShadowMapServiceConfiguration> m_configuration;
        std::shared_ptr<CSMSunLightCamera> m_sunLightCamera;
        //todo: backface culling 是做什麼的??
        //std::shared_ptr<Graphics::IDeviceRasterizerState> m_backfaceCullingState;
        //bool m_isRenderBackFace;

        static inline std::vector<MathLib::Matrix4> m_cascadeLightViewProjections;
        static inline MathLib::Vector4 m_cascadeDistances;
        static inline std::vector<MathLib::Matrix4> m_cascadeTextureCoordTransforms;
        static inline float m_faceLightThreshold = 0.02f;
    };
}

#endif // CASCADE_SHADOW_MAP_SERVICE_H
