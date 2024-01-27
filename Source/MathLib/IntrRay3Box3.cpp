﻿#include "IntrRay3Box3.h"
#include "MathGlobal.h"
#include <cassert>
#include <cmath>

using namespace Enigma::MathLib;

IntrRay3Box3::IntrRay3Box3(const Ray3& ray, const Box3& box) :
    m_ray{ ray }, m_box{ box }
{
    m_intersectionType = IntersectionType::POINT;
    m_quantity = 0;
}

const Ray3& IntrRay3Box3::GetRay() const
{
    return m_ray;
}

const Box3& IntrRay3Box3::GetBox() const
{
    return m_box;
}

Intersector::Result IntrRay3Box3::test(std::unique_ptr<IntersectorCache> /*last_result*/)
{
    /** RayOBB intersection, form Real-time Rendering p574 */
    float tmin = -Math::MAX_FLOAT;
    float tmax = Math::MAX_FLOAT;
    Vector3 p = m_box.Center() - m_ray.origin();

    for (int i = 0; i < 3; i++)
    {
        float e, f, t1, t2, tt;
        e = m_box.Axis(i).dot(p);
        f = m_box.Axis(i).dot(m_ray.direction());
        if (fabs(f) > Math::ZERO_TOLERANCE) // 軸跟方向不是垂直的, 射線跟slab plane有交點
        {
            t1 = (e + m_box.Extent(i)) / f;
            t2 = (e - m_box.Extent(i)) / f;
            // make sure t1<t2
            if (t1 > t2)
            {
                tt = t1; t1 = t2; t2 = tt;
            }
            if (t1 > tmin) tmin = t1;
            if (t2 < tmax) tmax = t2;

            if (tmin > tmax) return { false, nullptr };
            if (tmax < 0.0f) return { false, nullptr };
        }
        else if ((-e - m_box.Extent(i) > 0.0f) || (-e + m_box.Extent(i) < 0.0f)) return { false, nullptr }; // 射線與slab plane是平行的，所以要確認射線的原點是否在兩slab plane中間
    }

    if (tmin > 0.0f)
    {
        m_point[m_quantity] = tmin * m_ray.direction() + m_ray.origin();
        m_tParam[m_quantity] = tmin;
        m_quantity++;
    }
    if (tmax > 0.0f)
    {
        m_point[m_quantity] = tmax * m_ray.direction() + m_ray.origin();
        m_tParam[m_quantity] = tmax;
        m_quantity++;
    }
    if (m_quantity == 0) return { false, nullptr };
    //assert(m_iQuantity);  // 至少要有一個
    return { true, nullptr };
}

Intersector::Result IntrRay3Box3::find(std::unique_ptr<IntersectorCache> last_result)
{
    // 因為沒有多少運算上的差異，所以直接呼叫Test
    return test(std::move(last_result));
}

int IntrRay3Box3::GetQuantity() const
{
    return m_quantity;
}

const Vector3& IntrRay3Box3::GetPoint(int i) const
{
    assert((i < m_quantity) && (i >= 0));
    return m_point[i];
}

float IntrRay3Box3::GetRayT(int i) const
{
    assert((i < m_quantity) && (i >= 0));
    return m_tParam[i];
}
