﻿#include "IIndexBuffer.h"
#include "IGraphicAPI.h"
#include "GraphicThread.h"

using namespace Enigma::Graphics;

IIndexBuffer::IIndexBuffer(const std::string& name)
{
    m_name = name;
    m_bufferSize = 0;
}

IIndexBuffer::~IIndexBuffer()
{
}

void IIndexBuffer::Update(const uint_buffer& dataIndex)
{
    if (IGraphicAPI::Instance()->UseAsync())
    {
        IGraphicAPI::Instance()->GetGraphicThread()->
            PushTask([lifetime = shared_from_this(), dataIndex, this]() -> error { return UpdateBuffer(dataIndex); });
    }
    else
    {
        UpdateBuffer(dataIndex);
    }
}

void IIndexBuffer::RangedUpdate(const ranged_buffer& buffer)
{
    if (IGraphicAPI::Instance()->UseAsync())
    {
        IGraphicAPI::Instance()->GetGraphicThread()->
            PushTask([lifetime = shared_from_this(), buffer, this]() -> error { return RangedUpdateBuffer(buffer); });
    }
    else
    {
        RangedUpdateBuffer(buffer);
    }
}
