﻿#include "MultiTextureEgl.h"
#include "GraphicAPIEgl.h"
#include "MultiBackSurfaceEgl.h"
#include "Platforms/MemoryMacro.h"
#include "Platforms/PlatformLayer.h"
#include "Frameworks/EventPublisher.h"
#include "GraphicKernel/GraphicEvents.h"
#include <cassert>
#if TARGET_PLATFORM == PLATFORM_ANDROID
#include "png.h"

using namespace Enigma::Devices;
using ErrorCode = Enigma::Graphics::ErrorCode;

MultiTextureEgl::MultiTextureEgl(const std::string& name) : IMultiTexture(name)
{
}

MultiTextureEgl::~MultiTextureEgl()
{
    if (!m_textures.empty())
    {
        glDeleteTextures(m_textures.size(), &m_textures[0]);
    }
}

error MultiTextureEgl::LoadTextureImages(const std::vector<byte_buffer>& img_buffs)
{
    assert(!img_buffs.empty());

    if (!m_textures.empty())
    {
        glDeleteTextures(m_textures.size(), &m_textures[0]);
        m_textures.clear();
    }

    unsigned int count = img_buffs.size();
    m_textures.resize(count);
    glGenTextures(count, &m_textures[0]);

    for (unsigned int i = 0; i < count; i++)
    {
        byte_buffer buff = img_buffs[i];
        if (FATAL_LOG_EXPR((buff.empty()))) return ErrorCode::nullMemoryBuffer;
        if (png_check_sig((const unsigned char*)&buff[0], 8))
        {
            png_image image;
            memset(&image, 0, sizeof(image));
            image.version = PNG_IMAGE_VERSION;
            int res = png_image_begin_read_from_memory(&image, &buff[0], buff.size());
            if (res == 0) return ErrorCode::eglLoadTexture;
            byte_buffer raw_buffer;
            image.format = PNG_FORMAT_RGBA;
            unsigned int raw_buffer_size = PNG_IMAGE_SIZE(image);
            raw_buffer.resize(raw_buffer_size);
            res = png_image_finish_read(&image, NULL, &raw_buffer[0], 0, NULL);
            if (res == 0)
            {
                return ErrorCode::eglLoadTexture;
            }

            CreateOneFromSystemMemory(i, MathLib::Dimension{ image.width, image.height }, raw_buffer);
            png_image_free(&image);
        }
    }

    Frameworks::EventPublisher::Post(Frameworks::IEventPtr{ menew Graphics::MultiTextureResourceImagesLoaded(m_name) });
    return ErrorCode::ok;
}

error MultiTextureEgl::SaveTextureImages(const std::vector<FileSystem::IFilePtr>& files)
{
    if (FATAL_LOG_EXPR(m_textures.empty())) return ErrorCode::nullEglTexture;
    Platforms::Debug::Printf("GLES not support GetTexImage");
    return ErrorCode::notImplement;
}

error MultiTextureEgl::UseAsBackSurface(const Graphics::IBackSurfacePtr& back_surf)
{
    MultiBackSurfaceEgl* bb = dynamic_cast<MultiBackSurfaceEgl*>(back_surf.get());
    assert(bb);
    m_dimension = bb->GetDimension();
    unsigned int surf_count = bb->GetSurfaceCount();
    if (!m_textures.empty())
    {
        glDeleteTextures(m_textures.size(), &m_textures[0]);
        m_textures.clear();
    }
    m_textures.resize(surf_count);
    glGenTextures(surf_count, &m_textures[0]);

    GLenum* drawBuffers = memalloc(GLenum, surf_count);
    glBindFramebuffer(GL_FRAMEBUFFER, bb->GetFrameBufferHandle());
    for (unsigned int i = 0; i < m_textures.size(); i++)
    {
        glBindTexture(GL_TEXTURE_2D, m_textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_dimension.m_width, m_dimension.m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
        // 這幾個Texture Parameter 是必要的。同時，因為這個 Texture 並沒有 MipMap, 所以在 Texture Parameter &
        // Sampler Parameter 中，Mip Filter 要為 None.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_textures[i], 0);
        drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }
    glDrawBuffers(m_textures.size(), drawBuffers);
    SAFE_FREE(drawBuffers);

    Frameworks::EventPublisher::Post(Frameworks::IEventPtr{ menew Graphics::MultiTextureResourcesAsBackSurfaceUsed(m_name, back_surf->GetName()) });
    return ErrorCode::ok;
}

GLuint MultiTextureEgl::GetTextureHandle(unsigned int idx) const
{
    assert(!m_textures.empty() && idx < m_textures.size());
    return m_textures[idx];
}

const std::vector<GLuint>& MultiTextureEgl::GetTextureHandlesArray() const
{
    return m_textures;
}

error MultiTextureEgl::CreateOneFromSystemMemory(unsigned int index, const MathLib::Dimension& dimension,
                                                 const byte_buffer& buff)
{
    if (FATAL_LOG_EXPR((m_textures.empty()) || (index >= m_textures.size()))) return ErrorCode::nullEglTexture;
    glBindTexture(GL_TEXTURE_2D, m_textures[index]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dimension.m_width, dimension.m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &buff[0]);
    // mip map 要記得產生, mip filter 要用
    glGenerateMipmap(GL_TEXTURE_2D);
    m_dimension = dimension;
    m_format = Graphics::GraphicFormat::FMT_A8R8G8B8;
    glBindTexture(GL_TEXTURE_2D, 0);
    return ErrorCode::ok;
}

#endif