﻿/*********************************************************************
 * \file   IVertexShader.h
 * \brief  
 * 
 * \author Lancelot 'Robin' Chen
 * \date   July 2022
 *********************************************************************/
#ifndef VERTEX_SHADER_INTERFACE_H
#define VERTEX_SHADER_INTERFACE_H

#include "Frameworks/ExtentTypesDefine.h"
#include <memory>
#include <string>
#include <system_error>

namespace Enigma::Graphics
{
    using error = std::error_code;

    class IVertexShader : public std::enable_shared_from_this<IVertexShader>
    {
    public:
        IVertexShader(const std::string& name);
        IVertexShader(const IVertexShader&) = delete;
        IVertexShader(IVertexShader&&) = delete;
        virtual ~IVertexShader();
        IVertexShader& operator=(const IVertexShader&) = delete;
        IVertexShader& operator=(IVertexShader&&) = delete;

        const std::string& GetName() { return m_name; }
        /** compile vertex shader
        @param code HLSL/GLSL shader code
        @param profile "vs_1_1", "vs_2_0" ....
        @param entry entry point function name
        */
        virtual error CompileCode(const std::string& code, const std::string& profile, const std::string& entry) = 0;
        virtual future_error AsyncCompileCode(
            const std::string& code, const std::string& profile, const std::string& entry);

        virtual bool HasCompiled() { return m_hasCompiled; }

    protected:
        std::string m_name;
        bool m_hasCompiled;
    };
    using IVertexShaderPtr = std::shared_ptr<IVertexShader>;
    using IVertexShaderWeak = std::weak_ptr<IVertexShader>;
}

#endif // VERTEX_SHADER_INTERFACE_H
