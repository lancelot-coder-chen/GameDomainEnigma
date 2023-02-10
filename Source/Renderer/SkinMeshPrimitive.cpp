﻿#include "SkinMeshPrimitive.h"
#include "RenderablePrimitiveDtos.h"

using namespace Enigma::Renderer;
using namespace Enigma::Engine;
using namespace Enigma::MathLib;
using namespace Enigma::Frameworks;

const std::string SEMANTIC_BONE_MATRIX = "BoneMatrix";

DEFINE_RTTI(Renderer, SkinMeshPrimitive, MeshPrimitive);

SkinMeshPrimitive::SkinMeshPrimitive(const std::string& name) : MeshPrimitive(name)
{
    m_ownerNodeRootRefTransform = Matrix4::IDENTITY;
}

SkinMeshPrimitive::SkinMeshPrimitive(const SkinMeshPrimitive& skin) : MeshPrimitive(skin)
{
    m_boneEffectMatrix = skin.m_boneEffectMatrix;
    m_ownerNodeRootRefTransform = skin.m_ownerNodeRootRefTransform;
}

SkinMeshPrimitive::SkinMeshPrimitive(SkinMeshPrimitive&& skin) : MeshPrimitive(skin)
{
    m_boneEffectMatrix = std::move(skin.m_boneEffectMatrix);
    m_ownerNodeRootRefTransform = std::move(skin.m_ownerNodeRootRefTransform);
}

SkinMeshPrimitive::~SkinMeshPrimitive()
{
    LoosePrimitiveBoneMatrix();
}

SkinMeshPrimitive& SkinMeshPrimitive::operator=(const SkinMeshPrimitive& skin)
{
    MeshPrimitive::operator=(skin);
    m_boneEffectMatrix = skin.m_boneEffectMatrix;
    m_ownerNodeRootRefTransform = skin.m_ownerNodeRootRefTransform;
    return *this;
}

SkinMeshPrimitive& SkinMeshPrimitive::operator=(SkinMeshPrimitive&& skin)
{
    MeshPrimitive::operator=(skin);
    m_boneEffectMatrix = std::move(skin.m_boneEffectMatrix);
    m_ownerNodeRootRefTransform = std::move(skin.m_ownerNodeRootRefTransform);
    return *this;
}

GenericDto SkinMeshPrimitive::SerializeDto()
{
    SkinMeshPrimitiveDto dto(SerializeMeshDto());

    return dto.ToGenericDto();
}

void SkinMeshPrimitive::BindOwnerMeshNode(const MeshNode& node)
{
    m_ownerNodeRootRefTransform = node.GetRootRefTransform();
}

void SkinMeshPrimitive::ChangeEffectMaterial(const EffectMaterialList& effects)
{
    LoosePrimitiveEffectTexture();
    LoosePrimitiveBoneMatrix();
    m_effects.clear();
    if (effects.size() == 0) return;
    m_effects = effects;
    BindPrimitiveEffectTexture();
    BindPrimitiveBoneMatrix();
}

void SkinMeshPrimitive::ChangeEffectMaterialInSegment(unsigned index, const Engine::EffectMaterialPtr& effect)
{
    if (index >= m_effects.size()) return;
    LooseSegmentEffectTexture(index);
    LooseSegmentBoneMatrix(index);
    m_effects[index] = effect;
    BindSegmentEffectTexture(index);
    BindSegmentBoneMatrix(index);
}

void SkinMeshPrimitive::CreateBoneMatrixArray(unsigned size)
{
    m_boneEffectMatrix.resize(size, Matrix4::IDENTITY);
}

void SkinMeshPrimitive::UpdateBoneEffectMatrix(unsigned idx, const MathLib::Matrix4& ref_mx)
{
    if (idx >= m_boneEffectMatrix.size()) return;
    m_boneEffectMatrix[idx] = ref_mx;
}

void SkinMeshPrimitive::BindPrimitiveBoneMatrix()
{
    if (m_effects.empty()) return;
    EffectMaterialList::iterator eff_iter;
    for (eff_iter = m_effects.begin(); eff_iter != m_effects.end(); ++eff_iter)
    {
        if (!(*eff_iter)) continue;
        (*eff_iter)->SetVariableAssignFunc(SEMANTIC_BONE_MATRIX,
            [lifetime = weak_from_this()](EffectVariable& v)
            { if (!lifetime.expired()) std::dynamic_pointer_cast<SkinMeshPrimitive, Primitive>(lifetime.lock())->BoneMatrixAssign(v); });
    }
}

void SkinMeshPrimitive::BindSegmentBoneMatrix(unsigned index)
{
    if (index >= m_effects.size()) return;
    if (!m_effects[index]) return;
    m_effects[index]->SetVariableAssignFunc(SEMANTIC_BONE_MATRIX,
        [lifetime = weak_from_this()](EffectVariable& v)
        { if (!lifetime.expired()) std::dynamic_pointer_cast<SkinMeshPrimitive, Primitive>(lifetime.lock())->BoneMatrixAssign(v); });
}

void SkinMeshPrimitive::LoosePrimitiveBoneMatrix()
{
    if (m_effects.empty()) return;
    EffectMaterialList::iterator eff_iter;
    for (eff_iter = m_effects.begin(); eff_iter != m_effects.end(); ++eff_iter)
    {
        if (!(*eff_iter)) continue;
        (*eff_iter)->SetVariableAssignFunc(SEMANTIC_BONE_MATRIX, nullptr);
    }
}

void SkinMeshPrimitive::LooseSegmentBoneMatrix(unsigned index)
{
    if (index >= m_effects.size()) return;
    if (!m_effects[index]) return;
    m_effects[index]->SetVariableAssignFunc(SEMANTIC_BONE_MATRIX, nullptr);
}

void SkinMeshPrimitive::BoneMatrixAssign(Engine::EffectVariable& var)
{
    var.AssignValues(m_boneEffectMatrix, static_cast<unsigned>(m_boneEffectMatrix.size()));
}
