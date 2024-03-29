﻿#include "SkinMeshPrimitive.h"
#include "RenderablePrimitiveDtos.h"
#include "GameEngine/EffectMaterial.h"

using namespace Enigma::Renderables;
using namespace Enigma::Engine;
using namespace Enigma::MathLib;
using namespace Enigma::Frameworks;
using namespace Enigma::Primitives;

const std::string SEMANTIC_BONE_MATRIX = "BoneMatrix";

DEFINE_RTTI(Renderables, SkinMeshPrimitive, MeshPrimitive);

SkinMeshPrimitive::SkinMeshPrimitive(const PrimitiveId& id) : MeshPrimitive(id)
{
    m_factoryDesc = FactoryDesc(SkinMeshPrimitive::TYPE_RTTI.getName());
    m_ownerNodeRootRefTransform = Matrix4::IDENTITY;
}

SkinMeshPrimitive::SkinMeshPrimitive(const PrimitiveId& id, const Engine::GenericDto& dto, const std::shared_ptr<Geometries::GeometryRepository>& geometry_repository) : MeshPrimitive(id, dto, geometry_repository)
{
    m_factoryDesc = dto.getRtti();
    m_ownerNodeRootRefTransform = Matrix4::IDENTITY;
}

/*SkinMeshPrimitive::SkinMeshPrimitive(const SkinMeshPrimitive& skin) : MeshPrimitive(skin)
{
    m_boneEffectMatrix = skin.m_boneEffectMatrix;
    m_ownerNodeRootRefTransform = skin.m_ownerNodeRootRefTransform;
}

SkinMeshPrimitive::SkinMeshPrimitive(SkinMeshPrimitive&& skin) noexcept : MeshPrimitive(std::move(static_cast<MeshPrimitive&>(skin)))
{
    m_boneEffectMatrix = std::move(skin.m_boneEffectMatrix);
    m_ownerNodeRootRefTransform = std::move(skin.m_ownerNodeRootRefTransform);
}*/

SkinMeshPrimitive::~SkinMeshPrimitive()
{
    loosePrimitiveBoneMatrix();
}

/*SkinMeshPrimitive& SkinMeshPrimitive::operator=(const SkinMeshPrimitive& skin)
{
    MeshPrimitive::operator=(skin);
    m_boneEffectMatrix = skin.m_boneEffectMatrix;
    m_ownerNodeRootRefTransform = skin.m_ownerNodeRootRefTransform;
    return *this;
}

SkinMeshPrimitive& SkinMeshPrimitive::operator=(SkinMeshPrimitive&& skin) noexcept
{
    MeshPrimitive::operator=(skin);
    m_boneEffectMatrix = std::move(skin.m_boneEffectMatrix);
    m_ownerNodeRootRefTransform = std::move(skin.m_ownerNodeRootRefTransform);
    return *this;
}*/

GenericDto SkinMeshPrimitive::serializeDto() const
{
    SkinMeshPrimitiveDto dto(serializeMeshDto());
    return dto.toGenericDto();
}

void SkinMeshPrimitive::bindOwnerRootRefTransform(const MathLib::Matrix4& mx)
{
    m_ownerNodeRootRefTransform = mx;
}

void SkinMeshPrimitive::changeEffectMaterials(const EffectMaterialList& effects)
{
    loosePrimitiveEffectTexture();
    loosePrimitiveBoneMatrix();
    m_effects.clear();
    if (effects.size() == 0) return;
    m_effects = effects;
    bindPrimitiveEffectTexture();
    bindPrimitiveBoneMatrix();
}

void SkinMeshPrimitive::changeEffectMaterialInSegment(unsigned index, const std::shared_ptr<Engine::EffectMaterial>& effect)
{
    if (index >= m_effects.size()) return;
    looseSegmentEffectTexture(index);
    looseSegmentBoneMatrix(index);
    m_effects[index] = effect;
    bindSegmentEffectTexture(index);
    bindSegmentBoneMatrix(index);
}

void SkinMeshPrimitive::createBoneMatrixArray(unsigned size)
{
    m_boneEffectMatrix.resize(size, Matrix4::IDENTITY);
}

void SkinMeshPrimitive::updateBoneEffectMatrix(unsigned idx, const MathLib::Matrix4& ref_mx)
{
    if (idx >= m_boneEffectMatrix.size()) return;
    m_boneEffectMatrix[idx] = ref_mx;
}

void SkinMeshPrimitive::bindPrimitiveBoneMatrix()
{
    if (m_effects.empty()) return;
    EffectMaterialList::iterator eff_iter;
    for (eff_iter = m_effects.begin(); eff_iter != m_effects.end(); ++eff_iter)
    {
        if (!(*eff_iter)) continue;
        (*eff_iter)->setVariableAssignFunc(SEMANTIC_BONE_MATRIX,
            [lifetime = weak_from_this()](EffectVariable& v)
            { if (!lifetime.expired()) std::dynamic_pointer_cast<SkinMeshPrimitive, Primitive>(lifetime.lock())->assignBoneMatrix(v); });
    }
}

void SkinMeshPrimitive::bindSegmentBoneMatrix(unsigned index)
{
    if (index >= m_effects.size()) return;
    if (!m_effects[index]) return;
    m_effects[index]->setVariableAssignFunc(SEMANTIC_BONE_MATRIX,
        [lifetime = weak_from_this()](EffectVariable& v)
        { if (!lifetime.expired()) std::dynamic_pointer_cast<SkinMeshPrimitive, Primitive>(lifetime.lock())->assignBoneMatrix(v); });
}

void SkinMeshPrimitive::loosePrimitiveBoneMatrix()
{
    if (m_effects.empty()) return;
    EffectMaterialList::iterator eff_iter;
    for (eff_iter = m_effects.begin(); eff_iter != m_effects.end(); ++eff_iter)
    {
        if (!(*eff_iter)) continue;
        (*eff_iter)->setVariableAssignFunc(SEMANTIC_BONE_MATRIX, nullptr);
    }
}

void SkinMeshPrimitive::looseSegmentBoneMatrix(unsigned index)
{
    if (index >= m_effects.size()) return;
    if (!m_effects[index]) return;
    m_effects[index]->setVariableAssignFunc(SEMANTIC_BONE_MATRIX, nullptr);
}

void SkinMeshPrimitive::assignBoneMatrix(Engine::EffectVariable& var)
{
    var.assignValues(m_boneEffectMatrix, static_cast<unsigned>(m_boneEffectMatrix.size()));
}

