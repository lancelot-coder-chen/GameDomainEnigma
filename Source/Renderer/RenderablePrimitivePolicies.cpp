﻿#include "RenderablePrimitivePolicies.h"
#include "SkinMeshPrimitive.h"

using namespace Enigma::Renderer;
using namespace Enigma::Engine;

MeshPrimitivePolicy::MeshPrimitivePolicy() : RenderablePrimitivePolicy(), m_geometryFactoryDesc(Geometries::GeometryData::TYPE_RTTI.getName())
{

}

MeshPrimitivePolicy::~MeshPrimitivePolicy()
{

}

std::shared_ptr<MeshPrimitive> MeshPrimitivePolicy::createPrimitive() const
{
    return nullptr; // std::make_shared<MeshPrimitive>(m_name);
}

std::shared_ptr<MeshPrimitive> SkinMeshPrimitivePolicy::createPrimitive() const
{
    return nullptr; // std::make_shared<SkinMeshPrimitive>(m_name);
}

/*const EffectTextureMapPolicy::TextureTuplePolicy& MeshPrimitivePolicy::GetTextureTuplePolicy(
    unsigned tex_idx, unsigned tuple_idx) const
{
    assert(tex_idx < m_texturePolicies.size());
    assert(tuple_idx < m_texturePolicies[tex_idx].GetTuplePolicies().size());
    return m_texturePolicies[tex_idx].GetTuplePolicies()[tuple_idx];
}*/

ModelPrimitivePolicy::ModelPrimitivePolicy() : RenderablePrimitivePolicy()
{
}

ModelPrimitivePolicy::~ModelPrimitivePolicy()
{
}