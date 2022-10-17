﻿#include "RenderBuffer.h"
#include "EngineErrors.h"
#include "GraphicKernel/GraphicErrors.h"
#include "Platforms/PlatformLayer.h"
#include "GraphicKernel/IGraphicAPI.h"
#include <cassert>

using namespace Enigma::Engine;

RenderBuffer::RenderBuffer(const RenderBufferSignature& signature,
    const Graphics::IVertexBufferPtr& vertex_buffer, const Graphics::IIndexBufferPtr& index_buffer)
{
    m_signature = signature;
    m_vertexBuffer = vertex_buffer;
    m_indexBuffer = index_buffer;
    m_isDataEmpty = true;
}

RenderBuffer::~RenderBuffer()
{
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
}

error RenderBuffer::UpdateVertex(const byte_buffer& dataBuffer, const uint_buffer& indexBuffer)
{
    assert(m_vertexBuffer);
    if (FATAL_LOG_EXPR(dataBuffer.size() > m_vertexBuffer->BufferSize())) return Graphics::ErrorCode::bufferSize;
    if (m_indexBuffer)
    {
        unsigned int dataSize = (unsigned int)indexBuffer.size() * sizeof(unsigned int);
        if (FATAL_LOG_EXPR(dataSize > m_indexBuffer->BufferSize())) return Graphics::ErrorCode::bufferSize;
    }
    m_vertexBuffer->Update(dataBuffer);
    if (m_indexBuffer) m_indexBuffer->Update(indexBuffer);
    return ErrorCode::ok;
}

error RenderBuffer::RangedUpdateVertex(const Graphics::IVertexBuffer::ranged_buffer& vtxBuffer, 
    const std::optional<const Graphics::IIndexBuffer::ranged_buffer>& idxBuffer)
{
    assert(m_vertexBuffer);
    if (FATAL_LOG_EXPR(vtxBuffer.data.size() > m_vertexBuffer->BufferSize())) return Graphics::ErrorCode::bufferSize;
    if ((m_indexBuffer) && (idxBuffer))
    {
        unsigned int dataSize = (unsigned int)idxBuffer.value().data.size() * sizeof(unsigned int);
        if (FATAL_LOG_EXPR(dataSize > m_indexBuffer->BufferSize())) return Graphics::ErrorCode::bufferSize;
    }
    m_vertexBuffer->RangedUpdate(vtxBuffer);
    if ((m_indexBuffer) && (idxBuffer)) m_indexBuffer->RangedUpdate(idxBuffer.value());
    return ErrorCode::ok;
}

error RenderBuffer::Draw(const EffectMaterialPtr& effectMaterial,
    const GeometrySegment& segment)
{
    //if (FATAL_LOG_EXPR(!m_signature.GetVertexDeclaration())) return Graphics::ErrorCode::nullVertexLayout;
    if (FATAL_LOG_EXPR(!effectMaterial)) return ErrorCode::nullMaterial;
    if (FATAL_LOG_EXPR(!m_vertexBuffer)) return Graphics::ErrorCode::nullVertexBuffer;
    if (m_vertexBuffer)
    {
        Graphics::IGraphicAPI::Instance()->Bind(m_vertexBuffer, m_signature.GetTopology());
    }
    if (m_indexBuffer)
    {
        Graphics::IGraphicAPI::Instance()->Bind(m_indexBuffer);
    }

    effectMaterial->ApplyFirstPass();
    if (m_indexBuffer)
    {
        Graphics::IGraphicAPI::Instance()->Draw(segment.m_idxCount, segment.m_vtxCount,
            segment.m_startIdx, segment.m_startVtx);
    }
    else
    {
        Graphics::IGraphicAPI::Instance()->Draw(segment.m_vtxCount, segment.m_startVtx);
    }
    // if multi-pass effect
    while (effectMaterial->HasNextPass())
    {
        effectMaterial->ApplyNextPass();
        if (m_indexBuffer)
        {
            Graphics::IGraphicAPI::Instance()->Draw(segment.m_idxCount, segment.m_vtxCount,
                segment.m_startIdx, segment.m_startVtx);
        }
        else
        {
            Graphics::IGraphicAPI::Instance()->Draw(segment.m_vtxCount, segment.m_startVtx);
        }
    }
    return ErrorCode::ok;
}