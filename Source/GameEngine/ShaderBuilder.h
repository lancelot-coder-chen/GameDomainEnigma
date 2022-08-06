﻿/*********************************************************************
 * \file   ShaderBuilder.h
 * \brief  
 * 
 * \author Lancelot 'Robin' Chen
 * \date   August 2022
 *********************************************************************/
#ifndef SHADER_BUILDER_H
#define SHADER_BUILDER_H

#include "Frameworks/Event.h"
#include "Frameworks/EventSubscriber.h"
#include "GraphicKernel/IVertexShader.h"
#include "GraphicKernel/IPixelShader.h"
#include "GraphicKernel/IShaderProgram.h"
#include <string>

namespace Enigma::Engine
{
    struct ShaderCodeProfile
    {
        std::string m_code;
        std::string m_profile;
        std::string m_entry;
    };
    struct ShaderProgramPolicy
    {
        std::string m_programName;
        std::string m_vtxShaderName;
        std::string m_vtxFormatCode;
        ShaderCodeProfile m_vtxShaderCode;
        std::string m_vtxLayoutName;
        std::string m_pixShaderName;
        ShaderCodeProfile m_pixShaderCode;
    };

    class ShaderManager;

    class ShaderBuilder
    {
    private:
        /** inner events */
        class VertexShaderBuilt : public Frameworks::IEvent
        {
        public:
            VertexShaderBuilt(const std::string& name) :
                m_name(name) {};
            const std::string& GetShaderName() { return m_name; }
        private:
            std::string m_name;
        };
        class PixelShaderBuilt : public Frameworks::IEvent
        {
        public:
            PixelShaderBuilt(const std::string& name) :
                m_name(name) {};
            const std::string& GetShaderName() { return m_name; }
        private:
            std::string m_name;
        };

    public:
        class ShaderProgramBuilt : public Frameworks::IEvent
        {
        public:
            ShaderProgramBuilt(const std::string& name) :
                m_name(name) {};
            const std::string& GetShaderName() { return m_name; }
        private:
            std::string m_name;
        };

    public:
        ShaderBuilder(ShaderManager* host);
        ShaderBuilder(const ShaderBuilder&) = delete;
        ShaderBuilder(ShaderBuilder&&) = delete;
        ~ShaderBuilder();

        ShaderBuilder& operator=(const ShaderBuilder&) = delete;
        ShaderBuilder& operator=(ShaderBuilder&&) = delete;

        void BuildShaderProgram(const ShaderProgramPolicy& policy);

        Graphics::IShaderProgramPtr GetProgram() { return m_program; }

    private:
        void BuildVertexShader() const;
        void BuildPixelShader() const;

        void OnVertexShaderCreated(const Frameworks::IEventPtr& e);
        void OnVertexShaderCompiled(const Frameworks::IEventPtr& e);
        void OnVertexLayoutCreated(const Frameworks::IEventPtr& e);
        void OnPixelShaderCreated(const Frameworks::IEventPtr& e);
        void OnPixelShaderCompiled(const Frameworks::IEventPtr& e);

        void OnShaderBuilt(const Frameworks::IEventPtr& e);
        void OnShaderProgramCreated(const Frameworks::IEventPtr& e);

    private:
        ShaderManager* m_hostManager;

        Frameworks::EventSubscriberPtr m_onVertexShaderCreated;
        Frameworks::EventSubscriberPtr m_onVertexShaderCompiled;
        Frameworks::EventSubscriberPtr m_onVertexLayoutCreated;
        Frameworks::EventSubscriberPtr m_onPixelShaderCreated;
        Frameworks::EventSubscriberPtr m_onPixelShaderCompiled;

        Frameworks::EventSubscriberPtr m_onVertexShaderBuilt;
        Frameworks::EventSubscriberPtr m_onPixelShaderBuilt;
        Frameworks::EventSubscriberPtr m_onShaderProgramCreated;

        ShaderProgramPolicy m_policy;

        Graphics::IVertexShaderPtr m_vtxShader;
        Graphics::IPixelShaderPtr m_pixShader;
        Graphics::IShaderProgramPtr m_program;
    };
}

#endif // SHADER_BUILDER_H
