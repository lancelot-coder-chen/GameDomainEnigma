﻿/*********************************************************************
 * \file   Vector3.h
 * \brief  
 * 
 * \author Lancelot 'Robin' Chen
 * \date   April 2022
 *********************************************************************/
#ifndef _MATH_VECTOR3_H
#define _MATH_VECTOR3_H

#include "Vector4.h"

namespace Enigma::MathLib
{
    /** Math Lib Vector3 */
    class Vector3
    {
    public:
        // construction
        Vector3();  // uninitialized
        Vector3(float x, float y, float z);
        Vector3(float* f);

        /** @name coordinate access */
        //@{
        operator const float* () const;
        operator float* ();
        float operator[] (int i) const;
        float& operator[] (int i);
        float X() const;
        float& X();
        float Y() const;
        float& Y();
        float Z() const;
        float& Z();
        //@}

        /** @name assignment */
        //@{
        Vector3& operator= (const Vector4& v);
        //@}

        /** @name comparison */
        //@{
        bool operator== (const Vector3& v) const;  ///< 浮點數值比較
        bool operator!= (const Vector3& v) const;   ///< 浮點數值比較
        bool operator<  (const Vector3& v) const;   ///< 比較陣列記憶體內容
        bool operator<= (const Vector3& v) const;    ///< 比較陣列記憶體內容
        bool operator>  (const Vector3& v) const;   ///< 比較陣列記憶體內容
        bool operator>= (const Vector3& v) const;   ///< 比較陣列記憶體內容
        //@}

        /** @name arithmetic operations */
        //@{
        Vector3 operator+ (const Vector3& v) const;
        Vector3 operator- (const Vector3& v) const;
        Vector3 operator* (float scalar) const;
        Vector3 operator/ (float scalar) const;
        Vector3 operator- () const;
        //@}

        /** @name arithmetic updates */
        //@{
        Vector3& operator+= (const Vector3& v);
        Vector3& operator-= (const Vector3& v);
        Vector3& operator*= (float scalar);
        Vector3& operator/= (float scalar);
        //@}

        /** @name vector operations */
        //@{
        float Length() const;
        float SquaredLength() const;
        float Dot(const Vector3& v) const;
        Vector3 Normalize() const;
        /// The cross products are computed using the left-handed rule.
        Vector3 Cross(const Vector3& v) const;
        /// The cross products are computed using the left-handed rule.
        Vector3 UnitCross(const Vector3& v) const;
        //@}

        /** @name special vectors */
        //@{
        static const Vector3 ZERO;
        static const Vector3 UNIT_X;
        static const Vector3 UNIT_Y;
        static const Vector3 UNIT_Z;
        //@}

    private:
        // support for none-equal comparisons
        int CompareArrays(const Vector3& v) const;
        /** member data */
        union
        {
            struct
            {
                float m_x;
                float m_y;
                float m_z;
            };
            float m_tuple[3];
        };
    };
    /** scalar * vector3 */
    Vector3 operator* (float scalar, const Vector3& v);
}

#endif // !_MATH_VECTOR3_H
