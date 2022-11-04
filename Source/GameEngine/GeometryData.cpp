﻿#include "GeometryData.h"
#include "EngineErrors.h"
#include "Platforms/PlatformLayer.h"
#include <cassert>

using namespace Enigma::Engine;
using namespace Enigma::Frameworks;
using namespace Enigma::Graphics;
using namespace Enigma::MathLib;

DEFINE_RTTI_OF_BASE(Engine, GeometryData);

GeometryData::GeometryData(const std::string& name)
{
    m_name = name;
    m_vtxCapacity = m_idxCapacity = 0;
    m_vtxUsedCount = m_idxUsedCount = 0;
    m_geoSegmentVector.reserve(8);
    m_topology = PrimitiveTopology::Topology_Undefine;

    m_geometryBound = BoundingVolume{Box3::UNIT_BOX};
}

GeometryData::~GeometryData()
{

}

error GeometryData::CreateVertexCapacity(const std::string& vertex_format_string, unsigned vtx_capa, unsigned vtx_count, unsigned idx_capa, unsigned idx_count)
{
    m_vertexFormatCode.FromString(vertex_format_string);
    m_vertexFormatCode.CalculateVertexSize(&m_vertexDesc);

    if (vtx_capa)
    {
        m_vertexMemory.resize(vtx_capa * m_vertexDesc.m_totalVertexSize);
    }
    if (idx_capa)
    {
        m_indexMemory.resize(idx_capa);
    }
    m_vtxCapacity = vtx_capa;
    m_idxCapacity = idx_capa;
    m_vtxUsedCount = vtx_count;
    m_idxUsedCount = idx_count;

    m_geoSegmentVector.push_back(GeometrySegment());
    m_geoSegmentVector[0].m_startVtx = 0;
    m_geoSegmentVector[0].m_vtxCount = m_vtxUsedCount;
    m_geoSegmentVector[0].m_startIdx = 0;
    m_geoSegmentVector[0].m_idxCount = m_idxUsedCount;

    return ErrorCode::ok;
}

error GeometryData::ReSizeVertexMemoryCapacity(unsigned vtx_capa, unsigned idx_capa)
{
    if (vtx_capa)
    {
        m_vertexMemory.resize(vtx_capa * m_vertexDesc.m_totalVertexSize);
        m_vtxCapacity = vtx_capa;
    }
    if (idx_capa)
    {
        m_indexMemory.resize(idx_capa);
        m_idxCapacity = idx_capa;
    }

    return ErrorCode::ok;
}

void GeometryData::SetUsingVertexCount(unsigned vtx_count, unsigned idx_count)
{
    if (vtx_count <= m_vtxCapacity)
    {
        m_vtxUsedCount = vtx_count;
    }
    if (idx_count <= m_idxCapacity)
    {
        m_idxUsedCount = idx_count;
    }
    if (m_geoSegmentVector.size() == 1)
    {
        m_geoSegmentVector[0].m_startVtx = 0;
        m_geoSegmentVector[0].m_vtxCount = m_vtxUsedCount;
        m_geoSegmentVector[0].m_startIdx = 0;
        m_geoSegmentVector[0].m_idxCount = m_idxUsedCount;
    }
}

Vector3 GeometryData::GetPosition3(unsigned vtxIndex)
{
    Vector3 pos;
    GetVertexMemoryData(vtxIndex, m_vertexDesc.m_positionOffset, m_vertexDesc.m_posVecDimension, 3, (float*)pos, true);
    return pos;
}

Vector4 GeometryData::GetPosition4(unsigned vtxIndex)
{
    Vector4 pos;
    GetVertexMemoryData(vtxIndex, m_vertexDesc.m_positionOffset, m_vertexDesc.m_posVecDimension, 4, (float*)pos, true);
    return pos;
}

std::vector<Vector3> GeometryData::GetPosition3Array(unsigned count)
{
    return GetPosition3Array(0, count);
}

std::vector<Vector3> GeometryData::GetPosition3Array(unsigned offset, unsigned count)
{
    std::vector<Vector3> positions;
    positions.resize(count);
    GetVertexMemoryDataArray(offset, m_vertexDesc.m_positionOffset,
        m_vertexDesc.m_posVecDimension, 3, reinterpret_cast<float*>(&positions[0]), count, true);
    return positions;
}

error GeometryData::GetVertexMemoryData(unsigned vtxIndex, int elementOffset, int elementDimension,
    int destDimension, float* dest, bool isPos)
{
    assert(dest);

    if (FATAL_LOG_EXPR(vtxIndex >= m_vtxUsedCount)) return ErrorCode::invalidArrayIndex;

    if (FATAL_LOG_EXPR(m_vertexMemory.size() == 0)) return ErrorCode::nullMemoryBuffer;
    if (FATAL_LOG_EXPR(m_vertexDesc.m_totalVertexSize == 0)) return ErrorCode::zeroVertexSize;

    if (elementOffset < 0) return ErrorCode::ok;  // no need set

    unsigned int step = m_vertexDesc.m_totalVertexSize / sizeof(float);
    unsigned int base_idx = step * vtxIndex + elementOffset;

    //todo: 這邊參數的運算要看怎麼調整
    int cp_dimension = destDimension;
    if (cp_dimension > elementDimension) cp_dimension = elementDimension;
    memcpy(dest, &m_vertexMemory[base_idx * sizeof(float)], cp_dimension * sizeof(float));
    if (isPos)
    {
        if ((destDimension == 4) && (elementDimension == 3))
        {
            dest[3] = 1.0f;
        }
    }

    return ErrorCode::ok;
}

error GeometryData::SetVertexMemoryData(unsigned vtxIndex, int elementOffset, int elementDimension,
    int srcDimension, float* src, bool isPos)
{
    assert(src);

    if (FATAL_LOG_EXPR(vtxIndex >= m_vtxUsedCount)) return ErrorCode::invalidArrayIndex;

    if (FATAL_LOG_EXPR(m_vertexMemory.size() == 0)) return ErrorCode::nullMemoryBuffer;
    if (FATAL_LOG_EXPR(m_vertexDesc.m_totalVertexSize == 0)) return ErrorCode::zeroVertexSize;

    if (elementOffset < 0) return ErrorCode::ok;  // no need set

    unsigned int step = m_vertexDesc.m_totalVertexSize / sizeof(float);
    unsigned int base_idx = step * vtxIndex + elementOffset;
    int cp_dimension = srcDimension;
    if (cp_dimension > elementDimension) cp_dimension = elementDimension;
    memcpy(&m_vertexMemory[base_idx * sizeof(float)], src, cp_dimension * sizeof(float));
    if (isPos)
    {
        if ((srcDimension == 3) && (elementDimension == 4))
        {
            float* mem = (float*)(&m_vertexMemory[(base_idx + 3) * sizeof(float)]);
            *mem = 1.0f;
        }
    }

    return ErrorCode::ok;
}

error GeometryData::GetVertexMemoryDataArray(unsigned start, int elementOffset, int elementDimension,
    int destDimension, float* dest, unsigned count, bool isPos, std::function<bool()> skip_check)
{
    assert(dest);
    assert(count > 0);

    if (FATAL_LOG_EXPR(m_vertexMemory.size() == 0)) return ErrorCode::nullMemoryBuffer;
    if (FATAL_LOG_EXPR(m_vertexDesc.m_totalVertexSize == 0)) return ErrorCode::zeroVertexSize;

    if ((skip_check) && (skip_check())) return ErrorCode::ok;
    if (elementOffset < 0)
    {
        memset(dest, 0, count * destDimension * sizeof(float));
        return ErrorCode::ok;
    }

    unsigned int step = m_vertexDesc.m_totalVertexSize / sizeof(float);
    unsigned int pos_count = m_vtxUsedCount - start;
    if (count < pos_count) pos_count = count;
    float* src = (float*)&m_vertexMemory[(elementOffset + step * start) * sizeof(float)];
    int cp_dimension = destDimension;
    if (cp_dimension > elementDimension) cp_dimension = elementDimension;
    bool set_vec4 = false;
    if ((isPos) && (destDimension == 4) && (elementDimension == 3)) set_vec4 = true;
    for (unsigned int i = 0; i < pos_count; i++)
    {
        memcpy(&dest[i * destDimension], src, cp_dimension * sizeof(float));
        if (set_vec4) dest[i * destDimension + 3] = 1.0f;
        src += step;
    }

    return ErrorCode::ok;
}

error GeometryData::SetVertexMemoryDataArray(unsigned start, int elementOffset, int elementDimension,
    int srcDimension, float* src, unsigned count, bool isPos, std::function<bool()> skip_check)
{
    assert(src);
    assert(count > 0);

    if (FATAL_LOG_EXPR(m_vertexMemory.size() == 0)) return ErrorCode::nullMemoryBuffer;
    if (FATAL_LOG_EXPR(m_vertexDesc.m_totalVertexSize == 0)) return ErrorCode::zeroVertexSize;

    if ((skip_check) && (skip_check())) return ErrorCode::ok;
    if (elementOffset < 0) return ErrorCode::ok;  // no need set

    unsigned int step = m_vertexDesc.m_totalVertexSize / sizeof(float);
    unsigned int pos_count = m_vtxUsedCount - start;
    if (count < pos_count) pos_count = count;
    float* dst = (float*)&m_vertexMemory[(elementOffset + step * start) * sizeof(float)];
    int cp_dimension = srcDimension;
    if (cp_dimension > elementDimension) cp_dimension = elementDimension;
    bool get_vec4 = false;
    if ((isPos) && (srcDimension == 3) && (elementDimension == 4)) get_vec4 = true;
    for (unsigned int i = 0; i < pos_count; i++)
    {
        memcpy(dst, &src[i * srcDimension], cp_dimension * sizeof(float));
        if (get_vec4) dst[3] = 1.0f;
        dst += step;
    }

    return ErrorCode::ok;
}
