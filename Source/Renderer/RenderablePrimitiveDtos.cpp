﻿#include "RenderablePrimitiveDtos.h"
#include "RenderablePrimitivePolicies.h"
#include "GameEngine/GeometryData.h"
#include "GameEngine/EffectMaterialDto.h"
#include "GameEngine/EffectTextureMapDto.h"
#include "Animators/ModelPrimitiveAnimator.h"
#include "Animators/AnimatorDtos.h"
#include "MeshPrimitive.h"
#include "MeshNode.h"
#include "MeshNodeTree.h"
#include "ModelPrimitive.h"
#include "SkinMeshPrimitive.h"
#include "GameEngine/GenericPolicy.h"
#include "Platforms/MemoryAllocMacro.h"
#include <cassert>

using namespace Enigma::Renderer;
using namespace Enigma::Engine;
using namespace Enigma::Animators;

static std::string TOKEN_NAME = "Name";
static std::string TOKEN_GEOMETRY_NAME = "GeometryName";
static std::string TOKEN_RAW_GEOMETRY = "RawGeometry";
static std::string TOKEN_GEOMETRY_FACTORY = "GeometryFactory";
static std::string TOKEN_EFFECTS = "Effects";
static std::string TOKEN_TEXTURE_MAPS = "TextureMaps";
static std::string TOKEN_RENDER_LIST_ID = "RenderListId";
static std::string TOKEN_LOCAL_T_POS_TRANSFORM = "LocalT_PosTransform";
static std::string TOKEN_MESH_PRIMITIVE_OBJECT = "MeshPrimitiveObject";
static std::string TOKEN_PARENT_NODE_INDEX = "ParentNodeIndex";
static std::string TOKEN_MESH_NODES = "MeshNodes";
static std::string TOKEN_MESH_NODE_TREE = "MeshNodeTree";
static std::string TOKEN_MODEL_ANIMATOR = "ModelAnimator";

MeshPrimitiveDto::MeshPrimitiveDto() : m_factoryDesc(MeshPrimitive::TYPE_RTTI.GetName()), m_geometryFactory(GeometryData::TYPE_RTTI.GetName()), m_renderListID(Renderer::RenderListID::Scene)
{
}

MeshPrimitiveDto MeshPrimitiveDto::FromGenericDto(const Engine::GenericDto& dto)
{
    MeshPrimitiveDto mesh;
    mesh.TheFactoryDesc() = dto.GetRtti();
    if (const auto v = dto.TryGetValue<std::string>(TOKEN_NAME)) mesh.Name() = v.value();
    if (const auto v = dto.TryGetValue<std::string>(TOKEN_GEOMETRY_NAME)) mesh.GeometryName() = v.value();
    if (const auto v = dto.TryGetValue<GenericDto>(TOKEN_RAW_GEOMETRY)) mesh.TheGeometry() = v.value();
    if (const auto v = dto.TryGetValue<FactoryDesc>(TOKEN_GEOMETRY_FACTORY)) mesh.GeometryFactoryDesc() = v.value();
    if (const auto ary = dto.TryGetValue<GenericDtoCollection>(TOKEN_EFFECTS))
    {
        for (auto& eff_dto : ary.value())
        {
            mesh.Effects().emplace_back(eff_dto);
        }
    }
    if (const auto ary = dto.TryGetValue<GenericDtoCollection>(TOKEN_TEXTURE_MAPS))
    {
        for (auto& tex_dto : ary.value())
        {
            mesh.TextureMaps().emplace_back(tex_dto);
        }
    }
    if (const auto v = dto.TryGetValue<unsigned>(TOKEN_RENDER_LIST_ID)) mesh.RenderListID() = static_cast<Renderer::RenderListID>(v.value());
    return mesh;
}

GenericDto MeshPrimitiveDto::ToGenericDto() const
{
    GenericDto dto;
    dto.AddRtti(m_factoryDesc);
    dto.AddOrUpdate(TOKEN_NAME, m_name);
    dto.AddOrUpdate(TOKEN_GEOMETRY_NAME, m_geometryName);
    if (m_geometry)
    {
        dto.AddOrUpdate(TOKEN_RAW_GEOMETRY, m_geometry.value());
    }
    dto.AddOrUpdate(TOKEN_GEOMETRY_FACTORY, m_geometryFactory);
    dto.AddOrUpdate(TOKEN_EFFECTS, m_effects);
    dto.AddOrUpdate(TOKEN_TEXTURE_MAPS, m_textureMaps);
    dto.AddOrUpdate(TOKEN_RENDER_LIST_ID, static_cast<unsigned>(m_renderListID));
    return dto;
}

std::shared_ptr<GenericPolicy> MeshPrimitiveDto::MeshDtoConvertToPolicy(const GenericDto& dto, const std::shared_ptr<Engine::IDtoDeserializer>& deserializer)
{
    MeshPrimitiveDto mesh_dto = MeshPrimitiveDto::FromGenericDto(dto);
    auto policy = std::make_shared<MeshPrimitivePolicy>();
    policy->Name() = mesh_dto.m_name;
    policy->TheDtoDeserializer() = deserializer;
    if (mesh_dto.m_geometry)
    {
        policy->GeometryPolicy() = GeometryDataPolicy(mesh_dto.m_geometryName, mesh_dto.m_geometry.value());
        policy->GeometryFactoryDesc() = mesh_dto.m_geometry.value().GetRtti();
    }
    else
    {
        policy->GeometryPolicy() = GeometryDataPolicy(mesh_dto.m_geometryName, mesh_dto.m_geometryFactory.GetResourceFilename(), deserializer);
        policy->GeometryFactoryDesc() = mesh_dto.m_geometryFactory;
    }
    for (auto& eff : mesh_dto.m_effects)
    {
        policy->EffectDtos().emplace_back(EffectMaterialDto::FromGenericDto(eff));
    }
    for (auto& tex : mesh_dto.m_textureMaps)
    {
        policy->TextureDtos().emplace_back(EffectTextureMapDto::FromGenericDto(tex));
    }
    policy->RenderListId() = mesh_dto.m_renderListID;
    return policy;
}

SkinMeshPrimitiveDto::SkinMeshPrimitiveDto() : MeshPrimitiveDto()
{
    m_factoryDesc = FactoryDesc(SkinMeshPrimitive::TYPE_RTTI.GetName());
}

SkinMeshPrimitiveDto::SkinMeshPrimitiveDto(const MeshPrimitiveDto& mesh) : MeshPrimitiveDto(mesh)
{
    assert(Frameworks::Rtti::IsExactlyOrDerivedFrom(m_factoryDesc.GetRttiName(), SkinMeshPrimitive::TYPE_RTTI.GetName()));
}

SkinMeshPrimitiveDto SkinMeshPrimitiveDto::FromGenericDto(const Engine::GenericDto& dto)
{
    return MeshPrimitiveDto::FromGenericDto(dto);
}

GenericDto SkinMeshPrimitiveDto::ToGenericDto() const
{
    GenericDto dto = MeshPrimitiveDto::ToGenericDto();
    return dto;
}

std::shared_ptr<GenericPolicy> SkinMeshPrimitiveDto::SkinMeshDtoConvertToPolicy(const GenericDto& dto, const std::shared_ptr<Engine::IDtoDeserializer>& deserializer)
{
    auto mesh_policy = std::dynamic_pointer_cast<MeshPrimitivePolicy, GenericPolicy>(MeshPrimitiveDto::MeshDtoConvertToPolicy(dto, deserializer));
    return std::make_shared<SkinMeshPrimitivePolicy>(*mesh_policy);
}

MeshNodeDto::MeshNodeDto() : m_factoryDesc(MeshNode::TYPE_RTTI.GetName())
{
}

MeshNodeDto MeshNodeDto::FromGenericDto(const Engine::GenericDto& dto)
{
    MeshNodeDto node;
    node.TheFactoryDesc() = dto.GetRtti();
    if (const auto v = dto.TryGetValue<std::string>(TOKEN_NAME)) node.Name() = v.value();
    if (const auto v = dto.TryGetValue<MathLib::Matrix4>(TOKEN_LOCAL_T_POS_TRANSFORM)) node.LocalT_PosTransform() = v.value();
    //if (const auto v = dto.TryGetValue<MathLib::Matrix4>(TOKEN_ROOT_REF_TRANSFORM)) node.RootRefTransform() = v.value();
    if (const auto v = dto.TryGetValue<GenericDto>(TOKEN_MESH_PRIMITIVE_OBJECT)) node.TheMeshPrimitive() = v.value();
    if (const auto v = dto.TryGetValue<unsigned>(TOKEN_PARENT_NODE_INDEX)) node.ParentIndexInArray() = v.value();
    return node;
}

GenericDto MeshNodeDto::ToGenericDto() const
{
    GenericDto dto;
    dto.AddRtti(m_factoryDesc);
    dto.AddOrUpdate(TOKEN_NAME, m_name);
    dto.AddOrUpdate(TOKEN_LOCAL_T_POS_TRANSFORM, m_localT_PosTransform);
    //dto.AddOrUpdate(TOKEN_ROOT_REF_TRANSFORM, m_rootRefTransform);
    if (m_meshPrimitive)
    {
        dto.AddOrUpdate(TOKEN_MESH_PRIMITIVE_OBJECT, m_meshPrimitive.value());
    }
    if (m_parentIndexInArray)
    {
        dto.AddOrUpdate(TOKEN_PARENT_NODE_INDEX, m_parentIndexInArray.value());
    }
    return dto;
}

MeshNodeTreeDto::MeshNodeTreeDto() : m_factoryDesc(MeshNodeTree::TYPE_RTTI.GetName())
{
}

MeshNodeTreeDto MeshNodeTreeDto::FromGenericDto(const GenericDto& dto)
{
    MeshNodeTreeDto tree;
    tree.m_factoryDesc = dto.GetRtti();
    if (const auto v = dto.TryGetValue<GenericDtoCollection>(TOKEN_MESH_NODES)) tree.MeshNodes() = v.value();
    return tree;
}

GenericDto MeshNodeTreeDto::ToGenericDto() const
{
    GenericDto dto;
    dto.AddRtti(m_factoryDesc);
    dto.AddOrUpdate(TOKEN_MESH_NODES, m_nodeDtos);
    return dto;
}

ModelPrimitiveDto::ModelPrimitiveDto() : m_factoryDesc(ModelPrimitive::TYPE_RTTI.GetName())
{
}

ModelPrimitiveDto ModelPrimitiveDto::FromGenericDto(const GenericDto& dto)
{
    ModelPrimitiveDto model;
    if (const auto v = dto.TryGetValue<std::string>(TOKEN_NAME)) model.Name() = v.value();
    if (const auto v = dto.TryGetValue<GenericDto>(TOKEN_MESH_NODE_TREE)) model.TheNodeTree() = v.value();
    if (const auto v = dto.TryGetValue<GenericDto>(TOKEN_MODEL_ANIMATOR)) model.TheAnimator() = v.value();
    return model;
}

GenericDto ModelPrimitiveDto::ToGenericDto() const
{
    GenericDto dto;
    dto.AddRtti(m_factoryDesc);
    dto.AddOrUpdate(TOKEN_NAME, m_name);
    dto.AddOrUpdate(TOKEN_MESH_NODE_TREE, m_nodeTreeDto);
    if (m_animatorDto) dto.AddOrUpdate(TOKEN_MODEL_ANIMATOR, m_animatorDto.value());
    return dto;
}

std::shared_ptr<GenericPolicy> ModelPrimitiveDto::ModelDtoConvertToPolicy(const GenericDto& dto, const std::shared_ptr<Engine::IDtoDeserializer>& deserializer)
{
    ModelPrimitiveDto model_dto = ModelPrimitiveDto::FromGenericDto(dto);
    auto policy = std::make_shared<ModelPrimitivePolicy>();
    policy->Name() = model_dto.m_name;
    policy->TheDtoDeserializer() = deserializer;
    policy->NodeTreeDto() = MeshNodeTreeDto::FromGenericDto(model_dto.m_nodeTreeDto);
    if ((model_dto.m_animatorDto) && (model_dto.m_animatorDto->GetRtti().GetRttiName() == ModelPrimitiveAnimator::TYPE_RTTI.GetName()))
    {
        policy->TheModelAnimator() = ModelAnimatorDto::FromGenericDto(model_dto.m_animatorDto.value());
    }
    return policy;
}
