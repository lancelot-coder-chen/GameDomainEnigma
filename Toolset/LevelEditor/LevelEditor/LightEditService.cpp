﻿#include "LightEditService.h"
#include "Frameworks/CommandBus.h"
#include "LevelEditorCommands.h"
#include "GameCommon/GameLightCommands.h"

using namespace LevelEditor;
using namespace Enigma::Frameworks;
using namespace Enigma::GameCommon;

Rtti LightEditService::TYPE_RTTI{"LevelEditor.LightEditService", &ISystemService::TYPE_RTTI};

LightEditService::LightEditService(ServiceManager* srv_mngr) : ISystemService(srv_mngr)
{

}

LightEditService::~LightEditService()
{

}

ServiceResult LightEditService::OnInit()
{
    m_doCreatingEnvironmentLight = std::make_shared<CommandSubscriber>([=](auto c) { DoCreatingEnvironmentLight(c); });
    CommandBus::Subscribe(typeid(CreateEnvironmentLight), m_doCreatingEnvironmentLight);

    return ServiceResult::Complete;
}

ServiceResult LightEditService::OnTerm()
{
    CommandBus::Unsubscribe(typeid(CreateEnvironmentLight), m_doCreatingEnvironmentLight);
    m_doCreatingEnvironmentLight = nullptr;

    return ServiceResult::Complete;
}

void LightEditService::DoCreatingEnvironmentLight(const ICommandPtr& c)
{
    if (!c) return;
    auto cmd = std::dynamic_pointer_cast<CreateEnvironmentLight, ICommand>(c);
    if (!cmd) return;
    CommandBus::Post(std::make_shared<CreateAmbientLight>(cmd->GetWorldName(), cmd->GetWorldName() + "_amb_lit", Enigma::MathLib::ColorRGBA(0.2f, 0.2f, 0.2f, 1.0f)));
    CommandBus::Post(std::make_shared<CreateSunLight>(cmd->GetWorldName(), cmd->GetWorldName() + "_sun_lit", Enigma::MathLib::Vector3(-1.0, -1.0, -1.0), Enigma::MathLib::ColorRGBA(0.6f, 0.6f, 0.6f, 1.0f)));
}