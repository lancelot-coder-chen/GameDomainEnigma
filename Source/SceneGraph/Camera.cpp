﻿#include "Camera.h"
#include "SceneGraphErrors.h"
#include "CameraFrustumDtos.h"
#include "Frustum.h"
#include <cassert>
#include <memory>

using namespace Enigma::SceneGraph;
using namespace Enigma::Engine;

DEFINE_RTTI_OF_BASE(SceneGraph, Camera);

Camera::Camera(const std::string& name, GraphicCoordSys hand)
{
    m_name = name;
    m_handSys = hand;
    m_cullingFrustum = nullptr;
    m_vecLocation = MathLib::Vector3::ZERO;
    m_vecEyeToLookAt = MathLib::Vector3::UNIT_Z;
    m_vecUp = MathLib::Vector3::UNIT_Y;
    m_vecRight = MathLib::Vector3::UNIT_X;
}

Camera::Camera(const CameraDto& dto)
{
    m_name = dto.Name();
    m_handSys = dto.HandSystem();
    SetCameraFrame(dto.EyePosition(), dto.LookAtDirection(), dto.UpVector());
    //todo : 要改成 lazy load
    m_cullingFrustum = std::make_shared<Frustum>(FrustumDto::FromGenericDto(dto.Frustum()));
}

Camera::~Camera()
{
    m_cullingFrustum = nullptr;
}

GenericDto Camera::SerializeDto()
{
    CameraDto dto;
    dto.Name() = m_name;
    dto.HandSystem() = m_handSys;
    dto.EyePosition() = m_vecLocation;
    dto.LookAtDirection() = m_vecEyeToLookAt;
    dto.UpVector() = m_vecUp;
    dto.Frustum() = m_cullingFrustum->SerializeDto();
    return dto.ToGenericDto();
}

error Camera::SetCameraAxis(const MathLib::Vector3& eye_to_lookat, const MathLib::Vector3& up)
{
    MathLib::Vector3 _eye_to_lookat = eye_to_lookat.Normalize();
    m_vecEyeToLookAt = _eye_to_lookat;
    if (m_handSys == GraphicCoordSys::LeftHand)
    {
        m_vecCameraForward = _eye_to_lookat;
    }
    else
    {
        m_vecCameraForward = -_eye_to_lookat;
    }
    MathLib::Vector3 _up = up.Normalize();
    MathLib::Vector3 right = _up.Cross(m_vecCameraForward);
    m_vecRight = right.Normalize();
    m_vecUp = m_vecCameraForward.Cross(m_vecRight);

    _UpdateViewTransform();
    return ErrorCode::ok;
}

error Camera::SetCameraFrame(const MathLib::Vector3& eye, const MathLib::Vector3& eye_to_lookat, const MathLib::Vector3& up)
{
    m_vecLocation = eye;
    MathLib::Vector3 _eye_to_lookat = eye_to_lookat.Normalize();
    m_vecEyeToLookAt = _eye_to_lookat;
    if (m_handSys == GraphicCoordSys::LeftHand)
    {
        m_vecCameraForward = _eye_to_lookat;
    }
    else
    {
        m_vecCameraForward = -_eye_to_lookat;
    }
    MathLib::Vector3 _up = up.Normalize();
    MathLib::Vector3 _right = _up.Cross(m_vecCameraForward);
    m_vecRight = _right.Normalize();
    m_vecUp = m_vecCameraForward.Cross(m_vecRight);
    _UpdateViewTransform();
    return ErrorCode::ok;
}

error Camera::SetCameraLocation(const MathLib::Vector3& pos)
{
    m_vecLocation = pos;
    _UpdateViewTransform();
    return ErrorCode::ok;
}

error Camera::SetCullingFrustum(const FrustumPtr& frustum)
{
    assert(frustum);  // must valid pointer
    m_cullingFrustum = frustum;

    return ErrorCode::ok;
}

void Camera::_UpdateViewTransform()
{
    MathLib::Vector3 trans;
    trans.X() = m_vecRight.Dot(m_vecLocation);
    trans.Y() = m_vecUp.Dot(m_vecLocation);
    trans.Z() = m_vecCameraForward.Dot(m_vecLocation);
    m_mxViewTransform = MathLib::Matrix4(m_vecRight, m_vecUp, m_vecCameraForward, -trans, false);
}
