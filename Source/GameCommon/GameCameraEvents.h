﻿/*********************************************************************
 * \file   GameCameraEvents.h
 * \brief  game camera events
 * 
 * \author Lancelot 'Robin' Chen
 * \date   March 2023
 *********************************************************************/
#ifndef _GAME_CAMERA_EVENTS_H
#define _GAME_CAMERA_EVENTS_H

#include "Frameworks/Event.h"
#include "SceneGraph/Camera.h"

namespace Enigma::GameCommon
{
    class GameCameraCreated : public Frameworks::IEvent
    {
    public:
        GameCameraCreated(const std::shared_ptr<SceneGraph::Camera>& camera) : m_camera(camera) {}

        std::shared_ptr<SceneGraph::Camera> GetCamera() const { return m_camera.lock(); }

    protected:
        std::weak_ptr<SceneGraph::Camera> m_camera;
    };
    class GameCameraUpdated : public Frameworks::IEvent
    {
    public:
        enum class NotifyCode
        {
            CameraFrame = 0,
            FrustumZ,
            Aspect,
            FrustumFov,
        };
    public:
        GameCameraUpdated(const std::shared_ptr<SceneGraph::Camera>& camera, NotifyCode notify) : m_camera(camera), m_notify(notify) {}

        std::shared_ptr<SceneGraph::Camera> GetCamera() const { return m_camera.lock(); }
        NotifyCode GetNotifyCode() const { return m_notify; }

    protected:
        std::weak_ptr<SceneGraph::Camera> m_camera;
        NotifyCode m_notify;
    };
}

#endif // _GAME_CAMERA_EVENTS_H