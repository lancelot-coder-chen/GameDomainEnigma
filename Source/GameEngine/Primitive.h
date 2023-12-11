﻿/*********************************************************************
 * \file   Primitive.h
 * \brief  primitive base class, aggregate, use shared_ptr, owned animator entity
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
#include <list>

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

        const FactoryDesc& factoryDesc() const { return m_factoryDesc; }
        FactoryDesc& factoryDesc() { return m_factoryDesc; }

        virtual GenericDto serializeDto() const = 0;

        /** insert to renderer */
        virtual error insertToRendererWithTransformUpdating(const std::shared_ptr<IRenderer>& renderer,
            const MathLib::Matrix4& mxWorld, const RenderLightingState& lightingState) = 0;
        /** remove from renderer */
        virtual error removeFromRenderer(const std::shared_ptr<IRenderer>& renderer) = 0;

        /** get bounding volume */
        virtual const BoundingVolume& getBoundingVolume() { return m_bound; };
        /** calculate bounding volume */
        virtual void calculateBoundingVolume(bool axis_align) = 0;

        /** update world transform */
        virtual void updateWorldTransform(const MathLib::Matrix4& mxWorld) = 0;

        /** get current world transform */
        virtual const MathLib::Matrix4& getPrimitiveWorldTransform() { return m_mxPrimitiveWorld; };

        /** select visual technique */
        virtual void selectVisualTechnique(const std::string& techniqueName) { m_selectedVisualTech = techniqueName; };
        /** get selected visual technique */
        virtual std::string& getSelectedVisualTechnique() { return m_selectedVisualTech; };

        virtual void attachAnimator(const std::shared_ptr<Animator>& animator) { m_animator = animator; }
        virtual const std::shared_ptr<Animator>& getAnimator() { return m_animator; }

        /** enum animator list deep, including geometry's animator */
        virtual void enumAnimatorListDeep(std::list<std::shared_ptr<Animator>>& resultList);

        /** add primitive flag */
        void addPrimitiveFlag(PrimitiveFlags flag)
        {
            m_primitiveFlags |= flag;
        }
        /** remove primitive flag */
        void removePrimitiveFlag(PrimitiveFlags flag)
        {
            m_primitiveFlags &= (~flag);
        }
        /** get primitive flag */
        PrimitiveFlags getPrimitiveFlag() { return m_primitiveFlags; };
        /** test primitive flag */
        bool testPrimitiveFlag(PrimitiveFlags flag)
        {
            return (m_primitiveFlags & flag).any();
        }

    protected:
        FactoryDesc m_factoryDesc;
        BoundingVolume m_bound;
        PrimitiveFlags m_primitiveFlags;
        MathLib::Matrix4 m_mxPrimitiveWorld;
        std::string m_selectedVisualTech;
        std::shared_ptr<Animator> m_animator;
    };

    using PrimitivePtr = std::shared_ptr<Primitive>;
}

#endif // PRIMITIVE_H
