﻿/*********************************************************************
 * \file   EffectMaterial.h
 * \brief  Effect material, render element 裡會參考到, value object 不適合
 *
 * \author Lancelot 'Robin' Chen
 * \date   September 2022
 *********************************************************************/
#ifndef EFFECT_MATERIAL_H
#define EFFECT_MATERIAL_H

#include "EffectVariable.h"
#include "EffectTechnique.h"
#include "GraphicKernel/IShaderProgram.h"
#include "Frameworks/Rtti.h"
#include "FactoryDesc.h"
#include <string>
#include <system_error>
#include <list>

namespace Enigma::Engine
{
    using error = std::error_code;
    class EffectMaterialSource;
    class GenericDto;

    class EffectMaterial
    {
        DECLARE_EN_RTTI_OF_BASE;
    public:
        EffectMaterial(const std::string& name, const std::vector<EffectTechnique>& techniques);
        EffectMaterial(const EffectMaterial&);
        EffectMaterial(EffectMaterial&&) noexcept;
        virtual ~EffectMaterial();
        EffectMaterial& operator=(const EffectMaterial&);
        EffectMaterial& operator=(EffectMaterial&&) noexcept;

        GenericDto serializeDto();

        const std::string& getName() { return m_name; };

        const FactoryDesc& factoryDesc() const { return m_factoryDesc; }
        FactoryDesc& factoryDesc() { return m_factoryDesc; }

        void setSource(const std::shared_ptr<EffectMaterialSource>& mat_source);
        std::shared_ptr<EffectMaterialSource> getEffectMaterialSource() { return m_sourceMaterial.lock(); };

        /** select renderer & visual technique */
        void selectRendererTechnique(const std::string& renderer_tech_name);
        void selectVisualTechnique(const std::string& visual_tech_name);

        unsigned int getPassCount();
        error applyFirstPass();
        error applyNextPass();
        bool hasNextPass();

        void commitInstancedEffectVariables();

        std::list<std::reference_wrapper<EffectVariable>> getEffectVariablesByName(const std::string& name);
        std::list<std::reference_wrapper<EffectVariable>> getEffectVariablesBySemantic(const std::string& semantic);
        stdext::optional_ref<EffectVariable> getEffectVariableInPassByName(const std::string& pass_name, const std::string& name);
        stdext::optional_ref<EffectVariable> getEffectVariableInPassBySemantic(const std::string& pass_name, const std::string& semantic);

        void setVariableAssignFunc(const std::string& semantic, EffectVariable::VariableValueAssignFunc fn);
        /** 每個 material instance 已經是獨立的一份 var, 似乎沒有必要了，先留著就是 */
        void setInstancedAssignFunc(const std::string& semantic, EffectVariable::VariableValueAssignFunc fn);

        void assignVariableValue(const std::string& semantic, std::any value);
        void assignVariableValues(const std::string& semantic, std::any value_array, unsigned int value_count);
        void assignInPassVariableValue(const std::string& pass_name, const std::string& semantic, std::any value);
        void assignInPassVariableValues(const std::string& pass_name, const std::string& semantic, std::any value_array, unsigned int value_count);

        void mappingAutoVariables();

    protected:
        void selectTechnique();

    protected:
        typedef std::vector<EffectTechnique> EffectTechniqueArray;
        typedef std::list<std::pair<std::reference_wrapper<EffectVariable>, EffectVariable::VariableValueAssignFunc>>
            InstancedAssignFuncList;

    protected:
        FactoryDesc m_factoryDesc;

        std::string m_name;
        std::weak_ptr<EffectMaterialSource> m_sourceMaterial;

        EffectTechniqueArray m_effectTechniques;
        EffectTechniqueArray::iterator m_currentTechnique;
        InstancedAssignFuncList m_instancedAssignFuncList;
        std::string m_selectedRendererTechName;
        std::string m_selectedVisualTechName;
    };
    using EffectMaterialPtr = std::shared_ptr<EffectMaterial>;
};

#endif // EFFECT_MATERIAL_H
