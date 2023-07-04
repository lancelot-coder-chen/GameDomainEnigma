﻿#include "TerrainPrimitivePolicy.h"
#include "TerrainPrimitive.h"

using namespace Enigma::Terrain;

TerrainPrimitivePolicy::TerrainPrimitivePolicy() : MeshPrimitivePolicy()
{
}

TerrainPrimitivePolicy::TerrainPrimitivePolicy(const MeshPrimitivePolicy& policy) : MeshPrimitivePolicy(policy)
{
}

std::shared_ptr<Enigma::Renderer::MeshPrimitive> TerrainPrimitivePolicy::CreatePrimitive() const
{
    return std::make_shared<TerrainPrimitive>(m_name);
}

