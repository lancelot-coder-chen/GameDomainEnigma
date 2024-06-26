﻿#include "BoxBV.h"
#include "Platforms/PlatformLayer.h"
#include "MathLib/ContainmentBox3.h"
#include "MathLib/MathGlobal.h"
#include "SphereBV.h"
#include <cassert>
#include <cmath>

using namespace Enigma::Engine;

BoxBV::BoxBV(const MathLib::Box3& box) : GenericBV()
{
    m_box = box;
}

BoxBV::~BoxBV()
{
}

void BoxBV::CreateFromTransform(const MathLib::Matrix4& mx, const std::unique_ptr<GenericBV>& source)
{
    if (FATAL_LOG_EXPR(!source)) return;
    MathLib::Vector3 center;
    MathLib::Vector3 axis[3];
    MathLib::Vector3 extent;
    if (BoxBV* box_bv = dynamic_cast<BoxBV*>(source.get()))
    {
        center = box_bv->m_box.Center();
        for (int i = 0; i < 3; i++)
        {
            axis[i] = box_bv->m_box.Axis(i);
            extent[i] = box_bv->m_box.Extent(i);
        }
    }
    else if (SphereBV* sphere_bv = dynamic_cast<SphereBV*>(source.get()))
    {
        center = sphere_bv->GetSphere().Center();
        axis[0] = MathLib::Vector3::UNIT_X;
        axis[1] = MathLib::Vector3::UNIT_Y;
        axis[2] = MathLib::Vector3::UNIT_Z;
        extent[0] = extent[1] = extent[2] = sphere_bv->GetSphere().Radius();
    }
    else
    {
        assert(false);
    }

    m_box = { MathLib::Box3::UNIT_BOX };

    m_box.Center() = mx.TransformCoord(center);
    for (int i = 0; i < 3; i++)
    {
        m_box.Axis(i) = mx.TransformVector(axis[i]);
        float len = m_box.Axis(i).length();
        m_box.Axis(i).normalizeSelf();
        m_box.Extent(i) = len * extent[i];
    }
}

void BoxBV::Copy(const std::unique_ptr<GenericBV>& source)
{
    if (FATAL_LOG_EXPR(!source)) return;
    BoxBV* box_bv = dynamic_cast<BoxBV*>(source.get());
    if (FATAL_LOG_EXPR(!box_bv)) return;

    m_box = box_bv->m_box;
}

void BoxBV::ZeroReset()
{
    m_box = MathLib::Box3();
}

void BoxBV::MergeBoundingVolume(const MathLib::Matrix4& mx, const std::unique_ptr<GenericBV>& source)
{
    assert(!m_box.isEmpty());
    BoxBV merge_bv{ MathLib::Box3::UNIT_BOX };
    merge_bv.CreateFromTransform(mx, source);
    m_box = MathLib::ContainmentBox3::MergeBoxes(merge_bv.m_box, m_box);
}

Enigma::MathLib::Plane3::SideOfPlane BoxBV::WhichSide(const MathLib::Plane3& plane) const
{
    float projCenter = plane.Normal().dot(m_box.Center()) - plane.Constant();
    float abs0 = std::fabs(plane.Normal().dot(m_box.Axis(0)));
    float abs1 = std::fabs(plane.Normal().dot(m_box.Axis(1)));
    float abs2 = std::fabs(plane.Normal().dot(m_box.Axis(2)));
    float projRadius = m_box.Extent(0) * abs0 + m_box.Extent(1) * abs1 +
        m_box.Extent(2) * abs2;

    if (projCenter - projRadius >= 0.0f)
    {
        return MathLib::Plane3::SideOfPlane::Positive;
    }

    if (projCenter + projRadius <= 0.0f)
    {
        return MathLib::Plane3::SideOfPlane::Negative;
    }

    return MathLib::Plane3::SideOfPlane::Overlap;
}

void BoxBV::ComputeFromData(const MathLib::Vector3* pos, unsigned quantity, bool axis_align)
{
    assert(pos);
    assert(quantity > 0);
    if (axis_align)
    {
        m_box = MathLib::ContainmentBox3::ComputeAlignedBox(pos, quantity);
    }
    else
    {
        m_box = MathLib::ContainmentBox3::ComputeOrientedBox(pos, quantity);
    }
}

void BoxBV::ComputeFromData(const MathLib::Vector4* pos, unsigned quantity, bool axis_align)
{
    assert(pos);
    assert(quantity > 0);
    if (axis_align)
    {
        m_box = MathLib::ContainmentBox3::ComputeAlignedBox(pos, quantity);
    }
    else
    {
        m_box = MathLib::ContainmentBox3::ComputeOrientedBox(pos, quantity);
    }
}

void BoxBV::ComputeFromData(const float* vert, unsigned pitch, unsigned quantity, bool axis_align)
{
    assert(vert);
    assert(quantity > 0);
    if (axis_align)
    {
        m_box = MathLib::ContainmentBox3::ComputeAlignedBox(vert, pitch, quantity);
    }
    else
    {
        m_box = MathLib::ContainmentBox3::ComputeOrientedBox(vert, pitch, quantity);
    }
}

bool BoxBV::PointInside(const MathLib::Vector3& pos)
{
    MathLib::Vector3 diff = pos - m_box.Center();
    float d = diff.dot(m_box.Axis(0));
    if (((d + MathLib::Math::Epsilon()) < -m_box.Extent(0)) || ((d - MathLib::Math::Epsilon()) > m_box.Extent(0)))
    {
        return false;
    }
    d = diff.dot(m_box.Axis(1));
    if (((d + MathLib::Math::Epsilon()) < -m_box.Extent(1)) || ((d - MathLib::Math::Epsilon()) > m_box.Extent(1)))
    {
        return false;
    }
    d = diff.dot(m_box.Axis(2));
    if (((d + MathLib::Math::Epsilon()) < -m_box.Extent(2)) || ((d - MathLib::Math::Epsilon()) > m_box.Extent(2)))
    {
        return false;
    }

    return true;
}

BoxBV::FlagBits BoxBV::PointInsideFlags(const MathLib::Vector3& pos)
{
    FlagBits flags{0x0};
    MathLib::Vector3 diff = pos - m_box.Center();
    float d = diff.dot(m_box.Axis(0));
    if (((d + MathLib::Math::Epsilon()) > -m_box.Extent(0)) && ((d - MathLib::Math::Epsilon()) < m_box.Extent(0)))
    {
        flags.set(static_cast<size_t>(Axis::x));
    }
    d = diff.dot(m_box.Axis(1));
    if (((d + MathLib::Math::Epsilon()) > -m_box.Extent(1)) && ((d - MathLib::Math::Epsilon()) < m_box.Extent(1)))
    {
        flags.set(static_cast<size_t>(Axis::y));
    }
    d = diff.dot(m_box.Axis(2));
    if (((d + MathLib::Math::Epsilon()) > -m_box.Extent(2)) && ((d - MathLib::Math::Epsilon()) < m_box.Extent(2)))
    {
        flags.set(static_cast<size_t>(Axis::z));
    }

    return flags;
}
