﻿/*********************************************************************
 * \file   AnimationTimeSRT.h
 * \brief  Animation Time SRT, value object, use data object
 *
 * \author Lancelot 'Robin' Chen
 * \date   January 2023
 *********************************************************************/
#ifndef _ANIMATION_TIME_SRT_H
#define _ANIMATION_TIME_SRT_H

#include "MathLib/Matrix4.h"
#include "MathLib/Vector3.h"
#include "MathLib/Quaternion.h"
#include <tuple>
#include <vector>

namespace Enigma::Animators
{
    using SRTValueTie = std::tuple<MathLib::Vector3, MathLib::Quaternion, MathLib::Vector3>;

    class AnimationTimeSRTDto;

    /** Animation Time SRT data */
    class AnimationTimeSRT
    {
    public:
        struct ScaleKey
        {
            float m_time;
            MathLib::Vector3 m_vecKey;
            ScaleKey() : m_time(0.0f), m_vecKey(MathLib::Vector3(1.0f, 1.0f, 1.0f)) {}
            ScaleKey(float t, const MathLib::Vector3& vec) : m_time(t), m_vecKey(vec) {}
            ScaleKey(float t, float x, float y, float z) : m_time(t), m_vecKey(MathLib::Vector3(x, y, z)) {}
        };
        typedef std::vector<ScaleKey> ScaleKeyVector;

        struct RotationKey
        {
            float m_time;
            MathLib::Quaternion m_qtKey;
            RotationKey() : m_time(0.0f), m_qtKey(MathLib::Quaternion::IDENTITY) {}
            RotationKey(float t, const MathLib::Quaternion& q) : m_time(t), m_qtKey(q) {}
            RotationKey(float t, float w, float x, float y, float z) : m_time(t), m_qtKey(MathLib::Quaternion(w, x, y, z)) {}
        };
        typedef std::vector<RotationKey> RotationKeyVector;

        struct TranslateKey
        {
            float m_time;
            MathLib::Vector3 m_vecKey;
            TranslateKey() : m_time(0.0f), m_vecKey(MathLib::Vector3::ZERO) {}
            TranslateKey(float t, const MathLib::Vector3& vec) : m_time(t), m_vecKey(vec) {}
            TranslateKey(float t, float x, float y, float z) : m_time(t), m_vecKey(MathLib::Vector3(x, y, z)) {}
        };
        typedef std::vector<TranslateKey> TranslateKeyVector;

    public:
        AnimationTimeSRT();
        AnimationTimeSRT(const AnimationTimeSRTDto& dto);

        AnimationTimeSRTDto serializeDto();

        MathLib::Matrix4 CalculateTransformMatrix(float off_time);
        SRTValueTie CalculateLerpedSRT(float off_time);
        /** calculate faded transform matrix \n
        animation matrix = clip a's * weight_a + clip b's * (1.0 - weight_a)
        */
        MathLib::Matrix4 CalculateFadedTransformMatrix(float off_time_a, float off_time_b, float weight_a);
        /** calculate Faded Lerped SRT \n
        animation SRT = clip a's lerped SRT * weight_a + clip b's lerped SRT * (1.0 - weight_a)
        */
        SRTValueTie CalculateFadedLerpedSRT(float off_time_a, float off_time_b, float weight_a);

        void SetScaleKeyVector(const ScaleKeyVector& scale_key);
        void SetRotationKeyVector(const RotationKeyVector& rot_key);
        void SetTranslateKeyVector(const TranslateKeyVector& trans_key);
        const ScaleKeyVector& GetScaleKeyVector() const { return m_scaleKeyVector; };
        const RotationKeyVector& GetRotationKeyVector() const { return m_rotationKeyVector; };
        const TranslateKeyVector& GetTranslateKeyVector() const { return m_translateKeyVector; };

        /** append scale key to time offset */
        void AppendScaleKeyVector(float time_offset, const ScaleKeyVector& scale_key);
        /** append rotation key to time offset */
        void AppendRotationKeyVector(float time_offset, const RotationKeyVector& rot_key);
        /** append translate key to time offset */
        void AppendTranslateKeyVector(float time_offset, const TranslateKeyVector& trans_key);

        float GetMaxAnimationTime() const;

    protected:
        MathLib::Vector3 CalculateScaleKey(float offset_time);
        MathLib::Quaternion CalculateRotationKey(float offset_time);
        MathLib::Vector3 CalculateTranslateKey(float offset_time);

    protected:
        ScaleKeyVector m_scaleKeyVector;
        RotationKeyVector m_rotationKeyVector;
        TranslateKeyVector m_translateKeyVector;
    };
}

#endif // _ANIMATION_TIME_SRT_H
