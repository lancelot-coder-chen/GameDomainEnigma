﻿#include "IndexBufferEgl.h"
#include "GraphicAPIEgl.h"
#include "GraphicKernel/GraphicErrors.h"
#include "GraphicKernel/GraphicEvents.h"
#include "Frameworks/EventPublisher.h"
#include "Platforms/MemoryMacro.h"
#include "Platforms/PlatformLayer.h"
#include <cassert>

using namespace Enigma::Devices;
using ErrorCode = Enigma::Graphics::ErrorCode;

IndexBufferEgl::IndexBufferEgl(const std::string& name) :IIndexBuffer(name)
{
    m_bufferHandle = 0;
}

IndexBufferEgl::~IndexBufferEgl()
{
    if (m_bufferHandle)
    {
        glDeleteBuffers(1, &m_bufferHandle);
        m_bufferHandle = 0;
    }
}

error IndexBufferEgl::Create(unsigned sizeBuffer)
{
    m_bufferSize = sizeBuffer;
    assert(m_bufferSize > 0);
    if (m_bufferHandle)
    {
        glDeleteBuffers(1, &m_bufferHandle);
        m_bufferHandle = 0;
    }
    glGenBuffers(1, &m_bufferHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferHandle);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_bufferSize, 0, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    Graphics::IGraphicAPI::Instance()->BindIndexBuffer(nullptr); // gl state reset, 要清掉binder裡的 cache

    Frameworks::EventPublisher::Post(Frameworks::IEventPtr{ menew Graphics::IndexBufferResourceCreated(m_name) });
    return ErrorCode::ok;
}

error IndexBufferEgl::Update(const uint_buffer& dataIndex)
{
    assert(!dataIndex.empty());
    assert(m_bufferHandle != 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferHandle);
    unsigned int dataSize = (unsigned int)dataIndex.size() * sizeof(unsigned int);
    void* buff = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, dataSize, GL_MAP_WRITE_BIT);
    if (!buff) return ErrorCode::eglBufferMapping;

    memcpy(buff, &dataIndex[0], dataSize);

    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    Graphics::IGraphicAPI::Instance()->BindIndexBuffer(nullptr); // gl state reset, 要清掉binder裡的 cache

    Frameworks::EventPublisher::Post(Frameworks::IEventPtr{ menew Graphics::IndexBufferResourceUpdated(m_name) });
    return ErrorCode::ok;
}

error IndexBufferEgl::RangedUpdate(const ranged_buffer& buffer)
{
    assert(!buffer.data.empty());
    assert(m_bufferHandle != 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferHandle);
    unsigned int dataSize = (unsigned int)buffer.data.size();
    void* buff = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, buffer.idx_offset * sizeof(unsigned int), dataSize, GL_MAP_WRITE_BIT);
    if (!buff) return ErrorCode::eglBufferMapping;

    memcpy(buff, &buffer.data[0], dataSize);

    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    Graphics::IGraphicAPI::Instance()->BindIndexBuffer(nullptr); // gl state reset, 要清掉binder裡的 cache

    Frameworks::EventPublisher::Post(Frameworks::IEventPtr{ menew Graphics::IndexBufferResourceUpdated(m_name) });
    return ErrorCode::ok;
}