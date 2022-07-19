﻿#include "MultiBackSurfaceEgl.h"
#include "GraphicKernel/GraphicErrors.h"

#if TARGET_PLATFORM == PLATFORM_ANDROID

using namespace Enigma::Devices;
using ErrorCode = Enigma::Graphics::ErrorCode;

MultiBackSurfaceEgl::MultiBackSurfaceEgl(const std::string& name, const MathLib::Dimension& dimension,
    unsigned buffer_count, const std::vector<Graphics::GraphicFormat>& fmt) : IMultiBackSurface(name)
{
    m_dimension = dimension;
    m_formatArray = fmt;
    m_surfaceCount = buffer_count;
    m_format = fmt[0];
    m_frameBufferHandle = 0;
    CreateFrameBufferTexture(m_dimension, m_format);
}

MultiBackSurfaceEgl::~MultiBackSurfaceEgl()
{
    if (m_frameBufferHandle != 0)
    {
        glDeleteFramebuffers(1, &m_frameBufferHandle);
    }
}

error MultiBackSurfaceEgl::Resize(const MathLib::Dimension& dimension)
{
    m_dimension = dimension;
    return ErrorCode::ok;
}

void MultiBackSurfaceEgl::CreateFrameBufferTexture(const MathLib::Dimension& dimension, const Graphics::GraphicFormat& fmt)
{
    glGenFramebuffers(1, &m_frameBufferHandle);
}

#endif