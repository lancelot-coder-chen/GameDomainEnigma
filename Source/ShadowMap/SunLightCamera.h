﻿/*********************************************************************
 * \file   SunLightCamera.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   June 2023
 *********************************************************************/
#ifndef _SUN_LIGHT_CAMERA_H
#define _SUN_LIGHT_CAMERA_H

#include "SceneGraph/Camera.h"
#include "SceneGraph/Culler.h"

namespace Enigma::ShadowMap
{
    class SunLightCamera : public SceneGraph::Camera
    {
        DECLARE_EN_RTTI;
    public:
        SunLightCamera(const std::string& name);
        virtual ~SunLightCamera() override;
        SunLightCamera(const SunLightCamera& other) = delete;
        SunLightCamera(SunLightCamera&& other) noexcept = delete;
        SunLightCamera& operator=(const SunLightCamera& other) = delete;
        SunLightCamera& operator=(SunLightCamera&& other) noexcept = delete;

        void SetSunLightDir(const MathLib::Vector3& sun_dir);
        void SetViewerCamera(const std::shared_ptr<Camera>& viewer_camera);

        void CalcLightCameraSystemMatrix(SceneGraph::Culler* culler);

        const MathLib::Matrix4& GetLightViewProjMatrix() const { return m_mxLightViewProj; };
        virtual const MathLib::Matrix4& GetProjectionTransform() override;

    protected:
        void CalcSceneBoundFrustumPlane(SceneGraph::Culler* sceneCuller, const Engine::BoundingVolume& sceneWorldBound);
        void CalcLightCameraFrustum();
        std::array<MathLib::Vector3, 3> CalcLightCameraFrame() const;
        std::array<MathLib::Vector3, 8> CalcViewerFrustumCorner() const;
        void CalcSceneCropMatrix(const Engine::BoundingVolume& sceneWorldBound);

    protected:
        MathLib::Vector3 m_sunDir;
        std::weak_ptr<Camera> m_viewerCamera;

        float m_effectiveViewerNearZ;  ///< 有效範圍的近平面
        float m_effectiveViewerFarZ;    ///< 有效範圍的遠平面
        MathLib::Matrix4 m_mxLightViewProj;
        MathLib::Matrix4 m_mxSceneCrop; ///< 場景投影縮放
        MathLib::Matrix4 m_mxProjSceneCrop; ///< 合併縮放的投影矩陣
    };
}

#endif // _SUN_LIGHT_CAMERA_H