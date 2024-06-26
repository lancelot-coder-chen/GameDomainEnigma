﻿#include "Vector3.h"
#include "MathGlobal.h"
#include <cassert>
#include <cstring>
#include <cmath>

using namespace Enigma;
using namespace MathLib;

const Vector3 Vector3::ZERO(0.0f, 0.0f, 0.0f);
const Vector3 Vector3::UNIT_X(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::UNIT_Y(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::UNIT_Z(0.0f, 0.0f, 1.0f);

Vector3::Vector3()
{
    m_x = m_y = m_z = (float)0.0;
}

Vector3::Vector3(float x, float y, float z)
{
    m_tuple[0] = x;
    m_tuple[1] = y;
    m_tuple[2] = z;
}

Vector3::Vector3(const float* f)
{
    assert(f);
    memcpy(m_tuple, f, 3 * sizeof(float));
}

Vector3::operator const float* () const
{
    return m_tuple;
}

Vector3::operator float* ()
{
    return m_tuple;
}

float Vector3::operator[] (int i) const
{
    assert(0 <= i && i <= 2);
    return m_tuple[i];
}

float& Vector3::operator[] (int i)
{
    assert(0 <= i && i <= 2);
    return m_tuple[i];
}

float Vector3::x() const
{
    return m_tuple[0];
}

float& Vector3::x()
{
    return m_tuple[0];
}

float Vector3::y() const
{
    return m_tuple[1];
}

float& Vector3::y()
{
    return m_tuple[1];
}

float Vector3::z() const
{
    return m_tuple[2];
}

float& Vector3::z()
{
    return m_tuple[2];
}

Vector3& Vector3::operator= (const Vector4& v)
{
    m_tuple[0] = v[0];
    m_tuple[1] = v[1];
    m_tuple[2] = v[2];
    return *this;
}

int Vector3::compareArrays(const Vector3& v) const
{
    return memcmp(m_tuple, v.m_tuple, 3 * sizeof(float));
}

bool Vector3::operator== (const Vector3& v) const
{
    return (Math::IsEqual(m_x, v.m_x) && Math::IsEqual(m_y, v.m_y)
        && Math::IsEqual(m_z, v.m_z));
}

bool Vector3::operator!= (const Vector3& v) const
{
    return (!(Math::IsEqual(m_x, v.m_x) && Math::IsEqual(m_y, v.m_y)
        && Math::IsEqual(m_z, v.m_z)));
}

bool Vector3::operator< (const Vector3& v) const
{
    return compareArrays(v) < 0;
}

bool Vector3::operator<= (const Vector3& v) const
{
    return compareArrays(v) <= 0;
}

bool Vector3::operator> (const Vector3& v) const
{
    return compareArrays(v) > 0;
}

bool Vector3::operator>= (const Vector3& v) const
{
    return compareArrays(v) >= 0;
}

Vector3 Vector3::operator+ (const Vector3& v) const
{
    return Vector3(
        m_tuple[0] + v.m_tuple[0],
        m_tuple[1] + v.m_tuple[1],
        m_tuple[2] + v.m_tuple[2]);
}

Vector3 Vector3::operator- (const Vector3& v) const
{
    return Vector3(
        m_tuple[0] - v.m_tuple[0],
        m_tuple[1] - v.m_tuple[1],
        m_tuple[2] - v.m_tuple[2]);
}

Vector3 Vector3::operator* (float scalar) const
{
    return Vector3(
        scalar * m_tuple[0],
        scalar * m_tuple[1],
        scalar * m_tuple[2]);
}

Vector3 Vector3::operator/ (float scalar) const
{
    Vector3 quot;

    if (scalar != (float)0.0)
    {
        //float invScalar = ((float)1.0) / scalar;
        quot.m_tuple[0] = m_tuple[0] / scalar;
        quot.m_tuple[1] = m_tuple[1] / scalar;
        quot.m_tuple[2] = m_tuple[2] / scalar;
    }
    else
    {
        quot.m_tuple[0] = Math::MAX_FLOAT;
        quot.m_tuple[1] = Math::MAX_FLOAT;
        quot.m_tuple[2] = Math::MAX_FLOAT;
    }

    return quot;
}

Vector3 Vector3::operator- () const
{
    return Vector3(
        -m_tuple[0],
        -m_tuple[1],
        -m_tuple[2]);
}

Vector3& Vector3::operator+= (const Vector3& v)
{
    m_tuple[0] += v.m_tuple[0];
    m_tuple[1] += v.m_tuple[1];
    m_tuple[2] += v.m_tuple[2];
    return *this;
}

Vector3& Vector3::operator-= (const Vector3& v)
{
    m_tuple[0] -= v.m_tuple[0];
    m_tuple[1] -= v.m_tuple[1];
    m_tuple[2] -= v.m_tuple[2];
    return *this;
}

Vector3& Vector3::operator*= (float scalar)
{
    m_tuple[0] *= scalar;
    m_tuple[1] *= scalar;
    m_tuple[2] *= scalar;
    return *this;
}

Vector3& Vector3::operator/= (float scalar)
{
    if (scalar != (float)0.0)
    {
        //float invScalar = ((float)1.0) / scalar;
        m_tuple[0] /= scalar;
        m_tuple[1] /= scalar;
        m_tuple[2] /= scalar;
    }
    else
    {
        m_tuple[0] = Math::MAX_FLOAT;
        m_tuple[1] = Math::MAX_FLOAT;
        m_tuple[2] = Math::MAX_FLOAT;
    }

    return *this;
}

float Vector3::length() const
{
    return sqrt(
        m_tuple[0] * m_tuple[0] +
        m_tuple[1] * m_tuple[1] +
        m_tuple[2] * m_tuple[2]);
}

float Vector3::squaredLength() const
{
    return
        m_tuple[0] * m_tuple[0] +
        m_tuple[1] * m_tuple[1] +
        m_tuple[2] * m_tuple[2];
}

float Vector3::dot(const Vector3& v) const
{
    return
        m_tuple[0] * v.m_tuple[0] +
        m_tuple[1] * v.m_tuple[1] +
        m_tuple[2] * v.m_tuple[2];
}
void Vector3::normalizeSelf()
{
    float leng = length();

    if (leng > Math::ZERO_TOLERANCE)
    {
        //float invLength = ((float)1.0) / length;
        m_tuple[0] /= leng;
        m_tuple[1] /= leng;
        m_tuple[2] /= leng;
    }
    else
    {
        leng = (float)0.0;
        m_tuple[0] = (float)0.0;
        m_tuple[1] = (float)0.0;
        m_tuple[2] = (float)0.0;
    }
}

Vector3 Vector3::normalize() const
{
    float leng = length();

    Vector3 v;
    if (leng > Math::ZERO_TOLERANCE)
    {
        //float invLength = ((float)1.0) / length;
        v.m_tuple[0] = m_tuple[0] / leng;
        v.m_tuple[1] = m_tuple[1] / leng;
        v.m_tuple[2] = m_tuple[2] / leng;
    }
    else
    {
        v.m_tuple[0] = (float)0.0;
        v.m_tuple[1] = (float)0.0;
        v.m_tuple[2] = (float)0.0;
    }

    return v;
}

Vector3 Vector3::cross(const Vector3& v) const
{
    return Vector3(
        m_tuple[1] * v.m_tuple[2] - m_tuple[2] * v.m_tuple[1],
        m_tuple[2] * v.m_tuple[0] - m_tuple[0] * v.m_tuple[2],
        m_tuple[0] * v.m_tuple[1] - m_tuple[1] * v.m_tuple[0]);
}

Vector3 Vector3::unitCross(const Vector3& v) const
{
    Vector3 cross(
        m_tuple[1] * v.m_tuple[2] - m_tuple[2] * v.m_tuple[1],
        m_tuple[2] * v.m_tuple[0] - m_tuple[0] * v.m_tuple[2],
        m_tuple[0] * v.m_tuple[1] - m_tuple[1] * v.m_tuple[0]);
    return cross.normalize();
}

namespace Enigma::MathLib
{
    Vector3 operator* (float scalar, const Vector3& v)
    {
        return Vector3(
            scalar * v[0],
            scalar * v[1],
            scalar * v[2]);
    }
}