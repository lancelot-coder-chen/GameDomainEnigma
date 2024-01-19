﻿/*********************************************************************
 * \file   MeshPrimitiveBuilder.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   December 2022
 *********************************************************************/
#ifndef _MESH_PRIMITIVE_BUILDER_H
#define _MESH_PRIMITIVE_BUILDER_H

#include "RenderablePrimitivePolicies.h"
#include "Geometries/GeometryData.h"
#include "GameEngine/RenderBuffer.h"
#include "GameEngine/EffectMaterial.h"
#include "Frameworks/EventSubscriber.h"
#include "GameEngine/EffectTextureMap.h"
#include "RenderablePrimitiveDtos.h"
#include "Primitives/PrimitiveId.h"
#include <memory>

namespace Enigma::Renderables
{
    class MeshPrimitive;
    class MeshPrimitiveMetaDto;

    class MeshPrimitiveBuilder
    {
    public:
        class MeshPrimitiveBuilt : public Frameworks::IEvent
        {
        public:
            MeshPrimitiveBuilt(const Primitives::PrimitiveId& id, const std::string& name, const std::shared_ptr<MeshPrimitive>& prim)
                : m_id(id), m_name(name), m_prim(prim) {};
            const Primitives::PrimitiveId& id() const { return m_id; }
            const std::string& name() const { return m_name; }
            const std::shared_ptr<MeshPrimitive>& primitive() { return m_prim; }

        private:
            Primitives::PrimitiveId m_id;
            std::string m_name;
            std::shared_ptr<MeshPrimitive> m_prim;
        };
        class BuildMeshPrimitiveFailed : public Frameworks::IEvent
        {
        public:
            BuildMeshPrimitiveFailed(const Primitives::PrimitiveId& id, const std::string& name, std::error_code er)
                : m_id(id), m_name(name), m_error(er) {};

            const Primitives::PrimitiveId& id() const { return m_id; }
            const std::string& name() const { return m_name; }
            std::error_code error() const { return m_error; }

        private:
            Primitives::PrimitiveId m_id;
            std::string m_name;
            std::error_code m_error;
        };
    public:
        MeshPrimitiveBuilder();
        MeshPrimitiveBuilder(const MeshPrimitiveBuilder&) = delete;
        MeshPrimitiveBuilder(MeshPrimitiveBuilder&&) = delete;
        ~MeshPrimitiveBuilder();
        MeshPrimitiveBuilder& operator=(const MeshPrimitiveBuilder&) = delete;
        MeshPrimitiveBuilder& operator=(MeshPrimitiveBuilder&&) = delete;

        void constituteLazyMeshPrimitive(const std::shared_ptr<MeshPrimitive>& mesh, const Engine::GenericDto& dto);

    protected:
        void onRenderBufferBuilt(const Frameworks::IEventPtr& e);
        void onBuildRenderBufferFailed(const Frameworks::IEventPtr& e);

        void onEffectMaterialContented(const Frameworks::IEventPtr& e);
        void onContentEffectMaterialFailed(const Frameworks::IEventPtr& e);
        void onTextureContented(const Frameworks::IEventPtr& e);
        void onContentTextureFailed(const Frameworks::IEventPtr& e);

        void tryCompletingMesh();

        std::optional<unsigned> findBuildingEffectIndex(const Engine::EffectMaterialId& id);
        std::optional<std::tuple<unsigned, unsigned>> findLoadingTextureIndex(const Engine::TextureId& id);

    protected:
        Primitives::PrimitiveId m_buildingId;
        std::optional<MeshPrimitiveDto> m_buildingDto;
        std::unique_ptr<MeshPrimitiveMetaDto> m_metaDto;

        std::shared_ptr<MeshPrimitive> m_builtPrimitive;
        std::shared_ptr<Geometries::GeometryData> m_builtGeometry;
        std::shared_ptr<Engine::RenderBuffer> m_builtRenderBuffer;
        std::vector<std::shared_ptr<Engine::EffectMaterial>> m_builtEffects;
        std::vector<Engine::EffectTextureMap> m_builtTextures;

        Frameworks::EventSubscriberPtr m_onRenderBufferBuilt;
        Frameworks::EventSubscriberPtr m_onBuildRenderBufferFailed;

        Frameworks::EventSubscriberPtr m_onEffectMaterialContented;
        Frameworks::EventSubscriberPtr m_onContentEffectMaterialFailed;
        Frameworks::EventSubscriberPtr m_onTextureContented;
        Frameworks::EventSubscriberPtr m_onContentTextureFailed;
    };
}

#endif // _MESH_PRIMITIVE_BUILDER_H