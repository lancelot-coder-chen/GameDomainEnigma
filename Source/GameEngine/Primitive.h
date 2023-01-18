﻿/*********************************************************************
 * \file   Primitive.h
 * \brief  primitive base class, value object, use shared_ptr
 *
 * \author Lancelot 'Robin' Chen
 * \date   October 2022
 *********************************************************************/
#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include "RenderLightingState.h"
#include "BoundingVolume.h"
#include "MathLib/Matrix4.h"
#include "Frameworks/Rtti.h"
#include <memory>
#include <bitset>
#include <system_error>

namespace Enigma::Engine
{
    using error = std::error_code;

    class IRenderer;
    class GenericDto;
    class Animator;

    class Primitive : public std::enable_shared_from_this<Primitive>
    {
        DECLARE_EN_RTTI_OF_BASE;
    public:
        enum PrimitiveBits
        {
            Primitive_UnBound = 0x01,       ///< 不計算Bounding的Primitive
            Primitive_UnRenderable = 0x02, ///< 不做 Rendering (i.e. Hide)
        };
        using PrimitiveFlags = std::bitset<2>;

    public:
        Primitive();
        Primitive(const Primitive&) = delete;
        Primitive(Primitive&&) = delete;
        virtual ~Primitive();
        Primitive& operator=(const Primitive&) = delete;
        Primitive& operator=(Primitive&&) = delete;

        virtual GenericDto SerializeDto() = 0;

        /** insert to renderer */
        virtual error InsertToRendererWithTransformUpdating(const std::shared_ptr<IRenderer>& renderer,
            const MathLib::Matrix4& mxWorld, const RenderLightingState& lightingState) = 0;
        /** remove from renderer */
        virtual error RemoveFromRenderer(const std::shared_ptr<IRenderer>& renderer) = 0;

        /** get bounding volume */
        virtual const BoundingVolume& GetBoundingVolume() { return m_bound; };
        /** calculate bounding volume */
        virtual void CalculateBoundingVolume(bool axis_align) = 0;

        /** update world transform */
        virtual void UpdateWorldTransform(const MathLib::Matrix4& mxWorld) = 0;

        /** select visual technique */
        virtual void SelectVisualTechnique(const std::string& techniqueName) { m_selectedVisualTech = techniqueName; };
        /** get selected visual technique */
        virtual std::string& GetSelectedVisualTechnique() { return m_selectedVisualTech; };

        virtual void AttachAnimator(const std::shared_ptr<Animator>& animator) { m_animator = animator; }
        /** add primitive flag */
        void AddPrimitiveFlag(PrimitiveFlags flag)
        {
            m_primitiveFlags |= flag;
        }
        /** remove primitive flag */
        void RemovePrimitiveFlag(PrimitiveFlags flag)
        {
            m_primitiveFlags &= (~flag);
        }
        /** get primitive flag */
        PrimitiveFlags GetPrimitiveFlag() { return m_primitiveFlags; };
        /** test primitive flag */
        bool TestPrimitiveFlag(PrimitiveFlags flag)
        {
            return (m_primitiveFlags & flag).any();
        }

    protected:
        BoundingVolume m_bound;
        PrimitiveFlags m_primitiveFlags;
        MathLib::Matrix4 m_mxPrimitiveWorld;
        std::string m_selectedVisualTech;
        std::shared_ptr<Animator> m_animator;
    };

    using PrimitivePtr = std::shared_ptr<Primitive>;
}

#endif // PRIMITIVE_H
