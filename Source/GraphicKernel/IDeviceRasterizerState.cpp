﻿#include "IDeviceRasterizerState.h"
#include "GraphicErrors.h"
#include "IGraphicAPI.h"
#include "GraphicThread.h"

using namespace Enigma::Graphics;

IDeviceRasterizerState::IDeviceRasterizerState(const std::string& name)
{
    m_name = name;
}

IDeviceRasterizerState::~IDeviceRasterizerState()
{
}

/*void IDeviceRasterizerState::create(const RasterizerStateData& data)
{
    if (IGraphicAPI::instance()->UseAsync())
    {
        AsyncCreateFromData(data);
    }
    else
    {
        CreateFromData(data);
    }
}*/

void IDeviceRasterizerState::Bind()
{
    if (IGraphicAPI::instance()->UseAsync())
    {
        AsyncBindToDevice();
    }
    else
    {
        BindToDevice();
    }
}

error IDeviceRasterizerState::CreateFromData(const RasterizerStateData& data)
{
    memcpy(&m_data, &data, sizeof(RasterizerStateData));
    return ErrorCode::ok;
}

future_error IDeviceRasterizerState::AsyncCreateFromData(const RasterizerStateData& data)
{
    return IGraphicAPI::instance()->GetGraphicThread()->
        PushTask([lifetime = shared_from_this(), data, this]() -> error { return CreateFromData(data); });
}

future_error IDeviceRasterizerState::AsyncBindToDevice()
{
    return IGraphicAPI::instance()->GetGraphicThread()->
        PushTask([lifetime = shared_from_this(), this]() -> error { return BindToDevice(); });
}
