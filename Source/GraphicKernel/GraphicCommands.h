﻿/*********************************************************************
 * \file   GraphicCommands.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   August 2022
 *********************************************************************/
#ifndef GRAPHIC_COMMANDS_H
#define GRAPHIC_COMMANDS_H

#include "DeviceRequiredBits.h"
#include "Frameworks/Command.h"
#include "MathLib/ColorRGBA.h"

namespace Enigma::Graphics
{
    /** device create / cleanup */
    class CreateDevice : public Frameworks::ICommand
    {
    public:
        CreateDevice(const DeviceRequiredBits& rqb, void* hwnd) : m_rqb(rqb), m_hwnd(hwnd) {}
        const DeviceRequiredBits& GetRequiredBits() const { return m_rqb; }
        void* GetHwnd() const { return m_hwnd; }

    private:
        DeviceRequiredBits m_rqb;
        void* m_hwnd;
    };
    class CleanupDevice : public Frameworks::ICommand
    {
    };

    /** scene begin / end */
    class BeginScene : public Frameworks::ICommand
    {
    };
    class EndScene : public Frameworks::ICommand
    {
    };

    /** draw (indexed) primitive */
    class DrawPrimitive : public Frameworks::ICommand
    {
    public:
        DrawPrimitive(unsigned int vertexCount, unsigned int vertexOffset) : m_count(vertexCount), m_offset(vertexOffset) {}
        unsigned int GetVertexCount() const { return m_count; }
        unsigned int GetVertexOffset() const { return m_offset; }

    private:
        unsigned int m_count;
        unsigned int m_offset;
    };
    class DrawIndexedPrimitive : public Frameworks::ICommand
    {
    public:
        DrawIndexedPrimitive(unsigned int indexCount, unsigned int vertexCount, unsigned int indexOffset, int baseVertexOffset)
            : m_indexCount(indexCount), m_vertexCount(vertexCount), m_indexOffset(indexOffset), m_baseVertexOffset(baseVertexOffset) {}
        unsigned int GetIndexCount() const { return m_indexCount; }
        unsigned int GetVertexCount() const { return m_vertexCount; }
        unsigned int GetIndexOffset() const { return m_indexOffset; }
        int GetBaseVertexOffset() const { return m_baseVertexOffset; }

    private:
        unsigned int m_indexCount;
        unsigned int m_vertexCount;
        unsigned int m_indexOffset;
        int m_baseVertexOffset;
    };

    class ClearSurface : public Frameworks::ICommand
    {
    public:
        ClearSurface(const IBackSurfacePtr& back_surface, const IDepthStencilSurfacePtr& depth_surface,
            const MathLib::ColorRGBA& color, float depth_value, unsigned int stencil_value)
            : m_backSurface(back_surface), m_depthSurface(depth_surface), m_color(color), m_depthValue(depth_value),
            m_stencilValue(stencil_value) {};
        const IBackSurfacePtr& GetBackSurface() const { return m_backSurface; }
        const IDepthStencilSurfacePtr& GetDepthSurface() const { return m_depthSurface; }
        const MathLib::ColorRGBA& GetColor() const { return m_color; }
        float GetDepthValue() const { return m_depthValue; }
        unsigned int GetStencilValue() const { return m_stencilValue; }
    private:
        IBackSurfacePtr m_backSurface;
        IDepthStencilSurfacePtr m_depthSurface;
        MathLib::ColorRGBA m_color;
        float m_depthValue;
        unsigned int m_stencilValue;
    };
    class FlipBackSurface : public Frameworks::ICommand
    {
    };

    /** create surface */
    class CreatePrimarySurface : public Frameworks::ICommand
    {
    public:
        CreatePrimarySurface(const std::string& back_name, const std::string& depth_name)
    		: m_backName(back_name), m_depthName(depth_name) {};
        const std::string& GetBacksurfaceName() const { return m_backName; }
        const std::string& GetDepthsurfaceName() const { return m_depthName; }
    private:
        std::string m_backName;
        std::string m_depthName;
    };
    class CreateBacksurface : public Frameworks::ICommand
    {
    public:
        CreateBacksurface(const std::string& back_name, const MathLib::Dimension& dimension,
            const GraphicFormat& fmt) : m_backName(back_name), m_dimension(dimension), m_fmt(fmt) {};
        const std::string& GetBacksurfaceName() const { return m_backName; }
        const MathLib::Dimension& GetDimension() const { return m_dimension; }
        const GraphicFormat& GetFormat() const { return m_fmt; }
    private:
        std::string m_backName;
        MathLib::Dimension m_dimension;
        GraphicFormat m_fmt;
    };
    class CreateMultiBacksurface : public Frameworks::ICommand
    {
    public:
        CreateMultiBacksurface(const std::string& back_name, const MathLib::Dimension& dimension,
            unsigned int buff_count, const std::vector<GraphicFormat>& fmts)
    			: m_backName(back_name), m_dimension(dimension), m_buffCount(buff_count), m_fmts(fmts) {};
        const std::string& GetBacksurfaceName() const { return m_backName; }
        const MathLib::Dimension& GetDimension() const { return m_dimension; }
        unsigned int GetSurfaceCount() const { return m_buffCount; }
        const std::vector<GraphicFormat>& GetFormats() const { return m_fmts; }
    private:
        std::string m_backName;
        MathLib::Dimension m_dimension;
        unsigned int m_buffCount;
        std::vector<GraphicFormat> m_fmts;
    };
    class CreateDepthStencilSurface : public Frameworks::ICommand
    {
    public:
        CreateDepthStencilSurface(const std::string& depth_name, const MathLib::Dimension& dimension,
            const GraphicFormat& fmt) : m_depthName(depth_name), m_dimension(dimension), m_fmt(fmt) {};
        const std::string& GetDepthStencilSurfaceName() const { return m_depthName; }
        const MathLib::Dimension& GetDimension() const { return m_dimension; }
        const GraphicFormat& GetFormat() const { return m_fmt; }
    private:
        std::string m_depthName;
        MathLib::Dimension m_dimension;
        GraphicFormat m_fmt;
    };
    class ShareDepthStencilSurface : public Frameworks::ICommand
    {
    public:
        ShareDepthStencilSurface(const std::string& depth_name, const IDepthStencilSurfacePtr& from_depth)
    	: m_depthName(depth_name), m_fromDepth(from_depth) {};
        const std::string& GetDepthStencilSurfaceName() const { return m_depthName; }
        const IDepthStencilSurfacePtr& GetSourceDepth() const { return m_fromDepth; }
    private:
        std::string m_depthName;
        IDepthStencilSurfacePtr m_fromDepth;
    };
}


#endif // GRAPHIC_COMMANDS_H
