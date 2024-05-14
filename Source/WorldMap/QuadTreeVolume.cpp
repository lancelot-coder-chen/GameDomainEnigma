﻿#include "QuadTreeVolume.h"
#include "MathLib/ContainmentBox2.h"
#include "SceneGraph/SceneGraphQueries.h"

using namespace Enigma::WorldMap;
using namespace Enigma::SceneGraph;
using namespace Enigma::Engine;
using namespace Enigma::MathLib;

unsigned constexpr maxDepth = 4;

QuadTreeVolume::QuadTreeVolume(const SpatialId& id)
{
    m_id = id;
    if (!m_id.empty())
    {
        m_worldTransform = std::make_shared<QueryWorldTransform>(m_id)->dispatch();
        m_modelBounding = std::make_shared<QueryModelBound>(m_id)->dispatch();
        for (unsigned i = 0; i < maxChildren; ++i)
        {
            auto sub_volume_node_id = subVolumeNodeId(i);
            if (auto has_node = std::make_shared<HasSpatial>(sub_volume_node_id)->dispatch()) m_children[i] = std::make_shared<QuadTreeVolume>(sub_volume_node_id);
        }
    }
}

QuadTreeVolume::QuadTreeVolume(const SpatialId& id, const std::optional<SceneGraph::SpatialId>& parent_id, const Matrix4& worldTransform, const BoundingVolume& modelBounding)
    : m_id(id), m_parentId(parent_id), m_worldTransform(worldTransform), m_modelBounding(modelBounding)
{
}

void QuadTreeVolume::constituteFullQuadTree(unsigned depth)
{
    assert(depth <= maxDepth);
    if (m_id.empty()) return;
    if (depth == 0) return;
    if (!m_modelBounding.BoundingBox3()) return;
    Box3 model_box = m_modelBounding.BoundingBox3().value();

    constexpr std::array<float, maxChildren> child_offset_x = { -0.5f, -0.5f, 0.5f, 0.5f };
    constexpr std::array<float, maxChildren> child_offset_z = { -0.5f, 0.5f, -0.5f, 0.5f };
    Box3 child_box = Box3(Vector3::ZERO, model_box.Axis(0), model_box.Axis(1), model_box.Axis(2), model_box.Extent(0) * 0.5f, model_box.Extent(1), model_box.Extent(2) * 0.5f);
    for (unsigned i = 0; i < maxChildren; i++)
    {
        Vector3 child_center_in_parent = model_box.Axis(0) * child_offset_x[i] * model_box.Extent(0) + model_box.Axis(2) * child_offset_z[i] * model_box.Extent(2);
        Matrix4 child_transform = m_worldTransform * Matrix4::MakeTranslateTransform(child_center_in_parent);
        m_children[i] = std::make_shared<QuadTreeVolume>(subVolumeNodeId(i), m_id, child_transform, BoundingVolume{ child_box });
        m_children[i]->constituteFullQuadTree(depth - 1);
    }
}

bool QuadTreeVolume::isWorldPositionInside(const MathLib::Vector3& worldPosition) const
{
    if (m_id.empty()) return false;
    auto local_position = m_worldTransform.Inverse() * worldPosition;
    return (m_modelBounding.PointInsideFlags(local_position) & static_cast<GenericBV::FlagBits>(GenericBV::TestedAxis::XZ)) == GenericBV::TestedAxis::XZ;
}

std::shared_ptr<QuadTreeVolume> QuadTreeVolume::findFittingVolume(const BoundingVolume& bv_in_world)
{
    if (m_id.empty()) return nullptr;
    if (!testBoundingEnvelop(bv_in_world)) return nullptr;
    for (const auto& child_volume : m_children)
    {
        if (!child_volume) continue;
        if (!child_volume->isWorldPositionInside(bv_in_world.Center())) continue;
        if (auto fitting_volume = child_volume->findFittingVolume(bv_in_world)) return fitting_volume;
    }
    // 下層找不到適合size的volume，回傳自己
    return shared_from_this();
}

std::list<std::shared_ptr<QuadTreeVolume>> QuadTreeVolume::collectFitVolumes(const Engine::BoundingVolume& bv_in_world)
{
    std::list<std::shared_ptr<QuadTreeVolume>> result;
    collectFitVolumes(bv_in_world, result);
    return result;
}

SpatialId QuadTreeVolume::subVolumeNodeId(unsigned index) const
{
    assert(!m_id.empty());
    assert(index < maxChildren);
    auto sub_node_name = m_id.name() + "_" + std::to_string(index);
    return { sub_node_name, m_id.rtti() };
}

bool QuadTreeVolume::testBoundingEnvelop(const Engine::BoundingVolume& bv_in_world) const
{
    if (m_id.empty()) return false;
    BoundingVolume quad_bv = BoundingVolume::CreateFromTransform(m_modelBounding, m_worldTransform);
    Box3 quad_box = quad_bv.BoundingBox3().value();
    Box2 box_q(Vector2(quad_box.Center().x(), quad_box.Center().z()),
        Vector2(quad_box.Axis()[0].x(), quad_box.Axis()[0].z()),
        Vector2(quad_box.Axis()[2].x(), quad_box.Axis()[2].z()), quad_box.Extent()[0], quad_box.Extent()[2]);
    if (auto bv_box = bv_in_world.BoundingBox3()) // model bound is box
    {
        Box3 box_world = bv_box.value();
        Box2 box_m(Vector2(box_world.Center().x(), box_world.Center().z()), Vector2::UNIT_X, Vector2::UNIT_Y,
            box_world.Extent()[0], box_world.Extent()[2]);
        return ContainmentBox2::TestBox2EnvelopBox2(box_q, box_m);
    }
    return false;
}

void QuadTreeVolume::collectFitVolumes(const Engine::BoundingVolume& bv_in_world, std::list<std::shared_ptr<QuadTreeVolume>>& result)
{
    if (m_id.empty()) return;
    if (!isWorldPositionInside(bv_in_world.Center())) return;
    if (!testBoundingEnvelop(bv_in_world)) return;
    result.push_back(shared_from_this());
    for (const auto& child_volume : m_children)
    {
        if (!child_volume) continue;
        child_volume->collectFitVolumes(bv_in_world, result);
    }
}
