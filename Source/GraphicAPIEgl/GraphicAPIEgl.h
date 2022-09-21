﻿/*********************************************************************
 * \file   GraphicAPIEgl.h
 * \brief  
 * 
 * \author Lancelot 'Robin' Chen
 * \date   July 2022
 *********************************************************************/
#ifndef GRAPHIC_API_EGL_H
#define GRAPHIC_API_EGL_H

#include "GraphicKernel/IGraphicAPI.h"

namespace Enigma::Devices
{
    using error = std::error_code;
    class VertexDeclarationEgl;

    class GraphicAPIEgl : public Graphics::IGraphicAPI
    {
    public:
        GraphicAPIEgl();
        GraphicAPIEgl(const GraphicAPIEgl&) = delete;
        GraphicAPIEgl(GraphicAPIEgl&&) = delete;
        virtual ~GraphicAPIEgl() override;
        GraphicAPIEgl& operator=(const GraphicAPIEgl&) = delete;
        GraphicAPIEgl& operator=(GraphicAPIEgl&&) = delete;

        virtual const std::unique_ptr<Graphics::IShaderCompiler>& GetShaderCompiler() override;

        void SetFormat(int* attrb);
        void SetDimension(const MathLib::Dimension& dim);

        virtual void CreateDevice(const Graphics::DeviceRequiredBits& rqb, void* hwnd) override;
        virtual void CleanupDevice() override;

        virtual void BeginScene() override;
        virtual void EndScene() override;

        virtual void CreatePrimaryBackSurface(const std::string& back_name, const std::string& depth_name) override;

        virtual void CreateVertexShader(const std::string& name) override;
        virtual void CreatePixelShader(const std::string& name) override;
        virtual void CreateShaderProgram(const std::string& name, const Graphics::IVertexShaderPtr& vtx_shader, 
            const Graphics::IPixelShaderPtr& pix_shader, const Graphics::IVertexDeclarationPtr& vtx_decl) override;
        virtual void CreateVertexDeclaration(const std::string& name, const std::string& data_vertex_format,
            const Graphics::IVertexShaderPtr& shader) override;

        virtual void CreateVertexBuffer(const std::string& buff_name, unsigned int sizeofVertex, unsigned int sizeBuffer) override;
        virtual void CreateIndexBuffer(const std::string& buff_name, unsigned int sizeBuffer) override;

        virtual void Clear(const Graphics::IBackSurfacePtr& back_surface, const Graphics::IDepthStencilSurfacePtr& depth_surface,
            const MathLib::ColorRGBA& color, float depth_value, unsigned int stencil_value) override;
        virtual void Flip() override;

        /** vertex / index buffer egl 需要 call */
        virtual error BindVertexBuffer(const Graphics::IVertexBufferPtr& buffer, Graphics::PrimitiveTopology pt) override;
        virtual error BindIndexBuffer(const Graphics::IIndexBufferPtr& buffer) override;

    protected:
        virtual error DrawPrimitive(unsigned int vertexCount, unsigned int vertexOffset) override;
        virtual error DrawIndexedPrimitive(
            unsigned int indexCount, unsigned int vertexCount, unsigned int indexOffset,
            int baseVertexOffset) override;
        //virtual error FlipBackSurface() override;
        virtual error CreateBackSurface(const std::string& back_name, const MathLib::Dimension& dimension,
            const Graphics::GraphicFormat& fmt) override;
        virtual error CreateBackSurface(const std::string& back_name, const MathLib::Dimension& dimension, unsigned int buff_count,
            const std::vector<Graphics::GraphicFormat>& fmts) override;
        virtual error CreateDepthStencilSurface(const std::string& depth_name, const MathLib::Dimension& dimension,
            const Graphics::GraphicFormat& fmt) override;
        virtual error ShareDepthStencilSurface(const std::string& depth_name,
            const Graphics::IDepthStencilSurfacePtr& from_depth) override;
        //virtual error ClearSurface(const Graphics::IBackSurfacePtr& back_surface, const Graphics::IDepthStencilSurfacePtr& depth_surface,
          //  const MathLib::ColorRGBA& color, float depth_value, unsigned int stencil_value) override;
        //virtual error CreateVertexShader(const std::string& name) override;
        //virtual error CreatePixelShader(const std::string& name) override;
        //virtual error CreateShaderProgram(const std::string& name,
          //  const Graphics::IVertexShaderPtr& vtx_shader, const Graphics::IPixelShaderPtr& pix_shader,
          //  const Graphics::IVertexDeclarationPtr& vtx_decl) override;
        //virtual error CreateVertexDeclaration(const std::string& name, const std::string& data_vertex_format,
          //  const Graphics::IVertexShaderPtr& shader) override;
        virtual error CreateSamplerState(const std::string& name, const Graphics::IDeviceSamplerState::SamplerStateData& data) override;
        virtual error CreateRasterizerState(const std::string& name, const Graphics::IDeviceRasterizerState::RasterizerStateData& data) override;
        virtual error CreateAlphaBlendState(const std::string& name, const Graphics::IDeviceAlphaBlendState::BlendStateData& data) override;
        virtual error CreateDepthStencilState(const std::string& name, const Graphics::IDeviceDepthStencilState::DepthStencilData& data) override;
        virtual error CreateTexture(const std::string& tex_name) override;
        virtual error CreateMultiTexture(const std::string& tex_name) override;

        virtual error BindBackSurface(
            const Graphics::IBackSurfacePtr& back_surface, const Graphics::IDepthStencilSurfacePtr& depth_surface) override;
        virtual error BindViewPort(const Graphics::TargetViewPort& vp) override;
        virtual error BindVertexDeclaration(const Graphics::IVertexDeclarationPtr& vertexDecl) override;
        virtual error BindVertexShader(const Graphics::IVertexShaderPtr& shader) override;
        virtual error BindPixelShader(const Graphics::IPixelShaderPtr& shader) override;
        virtual error BindShaderProgram(const Graphics::IShaderProgramPtr& shader) override;

        void CleanupDeviceObjects();
    private:
        error BindVertexDeclarationEgl(const std::shared_ptr<VertexDeclarationEgl>& vtxDecl);
        error UnBindVertexDeclarationEgl(const std::shared_ptr<VertexDeclarationEgl>& vtxDecl);
    protected:
        MathLib::Dimension m_surfaceDimension;
    };
}
#endif // GRAPHIC_API_EGL_H
