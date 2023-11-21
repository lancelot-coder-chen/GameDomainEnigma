﻿#include "Light.h"
#include "LightInfo.h"
#include "LightInfoDtos.h"
#include "SceneGraphErrors.h"
#include "SceneGraphEvents.h"
#include "SceneGraphDtos.h"
#include "Frameworks/EventPublisher.h"

using namespace Enigma::SceneGraph;
using namespace Enigma::MathLib;

DEFINE_RTTI(SceneGraph, Light, Spatial);

Light::Light(const std::string& spatialName, const LightInfo& lightInfo) : Spatial(spatialName), m_lightInfo(lightInfo)
{
    m_factoryDesc = Engine::FactoryDesc(Light::TYPE_RTTI.getName());
}

Light::Light(const Engine::GenericDto& o) : Spatial(o)
{
    LightDto dto = LightDto::fromGenericDto(o);
    m_lightInfo = LightInfo(dto.LightInfo());
}

Light::~Light()
{
    Frameworks::EventPublisher::post(std::make_shared<LightInfoDeleted>(m_name, m_lightInfo.GetLightType()));
}

Enigma::Engine::GenericDto Light::serializeDto()
{
    LightDto dto(serializeSpatialDto());
    dto.LightInfo() = m_lightInfo.serializeDto().toGenericDto();
    return dto.toGenericDto();
}

error Light::onCullingVisible(Culler*, bool)
{
    return ErrorCode::ok;
}

error Light::_updateWorldData(const MathLib::Matrix4& mxParentWorld)
{
    error er = Spatial::_updateWorldData(mxParentWorld);
    if (er) return er;
    m_lightInfo.SetLightPosition(m_vecWorldPosition);

    _propagateSpatialRenderState();

    return er;
}
void Light::SetLightColor(const MathLib::ColorRGBA& color)
{
    Info().SetLightColor(color);
    Frameworks::EventPublisher::post(std::make_shared<LightInfoUpdated>(ThisLight(), LightInfoUpdated::NotifyCode::Color));
}

void Light::SetLightPosition(const MathLib::Vector3& vec)
{
    Info().SetLightPosition(vec);
    Frameworks::EventPublisher::post(std::make_shared<LightInfoUpdated>(ThisLight(), LightInfoUpdated::NotifyCode::Position));
}

void Light::SetLightDirection(const MathLib::Vector3& vec)
{
    Info().SetLightDirection(vec);
    Frameworks::EventPublisher::post(std::make_shared<LightInfoUpdated>(ThisLight(), LightInfoUpdated::NotifyCode::Direction));
}

void Light::SetLightAttenuation(const MathLib::Vector3& attenuation)
{
    Info().SetLightAttenuation(attenuation);
    Frameworks::EventPublisher::post(std::make_shared<LightInfoUpdated>(ThisLight(), LightInfoUpdated::NotifyCode::Attenuation));
}

void Light::SetLightRange(float range)
{
    Info().SetLightRange(range);
    Frameworks::EventPublisher::post(std::make_shared<LightInfoUpdated>(ThisLight(), LightInfoUpdated::NotifyCode::Range));
}

void Light::SetEnable(bool flag)
{
    Info().SetEnable(flag);
    Frameworks::EventPublisher::post(std::make_shared<LightInfoUpdated>(ThisLight(), LightInfoUpdated::NotifyCode::Enable));
}
