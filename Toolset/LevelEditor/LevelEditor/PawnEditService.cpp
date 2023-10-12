﻿#include "PawnEditService.h"

#include <Frameworks/StringFormat.h>

#include "LevelEditorCommands.h"
#include "LevelEditorErrors.h"
#include "PawnLoader.h"
#include "LevelEditorEvents.h"
#include "Frameworks/EventPublisher.h"
#include "GameCommon/GamesceneCommands.h"
#include "Frameworks/CommandBus.h"
#include "MathLib/Matrix4.h"
#include "Platforms/MemoryMacro.h"

using namespace LevelEditor;
using namespace Enigma::Frameworks;
using namespace Enigma::SceneGraph;
using namespace Enigma::GameCommon;
using namespace Enigma::MathLib;

Rtti PawnEditService::TYPE_RTTI{ "LevelEditor.PawnEditService", &ISystemService::TYPE_RTTI };

PawnEditService::PawnEditService(ServiceManager* srv_mngr) : ISystemService(srv_mngr)
{
    m_needTick = false;
    m_pawnLoader = nullptr;
}

PawnEditService::~PawnEditService()
{
    SAFE_DELETE(m_pawnLoader);
}

ServiceResult PawnEditService::OnInit()
{
    m_onPawnLoaded = std::make_shared<EventSubscriber>([=](auto e) { OnPawnLoaded(e); });
    EventPublisher::Subscribe(typeid(PawnLoaded), m_onPawnLoaded);
    m_onLoadPawnFailed = std::make_shared<EventSubscriber>([=](auto e) { OnLoadPawnFailed(e); });
    EventPublisher::Subscribe(typeid(LoadPawnFailed), m_onLoadPawnFailed);

    m_pawnLoader = new PawnLoader();

    return ServiceResult::Complete;
}

ServiceResult PawnEditService::OnTick()
{
    if (m_currentLoadingPawn) return ServiceResult::Pendding;
    if (m_loadingPawns.empty())
    {
        m_needTick = false;
        return ServiceResult::Pendding;
    }
    m_currentLoadingPawn = m_loadingPawns.front();
    m_loadingPawns.pop_front();
    assert(m_pawnLoader);
    m_pawnLoader->StartLoadingPawn(m_currentLoadingPawn->m_full_path);
    return ServiceResult::Pendding;
}

ServiceResult PawnEditService::OnTerm()
{
    EventPublisher::Unsubscribe(typeid(PawnLoaded), m_onPawnLoaded);
    m_onPawnLoaded = nullptr;
    EventPublisher::Unsubscribe(typeid(LoadPawnFailed), m_onLoadPawnFailed);
    m_onLoadPawnFailed = nullptr;

    SAFE_DELETE(m_pawnLoader);

    return ServiceResult::Complete;
}

error PawnEditService::PutCandidatePawn(const std::string& full_path, const Vector3& position)
{
    LoadingPawnMeta meta{ full_path, position };
    m_loadingPawns.push_back(meta);
    m_needTick = true;
    return ErrorCode::ok;
}

void PawnEditService::OnPawnLoaded(const IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<PawnLoaded>(e);
    if (!ev) return;
    if (!m_currentLoadingPawn) return;
    if (ev->GetPawnFilePath() != m_currentLoadingPawn->m_full_path) return;
    PutPawnAt(ev->GetPawn(), m_currentLoadingPawn->m_position);
    m_currentLoadingPawn = std::nullopt;
}

void PawnEditService::OnLoadPawnFailed(const Enigma::Frameworks::IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<LoadPawnFailed>(e);
    if (!ev) return;
    if (!m_currentLoadingPawn) return;
    if (ev->GetPawnFilePath() != m_currentLoadingPawn->m_full_path) return;
    CommandBus::Post(std::make_shared<OutputMessage>(string_format("Load Pawn Failed : %s", ev->GetErrorCode().message().c_str())));
    m_currentLoadingPawn = std::nullopt;
}

void PawnEditService::PutPawnAt(const std::shared_ptr<Pawn>& pawn, const Vector3& position)
{
    assert(pawn);
    CommandBus::Post(std::make_shared<AttachSceneRootChild>(pawn, Matrix4::MakeTranslateTransform(position)));
}
