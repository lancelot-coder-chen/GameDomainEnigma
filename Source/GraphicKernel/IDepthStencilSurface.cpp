﻿#include "IDepthStencilSurface.h"
#include "IGraphicAPI.h"
#include "GraphicThread.h"

using namespace Enigma::Graphics;

future_error IDepthStencilSurface::AsyncResize(const MathLib::Dimension& dimension)
{
    return IGraphicAPI::Instance()->GetGraphicThread()->
        PushTask([lifetime = shared_from_this(), dimension = dimension, this]() -> error { return Resize(dimension); });
}
