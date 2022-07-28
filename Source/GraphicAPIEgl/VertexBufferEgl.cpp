﻿#include "VertexBufferEgl.h"
#include "GraphicAPIEgl.h"
#include "GraphicKernel/GraphicErrors.h"
#include "GraphicKernel/GraphicEvents.h"
#include "Frameworks/EventPublisher.h"
#include "Platforms/MemoryMacro.h"
#include "Platforms/PlatformLayer.h"
#include <cassert>

using namespace Enigma::Devices;
using ErrorCode = Enigma::Graphics::ErrorCode;

VertexBufferEgl::VertexBufferEgl(const std::string& name) : IVertexBuffer(name)
{
    m_bufferHandle = 0;
}

VertexBufferEgl::~VertexBufferEgl()
{
    if (m_bufferHandle)
    {
        glDeleteBuffers(1, &m_bufferHandle);
    }
}

error VertexBufferEgl::Create(unsigned sizeofVertex, unsigned sizeBuffer)
{
    if (m_bufferHandle)
    {
        glDeleteBuffers(1, &m_bufferHandle);
        m_bufferHandle = 0;
    }
    m_sizeofVertex = sizeofVertex;
    m_bufferSize = sizeBuffer;
    assert(m_bufferSize > 0);
    glGenBuffers(1, &m_bufferHandle);
    glBindBuffer(GL_ARRAY_BUFFER, m_bufferHandle);

    glBufferData(GL_ARRAY_BUFFER, m_bufferSize, 0, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    Graphics::IGraphicAPI::Instance()->BindVertexBuffer(nullptr, Graphics::PrimitiveTopology::Topology_Undefine); // gl state reset, 要清掉binder裡的 cache

    Frameworks::EventPublisher::Post(Frameworks::IEventPtr{ menew Graphics::VertexBufferResourceCreated(m_name) });
    return ErrorCode::ok;
}

error VertexBufferEgl::Update(const byte_buffer& dataVertex)
{
    assert(m_bufferHandle != 0);
    assert(!dataVertex.empty());
    if (FATAL_LOG_EXPR(dataVertex.size() > m_bufferSize)) return ErrorCode::bufferSize;
    glBindBuffer(GL_ARRAY_BUFFER, m_bufferHandle);
    void* buff = glMapBufferRange(GL_ARRAY_BUFFER, 0, dataVertex.size(), GL_MAP_WRITE_BIT);
    if (!buff) return ErrorCode::eglBufferMapping;

    memcpy(buff, &dataVertex[0], dataVertex.size());

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    Graphics::IGraphicAPI::Instance()->BindVertexBuffer(nullptr, Graphics::PrimitiveTopology::Topology_Undefine); // gl state reset, 要清掉binder裡的 cache

    Frameworks::EventPublisher::Post(Frameworks::IEventPtr{ menew Graphics::VertexBufferResourceUpdated(m_name) });
    return ErrorCode::ok;
}

error VertexBufferEgl::RangedUpdate(const ranged_buffer& buffer)
{
    assert(m_bufferHandle != 0);
    assert(!buffer.data.empty());
    if (FATAL_LOG_EXPR(buffer.data.size() > m_bufferSize)) return ErrorCode::bufferSize;
    glBindBuffer(GL_ARRAY_BUFFER, m_bufferHandle);
    void* buff = glMapBufferRange(GL_ARRAY_BUFFER, buffer.vtx_offset * m_sizeofVertex,
        buffer.data.size(), GL_MAP_WRITE_BIT);
    if (!buff) return ErrorCode::eglBufferMapping;

    memcpy(buff, &buffer.data[0], buffer.data.size());

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    Graphics::IGraphicAPI::Instance()->BindVertexBuffer(nullptr, Graphics::PrimitiveTopology::Topology_Undefine); // gl state reset, 要清掉binder裡的 cache

    Frameworks::EventPublisher::Post(Frameworks::IEventPtr{
        menew Graphics::VertexBufferResourceRangedUpdated(m_name, buffer.vtx_offset, buffer.vtx_count) });
    return ErrorCode::ok;
}