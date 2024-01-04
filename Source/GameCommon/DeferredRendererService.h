﻿/*********************************************************************
 * \file   DeferredRendererService.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   May 2023
 *********************************************************************/
#ifndef DEFERRED_RENDERER_SERVICE_H
#define DEFERRED_RENDERER_SERVICE_H

#include "SceneRendererService.h"
#include "Renderer/MeshPrimitive.h"
#include "Frameworks/EventSubscriber.h"
#include "SceneGraph/Light.h"
#include "SceneGraph/SceneGraphEvents.h"
#include "SceneGraph/SceneGraphRepository.h"
#include "Frameworks/CommandSubscriber.h"
#include <memory>
#include <unordered_map>

namespace Enigma::GameCommon
{
    class LightingPawn;
    class LightVolumePawn;
    class DeferredRendererServiceConfiguration;

    class DeferredRendererService : public SceneRendererService
    {
        DECLARE_EN_RTTI
    public:
        DeferredRendererService(Frameworks::ServiceManager* mngr, const std::shared_ptr<GameSceneService>& scene_service,
            const std::shared_ptr<GameCameraService>& camera_service,
            const std::shared_ptr<Renderer::RendererManager>& renderer_manager,
            const std::shared_ptr<DeferredRendererServiceConfiguration>& configuration);
        DeferredRendererService(const DeferredRendererService&) = delete;
        DeferredRendererService(DeferredRendererService&&) = delete;
        virtual ~DeferredRendererService() override;
        DeferredRendererService& operator=(DeferredRendererService&) = delete;
        DeferredRendererService& operator=(DeferredRendererService&&) = delete;

        virtual Frameworks::ServiceResult onInit() override;
        virtual Frameworks::ServiceResult onTerm() override;

        virtual void CreateSceneRenderSystem(const std::string& renderer_name, const std::string& target_name) override;
        virtual void DestroySceneRenderSystem(const std::string& renderer_name, const std::string& target_name) override;

        virtual void PrepareGameScene() override;

    private:
        void CreateGBuffer(const Renderer::RenderTargetPtr& primary_target);

        void CreateAmbientLightQuad(const std::shared_ptr<SceneGraph::Light>& lit);
        void CreateSunLightQuad(const std::shared_ptr<SceneGraph::Light>& lit);
        void CreatePointLightVolume(const std::shared_ptr<SceneGraph::Light>& lit);
        void RemoveLightingPawn(const std::string& name);
        void UpdateAmbientLightQuad(const std::shared_ptr<SceneGraph::Light>& lit, SceneGraph::LightInfoUpdated::NotifyCode notify);
        void UpdateSunLightQuad(const std::shared_ptr<SceneGraph::Light>& lit, SceneGraph::LightInfoUpdated::NotifyCode notify);
        void UpdatePointLightVolume(const std::shared_ptr<SceneGraph::Light>& lit, SceneGraph::LightInfoUpdated::NotifyCode notify);

        void BindGBufferToLightingMesh(const std::shared_ptr<Renderer::MeshPrimitive>& mesh);
        void BindGBufferToLightingPawn(const std::shared_ptr<LightingPawn>& volume);

        void OnPrimaryRenderTargetCreated(const Frameworks::IEventPtr& e);
        void OnPrimaryRenderTargetResized(const Frameworks::IEventPtr& e);
        void OnGameCameraUpdated(const Frameworks::IEventPtr& e);
        void OnSceneGraphChanged(const Frameworks::IEventPtr& e);
        void OnGBufferTextureCreated(const Frameworks::IEventPtr& e);
        void OnGameLightCreated(const Frameworks::IEventPtr& e);
        void OnLightInfoDeleted(const Frameworks::IEventPtr& e);
        void OnLightInfoUpdated(const Frameworks::IEventPtr& e);
        void OnSceneGraphBuilt(const Frameworks::IEventPtr& e);
        void OnLightingPawnCreated(const Frameworks::IEventPtr& e);
        void OnPawnPrimitiveBuilt(const Frameworks::IEventPtr& e);

        void DoBindingGBuffer(const Frameworks::ICommandPtr& c);

        void OnLightingPawnBuilt(const std::string& lit_name, const std::shared_ptr<LightingPawn>& lighting_pawn);

        std::shared_ptr<LightingPawn> FindLightingPawn(const std::string& name);

        void CheckLightVolumeBackfaceCulling(const std::string& lit_name);
        void CheckLightVolumeBackfaceCulling(const std::shared_ptr<LightVolumePawn>& lit_vol, const std::shared_ptr<SceneGraph::Camera>& cam);

        void InsertLightPawnBuildingMeta(const std::string& pawn_name, const std::shared_ptr<SceneGraph::Light>& lit);

    private:
        std::shared_ptr<DeferredRendererServiceConfiguration> m_configuration;

        using LightingPawnMap = std::unordered_map<std::string, std::weak_ptr<LightingPawn>>;
        LightingPawnMap m_lightingPawns;

        struct SceneGraphLightPawnMeta
        {
            std::string m_parentNodeName;
            MathLib::Matrix4 m_localTransform;
        };
        using SceneGraphLightPawnMetaMap = std::unordered_map<std::string, SceneGraphLightPawnMeta>;
        SceneGraphLightPawnMetaMap m_buildingLightPawns;

        std::weak_ptr<Renderer::RenderTarget> m_gBuffer;

        Frameworks::EventSubscriberPtr m_onPrimaryRenderTargetCreated;
        Frameworks::EventSubscriberPtr m_onPrimaryRenderTargetResized;
        Frameworks::EventSubscriberPtr m_onGameCameraUpdated;
        Frameworks::EventSubscriberPtr m_onSceneGraphChanged;
        Frameworks::EventSubscriberPtr m_onGBufferTextureCreated;
        Frameworks::EventSubscriberPtr m_onGameLightCreated;
        Frameworks::EventSubscriberPtr m_onLightInfoDeleted;
        Frameworks::EventSubscriberPtr m_onLightInfoUpdated;
        Frameworks::EventSubscriberPtr m_onSceneGraphBuilt;
        Frameworks::EventSubscriberPtr m_onLightingPawnCreated;
        Frameworks::EventSubscriberPtr m_onPawnPrimitiveBuilt;

        Frameworks::CommandSubscriberPtr m_doBindingGBuffer;
    };
}
#endif // DEFERRED_RENDERER_SERVICE_H
