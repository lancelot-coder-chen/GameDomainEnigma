﻿#include "SceneGraphDtos.h"
#include "LazyNode.h"
#include "Node.h"
#include "GameEngine/BoundingVolumeDto.h"
#include "Spatial.h"
#include "Light.h"
#include "Pawn.h"
#include "SceneGraphPolicies.h"
#include "VisibilityManagedNode.h"
#include "GameEngine/DtoDeserializer.h"
#include <cassert>

using namespace Enigma::SceneGraph;
using namespace Enigma::MathLib;
using namespace Enigma::Engine;

static std::string TOKEN_NAME = "Name";
static std::string TOKEN_ID_NAME = "Id.Name";
static std::string TOKEN_ID_RTTI = "Id.Rtti";
static std::string TOKEN_PARENT_NAME = "ParentName";
static std::string TOKEN_LOCAL_TRANSFORM = "LocalTransform";
static std::string TOKEN_WORLD_TRANSFORM = "WorldTransform";
static std::string TOKEN_GRAPH_DEPTH = "GraphDepth";
static std::string TOKEN_WORLD_BOUND = "WorldBound";
static std::string TOKEN_MODEL_BOUND = "ModelBound";
static std::string TOKEN_CULLING_MODE = "CullingMode";
static std::string TOKEN_SPATIAL_FLAG = "SpatialFlag";
static std::string TOKEN_NOTIFY_FLAG = "NotifyFlag";
static std::string TOKEN_CHILD_NAMES = "ChildNames";
static std::string TOKEN_LIGHT_INFO = "LightInfo";
static std::string TOKEN_PAWN_PRIMITIVE_ID_NAME = "PawnPrimitiveId.Name";
static std::string TOKEN_PAWN_PRIMITIVE_ID_SEQUENCE = "PawnPrimitiveId.Sequence";
static std::string TOKEN_PAWN_PRIMITIVE_ID_RTTI = "PawnPrimitiveId.Rtti";
//static std::string TOKEN_PRIMITIVE_FACTORY = "PrimitiveFactory";

SpatialDto::SpatialDto() : m_factoryDesc(Spatial::TYPE_RTTI.getName()), m_isTopLevel(false), m_graphDepth(0), m_cullingMode(0), m_spatialFlag(0), m_notifyFlag(0)
{
    m_localTransform = Matrix4::IDENTITY;
    m_worldTransform = Matrix4::IDENTITY;
    BoundingVolume bv{ Box3::UNIT_BOX };
    m_modelBound = bv.serializeDto().toGenericDto();
    m_worldBound = bv.serializeDto().toGenericDto();
}

SpatialDto::SpatialDto(const Engine::GenericDto& dto) : m_factoryDesc(Spatial::TYPE_RTTI.getName()), m_isTopLevel(false), m_graphDepth(0), m_cullingMode(0), m_spatialFlag(0), m_notifyFlag(0)
{
    factoryDesc() = dto.getRtti();
    m_isTopLevel = dto.isTopLevel();
    if (auto v = dto.tryGetValue<std::string>(TOKEN_NAME)) name() = v.value();
    if (auto n = dto.tryGetValue<std::string>(TOKEN_ID_NAME))
    {
        if (auto r = dto.tryGetValue<std::string>(TOKEN_ID_RTTI))
        {
            id() = SpatialId(n.value(), Frameworks::Rtti::fromName(r.value()));
        }
    }
    if (auto v = dto.tryGetValue<std::string>(TOKEN_PARENT_NAME)) parentName() = v.value();
    if (auto v = dto.tryGetValue<Matrix4>(TOKEN_LOCAL_TRANSFORM)) localTransform() = v.value();
    if (auto v = dto.tryGetValue<Matrix4>(TOKEN_WORLD_TRANSFORM)) worldTransform() = v.value();
    if (auto v = dto.tryGetValue<unsigned int>(TOKEN_GRAPH_DEPTH)) graphDepth() = v.value();
    if (auto v = dto.tryGetValue<GenericDto>(TOKEN_MODEL_BOUND)) modelBound() = v.value();
    if (auto v = dto.tryGetValue<GenericDto>(TOKEN_WORLD_BOUND)) worldBound() = v.value();
    if (auto v = dto.tryGetValue<unsigned int>(TOKEN_CULLING_MODE)) cullingMode() = v.value();
    if (auto v = dto.tryGetValue<unsigned int>(TOKEN_SPATIAL_FLAG)) spatialFlag() = v.value();
    if (auto v = dto.tryGetValue<unsigned int>(TOKEN_NOTIFY_FLAG)) notifyFlag() = v.value();
}

GenericDto SpatialDto::toGenericDto() const
{
    GenericDto dto;
    dto.addRtti(m_factoryDesc);
    dto.asTopLevel(m_isTopLevel);
    dto.addOrUpdate(TOKEN_NAME, m_name);
    dto.addOrUpdate(TOKEN_ID_NAME, m_id.name());
    dto.addOrUpdate(TOKEN_ID_RTTI, m_id.rtti().getName());
    if (!m_parentName.empty()) dto.addOrUpdate(TOKEN_PARENT_NAME, m_parentName);
    dto.addOrUpdate(TOKEN_LOCAL_TRANSFORM, m_localTransform);
    dto.addOrUpdate(TOKEN_WORLD_TRANSFORM, m_worldTransform);
    dto.addOrUpdate(TOKEN_GRAPH_DEPTH, m_graphDepth);
    dto.addOrUpdate(TOKEN_MODEL_BOUND, m_modelBound);
    dto.addOrUpdate(TOKEN_WORLD_BOUND, m_worldBound);
    dto.addOrUpdate(TOKEN_CULLING_MODE, m_cullingMode);
    dto.addOrUpdate(TOKEN_SPATIAL_FLAG, m_spatialFlag);
    dto.addOrUpdate(TOKEN_NOTIFY_FLAG, m_notifyFlag);
    return dto;
}

NodeDto::NodeDto() : SpatialDto()
{
    m_factoryDesc = FactoryDesc(Node::TYPE_RTTI.getName());
}

NodeDto::NodeDto(const SpatialDto& spatial_dto) : SpatialDto(spatial_dto)
{
    assert(Frameworks::Rtti::isExactlyOrDerivedFrom(m_factoryDesc.GetRttiName(), Node::TYPE_RTTI.getName()));
}

NodeDto::NodeDto(const Engine::GenericDto& dto) : SpatialDto(dto)
{
    assert(Frameworks::Rtti::isExactlyOrDerivedFrom(m_factoryDesc.GetRttiName(), Node::TYPE_RTTI.getName()));
    if (auto v = dto.tryGetValue<std::vector<std::string>>(TOKEN_CHILD_NAMES)) m_childNames = v.value();
}

/*NodeDto NodeDto::fromGenericDto(const GenericDto& dto)
{
    NodeDto node_dto{ SpatialDto(dto) };
    return node_dto;
}*/

GenericDto NodeDto::toGenericDto() const
{
    GenericDto dto = SpatialDto::toGenericDto();
    dto.addOrUpdate(TOKEN_CHILD_NAMES, m_childNames);

    return dto;
}

LightDto::LightDto() : SpatialDto()
{
    m_factoryDesc = FactoryDesc(Light::TYPE_RTTI.getName());
}

LightDto::LightDto(const SpatialDto& spatial_dto) : SpatialDto(spatial_dto)
{
    assert(Frameworks::Rtti::isExactlyOrDerivedFrom(m_factoryDesc.GetRttiName(), Light::TYPE_RTTI.getName()));
}

LightDto::LightDto(const Engine::GenericDto& dto) : SpatialDto(dto)
{
    assert(Frameworks::Rtti::isExactlyOrDerivedFrom(m_factoryDesc.GetRttiName(), Light::TYPE_RTTI.getName()));
    if (auto v = dto.tryGetValue<GenericDto>(TOKEN_LIGHT_INFO)) m_lightInfo = v.value();
}

/*LightDto LightDto::fromGenericDto(const Engine::GenericDto& dto)
{
    LightDto light_dto{ SpatialDto(dto) };
    if (auto v = dto.tryGetValue<GenericDto>(TOKEN_LIGHT_INFO)) light_dto.m_lightInfo = v.value();
    return light_dto;
}*/

GenericDto LightDto::toGenericDto() const
{
    GenericDto dto = SpatialDto::toGenericDto();
    dto.addOrUpdate(TOKEN_LIGHT_INFO, m_lightInfo);

    return dto;
}

PawnDto::PawnDto() : SpatialDto()
{
    m_factoryDesc = FactoryDesc(Pawn::TYPE_RTTI.getName());
}

PawnDto::PawnDto(const SpatialDto& spatial_dto) : SpatialDto(spatial_dto)
{
    assert(Frameworks::Rtti::isExactlyOrDerivedFrom(m_factoryDesc.GetRttiName(), Pawn::TYPE_RTTI.getName()));
}

PawnDto::PawnDto(const Engine::GenericDto& dto) : SpatialDto(dto)
{
    assert(Frameworks::Rtti::isExactlyOrDerivedFrom(m_factoryDesc.GetRttiName(), Pawn::TYPE_RTTI.getName()));
    if (auto n = dto.tryGetValue<std::string>(TOKEN_PAWN_PRIMITIVE_ID_NAME))
    {
        if (auto r = dto.tryGetValue<std::string>(TOKEN_PAWN_PRIMITIVE_ID_RTTI))
        {
            std::uint64_t id_seq = 0;
            if (auto s = dto.tryGetValue<std::uint64_t>(TOKEN_PAWN_PRIMITIVE_ID_SEQUENCE))
            {
                id_seq = s.value();
            }
            primitiveId() = PrimitiveId(n.value(), id_seq, Frameworks::Rtti::fromName(r.value()));
        }
    }
    //if (auto v = dto.tryGetValue<FactoryDesc>(TOKEN_PRIMITIVE_FACTORY)) m_primitiveFactory = v.value();
}

/*PawnDto PawnDto::fromGenericDto(const Engine::GenericDto& dto)
{
    PawnDto pawn_dto{ SpatialDto(dto) };
    if (auto v = dto.tryGetValue<GenericDto>(TOKEN_PAWN_PRIMITIVE)) pawn_dto.m_primitive = v.value();
    //if (auto v = dto.tryGetValue<FactoryDesc>(TOKEN_PRIMITIVE_FACTORY)) pawn_dto.m_primitiveFactory = v.value();
    return pawn_dto;
}*/

GenericDto PawnDto::toGenericDto() const
{
    GenericDto dto = SpatialDto::toGenericDto();
    if (m_primitiveId)
    {
        dto.addOrUpdate(TOKEN_PAWN_PRIMITIVE_ID_NAME, m_primitiveId.value().name());
        dto.addOrUpdate(TOKEN_PAWN_PRIMITIVE_ID_SEQUENCE, m_primitiveId.value().sequence());
        dto.addOrUpdate(TOKEN_PAWN_PRIMITIVE_ID_RTTI, primitiveId().value().rtti().getName());
    }
    //dto.addOrUpdate(TOKEN_PRIMITIVE_FACTORY, m_primitiveFactory);
    return dto;
}

/*std::shared_ptr<PawnPolicy> PawnDto::convertToPolicy(const std::shared_ptr<Engine::IDtoDeserializer>& deserializer)
{
    if (m_primitive)
    {
        return std::make_shared<PawnPolicy>(m_name, m_primitive.value());
    }
    //todo : prefab support
    /*else
    {
        return std::make_shared<PawnPolicy>(m_name, m_primitiveFactory.GetPrefab(), deserializer);
    }*/
    /*return nullptr;
}*/

LazyNodeDto::LazyNodeDto() : NodeDto()
{
    m_factoryDesc = FactoryDesc(LazyNode::TYPE_RTTI.getName());
}

LazyNodeDto::LazyNodeDto(const NodeDto& node_dto) : NodeDto(node_dto)
{
    assert(Frameworks::Rtti::isExactlyOrDerivedFrom(m_factoryDesc.GetRttiName(), LazyNode::TYPE_RTTI.getName()));
}

LazyNodeDto::LazyNodeDto(const Engine::GenericDto& dto) : NodeDto(dto)
{
}

/*LazyNodeDto LazyNodeDto::fromGenericDto(const Engine::GenericDto& dto)
{
    LazyNodeDto node_dto{ NodeDto(dto) };
    return node_dto;
}*/

GenericDto LazyNodeDto::toGenericDto() const
{
    GenericDto dto = NodeDto::toGenericDto();
    return dto;
}

VisibilityManagedNodeDto::VisibilityManagedNodeDto() : LazyNodeDto()
{
    m_factoryDesc = FactoryDesc(VisibilityManagedNode::TYPE_RTTI.getName());
}

VisibilityManagedNodeDto::VisibilityManagedNodeDto(const LazyNodeDto& lazy_node_dto) : LazyNodeDto(lazy_node_dto)
{
    assert(Frameworks::Rtti::isExactlyOrDerivedFrom(m_factoryDesc.GetRttiName(), VisibilityManagedNode::TYPE_RTTI.getName()));
}

VisibilityManagedNodeDto::VisibilityManagedNodeDto(const Engine::GenericDto& dto) : LazyNodeDto(dto)
{
}

/*VisibilityManagedNodeDto VisibilityManagedNodeDto::fromGenericDto(const Engine::GenericDto& dto)
{
    VisibilityManagedNodeDto node_dto(LazyNodeDto::fromGenericDto(dto));
    return node_dto;
}*/

GenericDto VisibilityManagedNodeDto::toGenericDto() const
{
    GenericDto dto = LazyNodeDto::toGenericDto();
    return dto;
}
