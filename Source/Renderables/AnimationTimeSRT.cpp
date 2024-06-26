﻿#include "AnimationTimeSRT.h"
#include "MathLib/MathAlgorithm.h"
#include "ModelAnimationDtos.h"
#include <cassert>

using namespace Enigma::MathLib;
using namespace Enigma::Renderables;

AnimationTimeSRT::AnimationTimeSRT()
{
}

AnimationTimeSRT::AnimationTimeSRT(const Engine::GenericDto& dto)
{
    AnimationTimeSRTDto srt_dto(dto);
    auto& scale_keys = srt_dto.scaleTimeKeys();
    auto& rotate_keys = srt_dto.rotateTimeKeys();
    auto& translate_keys = srt_dto.translateTimeKeys();
    const unsigned scale_size = static_cast<unsigned>(scale_keys.size());
    const unsigned rotate_size = static_cast<unsigned>(rotate_keys.size());
    const unsigned translate_size = static_cast<unsigned>(translate_keys.size());
    assert(scale_size % 4 == 0);
    assert(rotate_size % 5 == 0);
    assert(translate_size % 4 == 0);
    for (unsigned i = 0; i < scale_size; i += 4)
    {
        m_scaleKeyVector.emplace_back(std::vector<float>(scale_keys.begin() + i, scale_keys.begin() + i + 4));
        //m_scaleKeyVector.emplace_back(ScaleKey(scale_keys[i], scale_keys[i + 1], scale_keys[i + 2], scale_keys[i + 3]));
    }
    for (unsigned i = 0; i < rotate_size; i += 5)
    {
        m_rotationKeyVector.emplace_back(std::vector<float>(rotate_keys.begin() + i, rotate_keys.begin() + i + 5));
        //m_rotationKeyVector.emplace_back(RotationKey(rotate_keys[i], rotate_keys[i + 1], rotate_keys[i + 2], rotate_keys[i + 3], rotate_keys[i + 4]));
    }
    for (unsigned i = 0; i < translate_size; i += 4)
    {
        m_translateKeyVector.emplace_back(std::vector<float>(translate_keys.begin() + i, translate_keys.begin() + i + 4));
        //m_translateKeyVector.emplace_back(TranslateKey(translate_keys[i], translate_keys[i + 1], translate_keys[i + 2], translate_keys[i + 3]));
    }
}

Enigma::Engine::GenericDto AnimationTimeSRT::serializeDto()
{
    AnimationTimeSRTDto dto;
    std::vector<float> scale_keys;
    for (auto& key : m_scaleKeyVector)
    {
        auto values = key.values();
        scale_keys.insert(scale_keys.end(), values.begin(), values.end());
        //scale_keys.emplace_back(key.m_time);
        //scale_keys.emplace_back(key.m_vecKey.x());
        //scale_keys.emplace_back(key.m_vecKey.y());
        //scale_keys.emplace_back(key.m_vecKey.z());
    }
    dto.scaleTimeKeys() = scale_keys;
    std::vector<float> rotate_keys;
    for (auto& key : m_rotationKeyVector)
    {
        auto values = key.values();
        rotate_keys.insert(rotate_keys.end(), values.begin(), values.end());
        //rotate_keys.emplace_back(key.m_time);
        //rotate_keys.emplace_back(key.m_qtKey.w());
        //rotate_keys.emplace_back(key.m_qtKey.x());
        //rotate_keys.emplace_back(key.m_qtKey.y());
        //rotate_keys.emplace_back(key.m_qtKey.z());
    }
    dto.rotateTimeKeys() = rotate_keys;
    std::vector<float> translate_keys;
    for (auto& key : m_translateKeyVector)
    {
        auto values = key.values();
        translate_keys.insert(translate_keys.end(), values.begin(), values.end());
        //translate_keys.emplace_back(key.m_time);
        //translate_keys.emplace_back(key.m_vecKey.x());
        //translate_keys.emplace_back(key.m_vecKey.y());
        //translate_keys.emplace_back(key.m_vecKey.z());
    }
    dto.translateTimeKeys() = translate_keys;
    return dto.toGenericDto();
}

Matrix4 AnimationTimeSRT::calculateTransformMatrix(float off_time)
{
    auto [scale, rotate, translate] = calculateLerpedSRT(off_time);
    return Matrix4::FromSRT(scale, rotate, translate);
}

SRTValueTie AnimationTimeSRT::calculateLerpedSRT(float off_time)
{
    Vector3 scale = calculateScaleKey(off_time);
    Quaternion rotate = calculateRotationKey(off_time);
    Vector3 translate = calculateTranslateKey(off_time);
    return std::make_tuple(scale, rotate, translate);
}

SRTValueTie AnimationTimeSRT::calculateFadedLerpedSRT(float off_time_a, float off_time_b, float weight_a)
{
    // 分開來寫二元搜尋內插 faded key, 或是寫成一個搜尋與內插的function 似乎差不多
    // 先用兩個function 分別 call offset time A, offset time B好了
    Vector3 vecScaleA, vecScaleB;
    vecScaleA = calculateScaleKey(off_time_a);
    vecScaleB = calculateScaleKey(off_time_b);
    Vector3 scale = vecScaleA * weight_a + vecScaleB * (1.0f - weight_a);

    Quaternion rotate, qtRotA, qtRotB;
    qtRotA = calculateRotationKey(off_time_a);
    qtRotB = calculateRotationKey(off_time_b);
    rotate.Slerp(weight_a, qtRotB, qtRotA);  // slerp 函式，第一個qt是weight=0,第二個qt是weight=1

    Vector3 vecTransA, vecTransB;
    vecTransA = calculateTranslateKey(off_time_a);
    vecTransB = calculateTranslateKey(off_time_b);
    Vector3 translate = vecTransA * weight_a + vecTransB * (1.0f - weight_a);

    return std::make_tuple(scale, rotate, translate);
}

Matrix4 AnimationTimeSRT::calculateFadedTransformMatrix(float off_time_a, float off_time_b, float weight_a)
{
    auto [scale, rotate, translate] = calculateFadedLerpedSRT(off_time_a, off_time_b, weight_a);
    return Matrix4::FromSRT(scale, rotate, translate);
}

void AnimationTimeSRT::setScaleKeyVector(const ScaleKeyVector& scale_key)
{
    m_scaleKeyVector = scale_key;
}

void AnimationTimeSRT::setRotationKeyVector(const RotationKeyVector& rot_key)
{
    m_rotationKeyVector = rot_key;
}

void AnimationTimeSRT::setTranslateKeyVector(const TranslateKeyVector& trans_key)
{
    m_translateKeyVector = trans_key;
}

void AnimationTimeSRT::appendScaleKeyVector(float time_offset, const ScaleKeyVector& scale_key)
{
    unsigned int org_size = static_cast<unsigned int>(m_scaleKeyVector.size());
    unsigned int new_size = org_size + static_cast<unsigned int>(scale_key.size());
    m_scaleKeyVector.resize(new_size);
    for (unsigned int i = 0; i < scale_key.size(); i++)
    {
        m_scaleKeyVector[org_size + i].m_time = time_offset + scale_key[i].m_time;
        m_scaleKeyVector[org_size + i].m_vecKey = scale_key[i].m_vecKey;
    }
}

void AnimationTimeSRT::appendRotationKeyVector(float time_offset, const RotationKeyVector& rot_key)
{
    unsigned int org_size = static_cast<unsigned int>(m_rotationKeyVector.size());
    unsigned int new_size = org_size + static_cast<unsigned int>(rot_key.size());
    m_rotationKeyVector.resize(new_size);
    for (unsigned int i = 0; i < rot_key.size(); i++)
    {
        m_rotationKeyVector[org_size + i].m_time = time_offset + rot_key[i].m_time;
        m_rotationKeyVector[org_size + i].m_qtKey = rot_key[i].m_qtKey;
    }
}

void AnimationTimeSRT::appendTranslateKeyVector(float time_offset, const TranslateKeyVector& trans_key)
{
    unsigned int org_size = static_cast<unsigned int>(m_translateKeyVector.size());
    unsigned int new_size = org_size + static_cast<unsigned int>(trans_key.size());
    m_translateKeyVector.resize(new_size);
    for (unsigned int i = 0; i < trans_key.size(); i++)
    {
        m_translateKeyVector[org_size + i].m_time = time_offset + trans_key[i].m_time;
        m_translateKeyVector[org_size + i].m_vecKey = trans_key[i].m_vecKey;
    }
}

float AnimationTimeSRT::getMaxAnimationTime() const
{
    float ret_time = 0.0f;
    if (m_scaleKeyVector.size())
    {
        if (m_scaleKeyVector[m_scaleKeyVector.size() - 1].m_time > ret_time)
            ret_time = m_scaleKeyVector[m_scaleKeyVector.size() - 1].m_time;
    }
    if (m_rotationKeyVector.size())
    {
        if (m_rotationKeyVector[m_rotationKeyVector.size() - 1].m_time > ret_time)
            ret_time = m_rotationKeyVector[m_rotationKeyVector.size() - 1].m_time;
    }
    if (m_translateKeyVector.size())
    {
        if (m_translateKeyVector[m_translateKeyVector.size() - 1].m_time > ret_time)
            ret_time = m_translateKeyVector[m_translateKeyVector.size() - 1].m_time;
    }
    return ret_time;
}

Vector3 AnimationTimeSRT::calculateScaleKey(float offset_time)
{
    assert(!m_scaleKeyVector.empty());

    unsigned int count = static_cast<unsigned int>(m_scaleKeyVector.size());
    if (offset_time <= m_scaleKeyVector[0].m_time) return m_scaleKeyVector[0].m_vecKey;
    if (offset_time >= m_scaleKeyVector[count - 1].m_time) return m_scaleKeyVector[count - 1].m_vecKey;

    Vector3 ret;

    // 二元搜尋
    unsigned int l = 0;
    unsigned int r = count - 1;
    while (r >= l)
    {
        unsigned int m = (l + r) >> 1;
        assert(m < count);
        if (m_scaleKeyVector[m].m_time <= offset_time)
        {
            if ((m < count - 1) && (m_scaleKeyVector[m + 1].m_time > offset_time))  // off-time在m~m+1範圍內
            {
                ret = (m_scaleKeyVector[m + 1].m_vecKey - m_scaleKeyVector[m].m_vecKey) *
                    ((offset_time - m_scaleKeyVector[m].m_time) / (m_scaleKeyVector[m + 1].m_time - m_scaleKeyVector[m].m_time))
                    + m_scaleKeyVector[m].m_vecKey;
                return ret;
            }
            else if (m == count - 1)  // m 是邊界
            {
                return m_scaleKeyVector[m].m_vecKey;
            }
            else
            {
                l = m + 1;
            }
        }
        else // m_KeySRTVector[m].m_fTimeOffset>off_time
        {
            if ((m > 0) && (m_scaleKeyVector[m - 1].m_time <= offset_time))  // off-time在m-1~m的範圍內
            {
                ret = (m_scaleKeyVector[m].m_vecKey - m_scaleKeyVector[m - 1].m_vecKey) *
                    ((offset_time - m_scaleKeyVector[m - 1].m_time) / (m_scaleKeyVector[m].m_time - m_scaleKeyVector[m - 1].m_time))
                    + m_scaleKeyVector[m - 1].m_vecKey;
                return ret;
            }
            else if (m == 0)  // m 是邊界
            {
                return m_scaleKeyVector[m].m_vecKey;
            }
            else
            {
                r = m - 1;
            }
        }
    } // end while
    assert(false); // if we got here, there is some error.
    return m_scaleKeyVector[0].m_vecKey;
}

Quaternion AnimationTimeSRT::calculateRotationKey(float offset_time)
{
    assert(!m_rotationKeyVector.empty());

    unsigned int count = static_cast<unsigned int>(m_rotationKeyVector.size());
    if (offset_time <= m_rotationKeyVector[0].m_time) return m_rotationKeyVector[0].m_qtKey;
    if (offset_time >= m_rotationKeyVector[count - 1].m_time) return m_rotationKeyVector[count - 1].m_qtKey;

    // 二元搜尋
    unsigned int l = 0;
    unsigned int r = count - 1;
    while (r >= l)
    {
        unsigned int m = (l + r) >> 1;
        assert(m < count);
        if (m_rotationKeyVector[m].m_time <= offset_time)
        {
            if ((m < count - 1) && (m_rotationKeyVector[m + 1].m_time > offset_time))  // off-time在m~m+1範圍內
            {
                return Quaternion::Slerp((offset_time - m_rotationKeyVector[m].m_time)
                    / (m_rotationKeyVector[m + 1].m_time - m_rotationKeyVector[m].m_time),
                    m_rotationKeyVector[m].m_qtKey, m_rotationKeyVector[m + 1].m_qtKey);
                //return ret;
            }
            else if (m == count - 1)  // m 是邊界
            {
                return m_rotationKeyVector[m].m_qtKey;
            }
            else
            {
                l = m + 1;
            }
        }
        else // m_KeySRTVector[m].m_fTimeOffset>off_time
        {
            if ((m > 0) && (m_rotationKeyVector[m - 1].m_time <= offset_time))  // off-time在m-1~m的範圍內
            {
                return Quaternion::Slerp((offset_time - m_rotationKeyVector[m - 1].m_time)
                    / (m_rotationKeyVector[m].m_time - m_rotationKeyVector[m - 1].m_time),
                    m_rotationKeyVector[m - 1].m_qtKey, m_rotationKeyVector[m].m_qtKey);
                //return ret;
            }
            else if (m == 0)  // m 是邊界
            {
                return m_rotationKeyVector[m].m_qtKey;
            }
            else
            {
                r = m - 1;
            }
        }
    } // end while
    assert(false); // if we got here, there is some error.
    return m_rotationKeyVector[0].m_qtKey;
}

Vector3 AnimationTimeSRT::calculateTranslateKey(float offset_time)
{
    assert(!m_translateKeyVector.empty());

    unsigned int count = static_cast<unsigned int>(m_translateKeyVector.size());
    if (offset_time <= m_translateKeyVector[0].m_time) return m_translateKeyVector[0].m_vecKey;
    if (offset_time >= m_translateKeyVector[count - 1].m_time) return m_translateKeyVector[count - 1].m_vecKey;

    Vector3 ret;

    // 二元搜尋
    unsigned int l = 0;
    unsigned int r = count - 1;
    while (r >= l)
    {
        unsigned int m = (l + r) >> 1;
        assert(m < count);
        if (m_translateKeyVector[m].m_time <= offset_time)
        {
            if ((m < count - 1) && (m_translateKeyVector[m + 1].m_time > offset_time))  // off-time在m~m+1範圍內
            {
                ret = (m_translateKeyVector[m + 1].m_vecKey - m_translateKeyVector[m].m_vecKey) *
                    ((offset_time - m_translateKeyVector[m].m_time)
                        / (m_translateKeyVector[m + 1].m_time - m_translateKeyVector[m].m_time))
                    + m_translateKeyVector[m].m_vecKey;
                return ret;
            }
            else if (m == count - 1)  // m 是邊界
            {
                return m_translateKeyVector[m].m_vecKey;
            }
            else
            {
                l = m + 1;
            }
        }
        else // m_KeySRTVector[m].m_fTimeOffset>off_time
        {
            if ((m > 0) && (m_translateKeyVector[m - 1].m_time <= offset_time))  // off-time在m-1~m的範圍內
            {
                ret = (m_translateKeyVector[m].m_vecKey - m_translateKeyVector[m - 1].m_vecKey) *
                    ((offset_time - m_translateKeyVector[m - 1].m_time)
                        / (m_translateKeyVector[m].m_time - m_translateKeyVector[m - 1].m_time))
                    + m_translateKeyVector[m - 1].m_vecKey;
                return ret;
            }
            else if (m == 0)  // m 是邊界
            {
                return m_translateKeyVector[m].m_vecKey;
            }
            else
            {
                r = m - 1;
            }
        }
    } // end while
    assert(false); // if we got here, there is some error.
    return m_translateKeyVector[0].m_vecKey;
}
