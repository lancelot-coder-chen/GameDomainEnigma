﻿#include "ScenePicker.h"
#include "SceneGraph/Pawn.h"
#include "GameEngine/IntrBVRay3.h"
#include "Platforms/PlatformLayer.h"
#include <algorithm>

using namespace LevelEditor;
using namespace Enigma::SceneGraph;
using namespace Enigma::MathLib;
using namespace Enigma::Engine;

ScenePicker::ScenePicker() : SceneTraveler()
{
    m_filter = FilterFlag::Pick_All;
}

ScenePicker::~ScenePicker()
{
}

SceneTraveler::TravelResult ScenePicker::TravelTo(const SpatialPtr& spatial)
{
    if ((!spatial) || (spatial->GetWorldBound().IsEmpty())) return TravelResult::InterruptError;

    IntrBVRay3 intrBV(spatial->GetWorldBound(), m_pickerRay);
    bool bv_test_res = intrBV.Test(0);
    if (!bv_test_res) return TravelResult::TestFail;  // no intersection

    if ((m_filter & FilterFlag::Pick_Node) && (spatial->TypeInfo().IsDerived(Node::TYPE_RTTI)))
    {
        PushNodeRecord(spatial);
    }
    else if ((m_filter & FilterFlag::Pick_Pawn) && (spatial->TypeInfo().IsDerived(Pawn::TYPE_RTTI)))
    {
        bool test_result = PushPawnRecord(spatial);
        if (!test_result) return TravelResult::TestFail;
    }

    return TravelResult::Continue;
}

void ScenePicker::SetAssociatedCamera(const std::shared_ptr<Camera>& camera)
{
    m_camera = camera;
}

void ScenePicker::SetClippingCoord(const Vector2& clippingCoord)
{
    if (FATAL_LOG_EXPR(m_camera.expired())) return;

    Vector3 clip_vec = Vector3(clippingCoord.X(), clippingCoord.Y(), 0.0f);
    Matrix4 mxProj = m_camera.lock()->GetProjectionTransform().Inverse();
    Vector3 camera_vec = mxProj.TransformCoord(clip_vec);
    Matrix4 mxView = m_camera.lock()->GetViewTransform().Inverse();
    Vector3 world_vec = mxView.TransformCoord(camera_vec);

    Vector3 ray_dir = world_vec - m_camera.lock()->GetLocation();
    ray_dir.NormalizeSelf();

    m_pickerRay = Ray3(m_camera.lock()->GetLocation(), ray_dir);
}

void ScenePicker::Picking(const SpatialPtr& sceneRoot)
{
    m_pickRecords.clear();
    if (!sceneRoot) return;
    sceneRoot->VisitBy(this);

    if (!m_pickRecords.empty())
    {
        sort(m_pickRecords.begin(), m_pickRecords.end(),
            [](const ScenePicker::PickerRecord& rec1, const ScenePicker::PickerRecord& rec2) -> bool
            { return (rec1.m_tParam < rec2.m_tParam); });
    }
}

unsigned int ScenePicker::GetRecordCount()
{
    return static_cast<unsigned int>(m_pickRecords.size());
}

std::optional<ScenePicker::PickerRecord> ScenePicker::GetPickRecord(unsigned int idx)
{
    if (idx >= m_pickRecords.size()) return std::nullopt;
    return m_pickRecords[idx];
}

void ScenePicker::PushNodeRecord(const SpatialPtr& spatial)
{
    NodePtr node = std::dynamic_pointer_cast<Node, Spatial>(spatial);
    if (!node) return;
    m_pickRecords.emplace_back(PickerRecord(node));
}

bool ScenePicker::PushPawnRecord(const SpatialPtr& spatial)
{
    auto pawn = std::dynamic_pointer_cast<Pawn>(spatial);
    if (!pawn) return false;

    IntrPrimitiveRay3 intrPrim(pawn->GetPrimitive(), m_pickerRay);
    bool prim_find_res = intrPrim.Find(0);
    if (!prim_find_res) return false;

    // copy result point
    if (intrPrim.GetQuantity())
    {
        for (size_t i = 0; i < intrPrim.GetQuantity(); i++)
        {
            float t = intrPrim.GetRayT(i);
            m_pickRecords.emplace_back(PickerRecord(pawn, t, t * m_pickerRay.Direction() + m_pickerRay.Origin()));
        }
    }
    return true;
}
