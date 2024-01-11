﻿/*********************************************************************
 * \file   ModelPrimitive.h
 * \brief  Model Primitive, value object, use shared_ptr
 *
 * \author Lancelot 'Robin' Chen
 * \date   December 2022
 *********************************************************************/
#ifndef _MODEL_PRIMITIVE_H
#define _MODEL_PRIMITIVE_H

#include "GameEngine/Primitive.h"
#include "MeshNodeTree.h"
#include "GameEngine/Animator.h"
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <list>

namespace Enigma::Renderer
{
    using error = std::error_code;
    class MeshPrimitive;
    class ModelPrimitive : public Engine::Primitive
    {
        DECLARE_EN_RTTI;
    public:
        ModelPrimitive(const Engine::PrimitiveId& id);
        ModelPrimitive(const Engine::PrimitiveId& id, const Engine::GenericDto& dto);
        ModelPrimitive(const ModelPrimitive& prim) = delete;
        ModelPrimitive(ModelPrimitive&& prim) = delete;
        ~ModelPrimitive() override;
        ModelPrimitive& operator=(const ModelPrimitive& prim) = delete;
        ModelPrimitive& operator=(ModelPrimitive&& prim) = delete;

        virtual Engine::GenericDto serializeDto() const override;

        const std::string& getName() const { return m_name; }

        const MeshNodeTree& getMeshNodeTree() const { return m_nodeTree; };
        MeshNodeTree& getMeshNodeTree() { return m_nodeTree; };
        unsigned getMeshPrimitiveCount();

        /** get mesh primitive
        @param cached_index index in cached mesh primitive array
        */
        std::shared_ptr<MeshPrimitive> getMeshPrimitive(unsigned int cached_index);
        std::shared_ptr<MeshPrimitive> findMeshPrimitive(const std::string& name);
        /** get mesh node
        @param cached_index index in cached mesh primitive array
        */
        stdext::optional_ref<MeshNode> getCachedMeshNode(unsigned int cached_index);

        void updateMeshNodeLocalTransform(unsigned int index, const MathLib::Matrix4& mxLocal);

        /** insert to renderer */
        virtual error insertToRendererWithTransformUpdating(const std::shared_ptr<Engine::IRenderer>& renderer,
            const MathLib::Matrix4& mxWorld, const Engine::RenderLightingState& lightingState) override;
        /** remove from renderer */
        virtual error removeFromRenderer(const std::shared_ptr<Engine::IRenderer>& renderer) override;

        /** calculate bounding volume */
        virtual void calculateBoundingVolume(bool axis_align) override;

        /** update world transform */
        virtual void updateWorldTransform(const MathLib::Matrix4& mxWorld) override;

        virtual void selectVisualTechnique(const std::string& techniqueName) override;

        /** enum animator list deep, including geometry's animator */
        virtual void enumAnimatorListDeep(std::list<std::shared_ptr<Engine::Animator>>& resultList) override;

    protected:
        /** sometimes we need re-cache */
        void cacheMeshPrimitive();

    protected:
        std::string m_name;
        MeshNodeTree m_nodeTree;
        std::vector<unsigned int> m_meshPrimitiveIndexCache;  ///< 記錄哪個index的mesh node擁有mesh primitive
    };
}

#endif // _MODEL_PRIMITIVE_H
