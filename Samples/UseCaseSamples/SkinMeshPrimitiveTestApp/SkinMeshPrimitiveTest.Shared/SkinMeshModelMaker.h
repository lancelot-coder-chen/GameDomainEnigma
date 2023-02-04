﻿/*********************************************************************
 * \file   SkinMeshModelMaker.h
 * \brief  
 * 
 * \author Lancelot 'Robin' Chen
 * \date   February 2023
 *********************************************************************/
#ifndef _SKIN_MESH_MODEL_MAKER_H
#define _SKIN_MESH_MODEL_MAKER_H

#include <Renderer/RenderablePrimitivePolicies.h>
#include <string>

class SkinMeshModelMaker
{
public:
    static std::shared_ptr<Enigma::Renderer::ModelPrimitivePolicy> MakeModelPrimitivePolicy(const std::string& model_name, const std::string& geo_name);

private:
    static Enigma::Renderer::SkinMeshPrimitiveDto MakeSkinMeshPrimitiveDto(const std::string& mesh_name, const std::string& geo_name);
    static Enigma::Engine::EffectMaterialDto MakeEffectDto(const std::string& eff_name);
    static Enigma::Engine::EffectTextureMapDto MakeTextureMapDto();
};

#endif // _SKIN_MESH_MODEL_MAKER_H
