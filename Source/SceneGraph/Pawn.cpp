﻿#include "Pawn.h"
#include "Culler.h"
#include "SceneGraphErrors.h"
#include "GameEngine/BoundingVolume.h"
#include "SceneGraphDtos.h"
#include <cassert>

using namespace Enigma::SceneGraph;

DEFINE_RTTI(SceneGraph, Pawn, Spatial);

Pawn::Pawn(const std::string& name) : Spatial(name)
{
    m_factoryDesc = Engine::FactoryDesc(Pawn::TYPE_RTTI.getName());
    m_primitive = nullptr;
    removeSpatialFlag(Spatial_Unlit);
}

Pawn::Pawn(const Engine::GenericDto& dto) : Spatial(dto)
{
}

Pawn::~Pawn()
{
    m_primitive = nullptr;
}

Enigma::Engine::GenericDto Pawn::serializeDto()
{
    return SerializePawnDto().toGenericDto();
}

PawnDto Pawn::SerializePawnDto()
{
    PawnDto dto(serializeSpatialDto());
    if (m_primitive)
    {
        dto.primitive() = m_primitive->serializeDto();
    }
    return dto;
}

error Pawn::onCullingVisible(Culler* culler, bool noCull)
{
    assert(culler);
    culler->Insert(thisSpatial());
    return ErrorCode::ok;
}

bool Pawn::isRenderable()
{
    if (m_primitive) return true;
    return false;
}

error Pawn::insertToRenderer(const Engine::IRendererPtr& render)
{
    assert(render);
    if (!m_primitive) return ErrorCode::nullPrimitive;

    error er = m_primitive->InsertToRendererWithTransformUpdating(render, m_mxWorldTransform, m_spatialRenderState.ToLightingState());
    return er;
}

void Pawn::SetPrimitive(const Engine::PrimitivePtr& prim)
{
    m_primitive = prim;

    // update local bound, world bound
    if ((m_primitive) && (!m_primitive->GetBoundingVolume().IsEmpty()))
    {
        m_modelBound = Engine::BoundingVolume{ m_primitive->GetBoundingVolume() };
    }

    _updateBoundData();
    _propagateSpatialRenderState();
}

void Pawn::CalculateModelBound(bool axis_align)
{
    if (m_primitive)
    {
        m_primitive->CalculateBoundingVolume(axis_align);
    }
    // update local bound, world bound
    if ((m_primitive) && (!m_primitive->GetBoundingVolume().IsEmpty()))
    {
        m_modelBound = Engine::BoundingVolume{ m_primitive->GetBoundingVolume() };
    }

    _updateBoundData();
}

error Pawn::_updateLocalTransform(const MathLib::Matrix4& mxLocal)
{
    error er = Spatial::_updateLocalTransform(mxLocal);
    if (m_primitive) m_primitive->UpdateWorldTransform(m_mxWorldTransform);
    return er;
}

error Pawn::_updateWorldData(const MathLib::Matrix4& mxParentWorld)
{
    error er = Spatial::_updateWorldData(mxParentWorld);
    if (m_primitive) m_primitive->UpdateWorldTransform(m_mxWorldTransform);
    return er;
}

void Pawn::EnumAnimatorListDeep(std::list<std::shared_ptr<Engine::Animator>>& resultList)
{
    if (m_primitive) m_primitive->EnumAnimatorListDeep(resultList);
}
