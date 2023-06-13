﻿#include "SunLightCamera.h"
#include "ShadowCasterBoundFilter.h"
#include "MathLib/MathGlobal.h"
#include "SceneGraph/Frustum.h"
#include "MathLib/Triangle3.h"
#include "MathLib/TrianglePlaneClipper.h"

using namespace Enigma::ShadowMap;
using namespace Enigma::SceneGraph;
using namespace Enigma::MathLib;

DEFINE_RTTI(ShadowMap, SunLightCamera, Camera);

SunLightCamera::SunLightCamera(const std::string& name) : Camera(name, GraphicCoordSys::LeftHand)
{

}

SunLightCamera::~SunLightCamera()
{

}

void SunLightCamera::SetSunLightDir(const Vector3& sun_dir)
{
    m_sunDir = sun_dir;
    m_sunDir.NormalizeSelf();
    Vector3 up = Vector3::UNIT_Y;
    if ((std::fabs(m_sunDir.Y()) - 1.0f < Math::Epsilon())
        && (std::fabs(m_sunDir.Y()) - 1.0f > -Math::Epsilon()))
    {
        up = Vector3::UNIT_X;
    }
    ChangeCameraFrame(std::nullopt, m_sunDir, up);
}

void SunLightCamera::SetViewerCamera(const std::shared_ptr<Camera>& viewer_camera)
{
    m_viewerCamera = viewer_camera;
}

void SunLightCamera::CalcLightCameraSystemMatrix(SceneGraph::Culler* culler)
{
    if (!culler) return;

    ShadowCasterBoundFilter filterShadowBound;
    filterShadowBound.ComputeMergedBound(culler->GetVisibleSet());

    CalcSceneBoundFrustumPlane(culler, filterShadowBound.GetMergedBound());

    CalcLightCameraFrustum();

    CalcSceneCropMatrix(filterShadowBound.GetMergedBound());
    m_mxLightViewProj = m_mxProjSceneCrop * GetViewTransform();
}

void SunLightCamera::CalcSceneBoundFrustumPlane(Culler* sceneCuller, const Engine::BoundingVolume& sceneWorldBound)
{
    /*
    這裡在算有效範圍的遠近平面，只有當Frustum完全在scene BB內時，才能直接使用Viewer的遠近平面，
    否則，遠近平面都還有調整的空間，遠近平面的距離是整個SM解析度的關鍵。
    為什麼要切割? 不直接使用scene BB的8個頂點?
    如果 BB 與 frustum Z軸不是垂直的，而且BB比frustum大很多，用BB的角落算出的z值，
    會比實際看得見的範圍的z值差很多
    */
    if (m_viewerCamera.expired()) return;
    if (!m_viewerCamera.lock()->GetCullingFrustum()) return;

    m_effectiveViewerNearZ = m_viewerCamera.lock()->GetCullingFrustum()->GetNearPlaneZ();
    m_effectiveViewerFarZ = m_viewerCamera.lock()->GetCullingFrustum()->GetFarPlaneZ();

    if (sceneWorldBound.IsEmpty()) return;
    auto cropBox = sceneWorldBound.BoundingBox3();
    if (!cropBox) return;
    // 如果攝影機在BB內，near z 不需調整
    bool isCameraInBox = sceneWorldBound.PointInside(m_viewerCamera.lock()->GetLocation());

    auto vecCropBox = cropBox->ComputeVertices();
    unsigned int nFaceIndex[] =
    {
        0,2,1, 0,3,2, 4,5,6, 4,6,7,
        3,6,2, 3,7,6, 0,1,5, 0,5,4,
        4,7,3, 4,3,0, 1,2,6, 1,6,5,
    };
    std::vector<Triangle3> boxTriangles;
    boxTriangles.resize(12);
    for (unsigned int face = 0; face < 12; face++)
    {
        boxTriangles[face].Vector(0) = vecCropBox[nFaceIndex[face * 3]];
        boxTriangles[face].Vector(1) = vecCropBox[nFaceIndex[face * 3 + 1]];
        boxTriangles[face].Vector(2) = vecCropBox[nFaceIndex[face * 3 + 2]];
    }
    TrianglePlaneClipper tpClipper;
    tpClipper.SetPlanes(sceneCuller->GetPlanes());
    tpClipper.SetTriangles(boxTriangles);
    tpClipper.Clip();
    if (tpClipper.ClippedTriangleCount() == 0) return;

    Matrix4 mxView = m_viewerCamera.lock()->GetViewTransform();
    Vector3 vecBoxEdgeInView = mxView.TransformCoord(tpClipper.GetResultTriangles()[0].Vector(0));
    Vector3 vecMin = vecBoxEdgeInView;
    Vector3 vecMax = vecBoxEdgeInView;
    for (unsigned int ei = 0; ei < tpClipper.ClippedTriangleCount(); ei++)
    {
        vecBoxEdgeInView = mxView.TransformCoord(tpClipper.GetResultTriangles()[ei].Vector(0));
        vecMin = Math::MinVectorComponent(vecMin, vecBoxEdgeInView);
        vecMax = Math::MaxVectorComponent(vecMax, vecBoxEdgeInView);
        vecBoxEdgeInView = mxView.TransformCoord(tpClipper.GetResultTriangles()[ei].Vector(1));
        vecMin = Math::MinVectorComponent(vecMin, vecBoxEdgeInView);
        vecMax = Math::MaxVectorComponent(vecMax, vecBoxEdgeInView);
        vecBoxEdgeInView = mxView.TransformCoord(tpClipper.GetResultTriangles()[ei].Vector(2));
        vecMin = Math::MinVectorComponent(vecMin, vecBoxEdgeInView);
        vecMax = Math::MaxVectorComponent(vecMax, vecBoxEdgeInView);
    }
    if ((m_effectiveViewerNearZ < vecMin.Z()) && (!isCameraInBox)) m_effectiveViewerNearZ = vecMin.Z();
    if (m_effectiveViewerFarZ > vecMax.Z()) m_effectiveViewerFarZ = vecMax.Z();
}

void SunLightCamera::CalcLightCameraFrustum()
{
    std::array<Vector3, 3> vecLightFrustumAxis = CalcLightCameraFrame();

    std::array<Vector3, 8> vecViewerFrustumCorner = CalcViewerFrustumCorner();
    // light camera's look target is viewer frustum's center
    Vector3 vecViewerFrustumCenter = vecViewerFrustumCorner[0];
    for (unsigned int i = 1; i < 8; i++)
    {
        vecViewerFrustumCenter += vecViewerFrustumCorner[i];
    }
    vecViewerFrustumCenter /= 8.0f;

    Vector3 trans;
    trans.X() = vecLightFrustumAxis[0].Dot(vecViewerFrustumCenter);
    trans.Y() = vecLightFrustumAxis[1].Dot(vecViewerFrustumCenter);
    trans.Z() = vecLightFrustumAxis[2].Dot(vecViewerFrustumCenter);
    Matrix4 mxLightViewTransform = Matrix4(vecLightFrustumAxis[0], vecLightFrustumAxis[1], vecLightFrustumAxis[2], -trans, false);

    // transfer viewer frustum corner to light view, and get max,min vector
    // this light box center is at viewer frustum center
    vecViewerFrustumCorner[0] = mxLightViewTransform.TransformCoord(vecViewerFrustumCorner[0]);
    Vector3 vecMaxLightBox = vecViewerFrustumCorner[0];
    Vector3 vecMinLightBox = vecViewerFrustumCorner[0];
    for (unsigned int i = 1; i < 8; i++)
    {
        vecViewerFrustumCorner[i] = mxLightViewTransform.TransformCoord(vecViewerFrustumCorner[i]);
        vecMaxLightBox = Math::MaxVectorComponent(vecMaxLightBox, vecViewerFrustumCorner[i]);
        vecMinLightBox = Math::MinVectorComponent(vecMinLightBox, vecViewerFrustumCorner[i]);
    }

    // now can get light camera pos
    float lightCameraMoveBack = -vecMinLightBox.Z() + 0.2f;  // add some bias
    // move light camera back from dir axis and set light camera frame
    Vector3 vecLightCameraPos = vecViewerFrustumCenter - lightCameraMoveBack * m_sunDir;
    ChangeCameraFrame(vecLightCameraPos, vecLightFrustumAxis[2], vecLightFrustumAxis[1]);
    //SetLightCameraViewTransform(vecLightCameraPos, vecLightFrustumAxis[2], vecLightFrustumAxis[1], vecLightFrustumAxis[0]);
    // frustum plane w
    float planeW = -vecMinLightBox.X();
    if (planeW < vecMaxLightBox.X()) planeW = vecMaxLightBox.X();
    // frustum plane h
    float planeH = -vecMinLightBox.Y();
    if (planeH < vecMaxLightBox.Y()) planeH = vecMaxLightBox.Y();
    // frustum far z
    float farZ = vecMaxLightBox.Z() - vecMinLightBox.Z() + 0.5f;
    if (!m_cullingFrustum)
    {
        m_cullingFrustum = std::make_shared<Frustum>(m_name + "_frustum", m_handSys, Frustum::ProjectionType::Ortho);
    }
    m_cullingFrustum->SetOrthoProjection(planeW * 2.0f, planeH * 2.0f, 0.1f, farZ);
    m_mxProjSceneCrop = m_mxSceneCrop * m_cullingFrustum->GetProjectionTransform();
}

std::array<Vector3, 3> SunLightCamera::CalcLightCameraFrame() const
{
    std::array<Vector3, 3> vecLightCameraFrame;
    vecLightCameraFrame[2] = m_sunDir;
    vecLightCameraFrame[2].NormalizeSelf();
    Vector3 vecUp = Vector3::UNIT_Y;
    if ((std::fabs(vecLightCameraFrame[2].Y()) - 1.0f < Math::Epsilon())
        && (std::fabs(vecLightCameraFrame[2].Y()) - 1.0f > -Math::Epsilon()))
    {
        vecUp = Vector3::UNIT_X;
    }
    vecLightCameraFrame[0] = vecUp.Cross(vecLightCameraFrame[2]);
    vecLightCameraFrame[0].NormalizeSelf();
    vecLightCameraFrame[1] = vecLightCameraFrame[2].Cross(vecLightCameraFrame[0]);
    vecLightCameraFrame[1].NormalizeSelf();
    return vecLightCameraFrame;
}

std::array<Vector3, 8> SunLightCamera::CalcViewerFrustumCorner() const
{
    std::array<Vector3, 8> vecFrustumCorner;
    if (m_viewerCamera.expired()) return vecFrustumCorner;
    FrustumPtr frustum = m_viewerCamera.lock()->GetCullingFrustum();
    if (!frustum) return vecFrustumCorner;

    Vector3 vZ = m_viewerCamera.lock()->GetEyeToLookatVector();
    Vector3 vX = m_viewerCamera.lock()->GetRightVector();
    Vector3 vY = m_viewerCamera.lock()->GetUpVector();
    float aspect = frustum->GetAspectRatio();
    float fov = frustum->GetFov();

    float nearPlaneHalfHeight = std::tan(fov * 0.5f) * m_effectiveViewerNearZ;
    float nearPlaneHalfWidth = nearPlaneHalfHeight * aspect;

    float farPlaneHalfHeight = std::tan(fov * 0.5f) * m_effectiveViewerFarZ;
    float farPlaneHalfWidth = farPlaneHalfHeight * aspect;

    Vector3 vecNearPlaneCenter = m_viewerCamera.lock()->GetLocation() + vZ * m_effectiveViewerNearZ;
    Vector3 vecFarPlaneCenter = m_viewerCamera.lock()->GetLocation() + vZ * m_effectiveViewerFarZ;

    vecFrustumCorner[0] = vecNearPlaneCenter - vX * nearPlaneHalfWidth - vY * nearPlaneHalfHeight;
    vecFrustumCorner[1] = vecNearPlaneCenter - vX * nearPlaneHalfWidth + vY * nearPlaneHalfHeight;
    vecFrustumCorner[2] = vecNearPlaneCenter + vX * nearPlaneHalfWidth + vY * nearPlaneHalfHeight;
    vecFrustumCorner[3] = vecNearPlaneCenter + vX * nearPlaneHalfWidth - vY * nearPlaneHalfHeight;

    vecFrustumCorner[4] = vecFarPlaneCenter - vX * farPlaneHalfWidth - vY * farPlaneHalfHeight;
    vecFrustumCorner[5] = vecFarPlaneCenter - vX * farPlaneHalfWidth + vY * farPlaneHalfHeight;
    vecFrustumCorner[6] = vecFarPlaneCenter + vX * farPlaneHalfWidth + vY * farPlaneHalfHeight;
    vecFrustumCorner[7] = vecFarPlaneCenter + vX * farPlaneHalfWidth - vY * farPlaneHalfHeight;
    return vecFrustumCorner;
}

void SunLightCamera::CalcSceneCropMatrix(const Engine::BoundingVolume& sceneWorldBound)
{
    if (sceneWorldBound.IsEmpty()) return;
    Matrix4 mxLightViewProj = m_cullingFrustum->GetProjectionTransform() * GetViewTransform();

    Engine::BoundingVolume cropBound = Engine::BoundingVolume::CreateFromTransform(sceneWorldBound, mxLightViewProj);
    auto cropBox = cropBound.BoundingBox3();
    if (!cropBox) return;
    const auto vecCropBox = cropBox->ComputeVertices();
    assert(vecCropBox.size() == 8);
    Vector3 vecMin = vecCropBox[0];
    Vector3 vecMax = vecCropBox[0];
    for (unsigned int i = 1; i < vecCropBox.size(); i++)
    {
        vecMin = Math::MinVectorComponent(vecMin, vecCropBox[i]);
        vecMax = Math::MaxVectorComponent(vecMax, vecCropBox[i]);
    }
    vecMin = Math::MaxVectorComponent(vecMin, Vector3(-1.0f, -1.0f, 0.0f));
    vecMax = Math::MinVectorComponent(vecMax, Vector3(1.0f, 1.0f, 1.0f));
    vecMin.Z() = 0.0f;
    float scaleX, scaleY, scaleZ;
    float offsetX, offsetY, offsetZ;
    scaleX = 2.0f / (vecMax.X() - vecMin.X());
    scaleY = 2.0f / (vecMax.Y() - vecMin.Y());
    scaleZ = 1.0f / (vecMax.Z() - vecMin.Z());
    offsetX = -0.5f * (vecMax.X() + vecMin.X()) * scaleX;
    offsetY = -0.5f * (vecMax.Y() + vecMin.Y()) * scaleY;
    offsetZ = -vecMin.Z() * scaleZ;
    m_mxSceneCrop = Matrix4(scaleX, 0.0f, 0.0f, offsetX,
        0.0f, scaleY, 0.0f, offsetY,
        0.0f, 0.0f, scaleZ, offsetZ,
        0.0f, 0.0f, 0.0f, 1.0f);
    m_mxProjSceneCrop = m_mxSceneCrop * m_cullingFrustum->GetProjectionTransform();
}

const Matrix4& SunLightCamera::GetProjectionTransform()
{
    return m_mxProjSceneCrop;
}
