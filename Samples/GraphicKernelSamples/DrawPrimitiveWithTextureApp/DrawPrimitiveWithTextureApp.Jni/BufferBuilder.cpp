﻿#include "BufferBuilder.h"
#include "GraphicKernel/IVertexBuffer.h"
#include "GraphicKernel/IIndexBuffer.h"
#include <Frameworks/EventPublisher.h>
#include <GraphicKernel/GraphicEvents.h>
#include <Platforms/MemoryAllocMacro.h>
#include <Platforms/PlatformLayer.h>

using namespace Enigma::Frameworks;
using namespace Enigma::Graphics;
using namespace Enigma::Platforms;

BufferBuilder::BufferBuilder()
{
    m_onVertexBufferCreated = std::make_shared<EventSubscriber>([=](auto e) { this->OnVertexBufferCreated(e); });
    EventPublisher::Subscribe(typeid(DeviceVertexBufferCreated), m_onVertexBufferCreated);
    m_onVertexBufferResourceCreated = std::make_shared<EventSubscriber>([=](auto e) { this->OnVertexBufferResourceCreated(e); });
    EventPublisher::Subscribe(typeid(VertexBufferResourceCreated), m_onVertexBufferResourceCreated);
    m_onVertexBufferUpdated = std::make_shared<EventSubscriber>([=](auto e) { this->OnVertexBufferUpdated(e); });
    EventPublisher::Subscribe(typeid(VertexBufferResourceUpdated), m_onVertexBufferUpdated);

    m_onIndexBufferCreated = std::make_shared<EventSubscriber>([=](auto e) { this->OnIndexBufferCreated(e); });
    EventPublisher::Subscribe(typeid(DeviceIndexBufferCreated), m_onIndexBufferCreated);
    m_onIndexBufferResourceCreated = std::make_shared<EventSubscriber>([=](auto e) { this->OnIndexBufferResourceCreated(e); });
    EventPublisher::Subscribe(typeid(IndexBufferResourceCreated), m_onIndexBufferResourceCreated);
    m_onIndexBufferUpdated = std::make_shared<EventSubscriber>([=](auto e) { this->OnIndexBufferUpdated(e); });
    EventPublisher::Subscribe(typeid(IndexBufferResourceUpdated), m_onIndexBufferUpdated);
}

BufferBuilder::~BufferBuilder()
{
    EventPublisher::Unsubscribe(typeid(DeviceVertexBufferCreated), m_onVertexBufferCreated);
    m_onVertexBufferCreated = nullptr;
    EventPublisher::Unsubscribe(typeid(VertexBufferResourceCreated), m_onVertexBufferResourceCreated);
    m_onVertexBufferResourceCreated = nullptr;
    EventPublisher::Unsubscribe(typeid(VertexBufferResourceUpdated), m_onVertexBufferUpdated);
    m_onVertexBufferUpdated = nullptr;

    EventPublisher::Unsubscribe(typeid(DeviceIndexBufferCreated), m_onIndexBufferCreated);
    m_onIndexBufferCreated = nullptr;
    EventPublisher::Unsubscribe(typeid(IndexBufferResourceCreated), m_onIndexBufferResourceCreated);
    m_onIndexBufferResourceCreated = nullptr;
    EventPublisher::Unsubscribe(typeid(IndexBufferResourceUpdated), m_onIndexBufferUpdated);
    m_onIndexBufferUpdated = nullptr;
}

void BufferBuilder::BuildVertexBuffer(const std::string& name, size_t vertex_size, const byte_buffer& data_buffer)
{
    m_vtxBufferName = name;
    m_vtxSize = vertex_size;
    m_vtxDataBuffer = data_buffer;
    if (IGraphicAPI::Instance()->UseAsync())
    {
        IGraphicAPI::Instance()->AsyncCreateVertexBuffer(m_vtxBufferName);
    }
    else
    {
        IGraphicAPI::Instance()->CreateVertexBuffer(m_vtxBufferName);
    }
}

void BufferBuilder::BuildIndexBuffer(const std::string& name, const uint_buffer& data_buffer)
{
    m_idxBufferName = name;
    m_idxDataBuffer = data_buffer;
    if (IGraphicAPI::Instance()->UseAsync())
    {
        IGraphicAPI::Instance()->AsyncCreateIndexBuffer(m_idxBufferName);
    }
    else
    {
        IGraphicAPI::Instance()->CreateIndexBuffer(m_idxBufferName);
    }
}

void BufferBuilder::OnVertexBufferCreated(const IEventPtr& e)
{
    if (!e) return;
    auto ev = std::dynamic_pointer_cast<DeviceVertexBufferCreated, IEvent>(e);
    if (!ev) return;
    IVertexBufferPtr buffer = IGraphicAPI::Instance()->GetGraphicAsset<IVertexBufferPtr>(ev->GetBufferName());
    if (!buffer)
    {
        Debug::Printf("can't get vertex buffer asset %s", ev->GetBufferName().c_str());
        return;
    }
    if (IGraphicAPI::Instance()->UseAsync())
    {
        buffer->AsyncCreate(m_vtxSize, m_vtxDataBuffer.size());
    }
    else
    {
        buffer->Create(m_vtxSize, m_vtxDataBuffer.size());
    }
}

void BufferBuilder::OnVertexBufferResourceCreated(const IEventPtr& e)
{
    if (!e) return;
    auto ev = std::dynamic_pointer_cast<VertexBufferResourceCreated, IEvent>(e);
    if (!ev) return;
    if (ev->GetVertexBufferName() != m_vtxBufferName)
    {
        Debug::Printf("resource vertex buffer name not match %s", ev->GetVertexBufferName().c_str());
        return;
    }
    IVertexBufferPtr buffer = IGraphicAPI::Instance()->GetGraphicAsset<IVertexBufferPtr>(ev->GetVertexBufferName());
    if (!buffer)
    {
        Debug::Printf("can't get vertex buffer asset %s", ev->GetVertexBufferName().c_str());
        return;
    }
    if (IGraphicAPI::Instance()->UseAsync())
    {
        buffer->AsyncUpdate(m_vtxDataBuffer);
    }
    else
    {
        buffer->Update(m_vtxDataBuffer);
    }
}

void BufferBuilder::OnVertexBufferUpdated(const IEventPtr& e)
{
    if (!e) return;
    auto ev = std::dynamic_pointer_cast<VertexBufferResourceUpdated, IEvent>(e);
    if (!ev) return;
    if (ev->GetVertexBufferName() != m_vtxBufferName)
    {
        Debug::Printf("resource vertex buffer name not match %s", ev->GetVertexBufferName().c_str());
        return;
    }
    EventPublisher::Post(IEventPtr{ menew VertexBufferBuilt{ m_vtxBufferName } });
}
void BufferBuilder::OnIndexBufferCreated(const IEventPtr& e)
{
    if (!e) return;
    auto ev = std::dynamic_pointer_cast<DeviceIndexBufferCreated, IEvent>(e);
    if (!ev) return;
    IIndexBufferPtr buffer = IGraphicAPI::Instance()->GetGraphicAsset<IIndexBufferPtr>(ev->GetBufferName());
    if (!buffer)
    {
        Debug::Printf("can't get index buffer asset %s", ev->GetBufferName().c_str());
        return;
    }
    if (IGraphicAPI::Instance()->UseAsync())
    {
        buffer->AsyncCreate(m_idxDataBuffer.size() * sizeof(unsigned int));
    }
    else
    {
        buffer->Create(m_idxDataBuffer.size() * sizeof(unsigned int));
    }
}

void BufferBuilder::OnIndexBufferResourceCreated(const IEventPtr& e)
{
    if (!e) return;
    auto ev = std::dynamic_pointer_cast<IndexBufferResourceCreated, IEvent>(e);
    if (!ev) return;
    if (ev->GetIndexBufferName() != m_idxBufferName)
    {
        Debug::Printf("resource index buffer name not match %s", ev->GetIndexBufferName().c_str());
        return;
    }
    IIndexBufferPtr buffer = IGraphicAPI::Instance()->GetGraphicAsset<IIndexBufferPtr>(ev->GetIndexBufferName());
    if (!buffer)
    {
        Debug::Printf("can't get index buffer asset %s", ev->GetIndexBufferName().c_str());
        return;
    }
    if (IGraphicAPI::Instance()->UseAsync())
    {
        buffer->AsyncUpdate(m_idxDataBuffer);
    }
    else
    {
        buffer->Update(m_idxDataBuffer);
    }
}

void BufferBuilder::OnIndexBufferUpdated(const IEventPtr& e)
{
    if (!e) return;
    auto ev = std::dynamic_pointer_cast<IndexBufferResourceUpdated, IEvent>(e);
    if (!ev) return;
    if (ev->GetIndexBufferName() != m_idxBufferName)
    {
        Debug::Printf("resource index buffer name not match %s", ev->GetIndexBufferName().c_str());
        return;
    }
    EventPublisher::Post(IEventPtr{ menew IndexBufferBuilt{ m_idxBufferName } });
}

