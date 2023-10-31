﻿#include "PawnEditService.h"
#include "Frameworks/StringFormat.h"
#include "LevelEditorCommands.h"
#include "LevelEditorErrors.h"
#include "PawnLoader.h"
#include "LevelEditorEvents.h"
#include "Frameworks/EventPublisher.h"
#include "GameCommon/GamesceneCommands.h"
#include "Frameworks/CommandBus.h"
#include "MathLib/Matrix4.h"
#include "Platforms/MemoryMacro.h"
#include "Prefabs/PawnPrefabDto.h"
#include "Prefabs/PrefabCommands.h"
#include "Prefabs/PrefabEvents.h"
#include "GameEngine/FactoryDesc.h"
#include "GameCommon/AnimatedPawn.h"
#include "WorldMap/WorldMapQueries.h"
#include "Frameworks/QueryDispatcher.h"

using namespace LevelEditor;
using namespace Enigma::Frameworks;
using namespace Enigma::SceneGraph;
using namespace Enigma::GameCommon;
using namespace Enigma::MathLib;
using namespace Enigma::Prefabs;
using namespace Enigma::Engine;

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

ServiceResult PawnEditService::onInit()
{
    m_onPrefabLoaded = std::make_shared<EventSubscriber>([=](auto e) { onPrefabLoaded(e); });
    EventPublisher::subscribe(typeid(PawnPrefabLoaded), m_onPrefabLoaded);
    m_onLoadPrefabFailed = std::make_shared<EventSubscriber>([=](auto e) { onLoadPrefabFailed(e); });
    EventPublisher::subscribe(typeid(LoadPawnPrefabFailed), m_onLoadPrefabFailed);

    m_pawnLoader = new PawnLoader();

    return ServiceResult::Complete;
}

ServiceResult PawnEditService::onTick()
{
    if (m_currentLoadingPawn) return ServiceResult::Pendding;
    loadNextPawn();
    return ServiceResult::Pendding;
}

ServiceResult PawnEditService::onTerm()
{
    EventPublisher::unsubscribe(typeid(PawnLoaded), m_onPrefabLoaded);
    m_onPrefabLoaded = nullptr;
    EventPublisher::unsubscribe(typeid(LoadPawnPrefabFailed), m_onLoadPrefabFailed);
    m_onLoadPrefabFailed = nullptr;

    SAFE_DELETE(m_pawnLoader);

    return ServiceResult::Complete;
}

error PawnEditService::putCandidatePawn(const std::string& name, const std::string& full_path, const Vector3& position)
{
    LoadingPawnMeta meta{ name, full_path, position };
    m_loadingPawns.push_back(meta);
    m_needTick = true;
    return ErrorCode::ok;
}

void PawnEditService::loadNextPawn()
{
    if (m_currentLoadingPawn) return;
    if (m_loadingPawns.empty())
    {
        m_needTick = false;
        return;
    }
    m_currentLoadingPawn = m_loadingPawns.front();
    m_loadingPawns.pop_front();
    PawnPrefabDto dto;
    dto.name() = m_currentLoadingPawn->m_name;
    dto.isTopLevel() = true;
    dto.factoryDesc() = FactoryDesc(AnimatedPawn::TYPE_RTTI.getName()).ClaimByPrefab(m_currentLoadingPawn->m_full_path);
    dto.worldTransform() = Matrix4::MakeTranslateTransform(m_currentLoadingPawn->m_position);
    CommandBus::post(std::make_shared<LoadPawnPrefab>(dto.toGenericDto()));
}

void PawnEditService::onPrefabLoaded(const IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<PawnPrefabLoaded>(e);
    if (!ev) return;
    if (!m_currentLoadingPawn) return;
    if (ev->getPrefabAtPath() != m_currentLoadingPawn->m_full_path) return;
    m_loadedPawn = ev->getPawn();
    tryPutPawnAt(m_loadedPawn, m_currentLoadingPawn->m_position);
}

void PawnEditService::onLoadPrefabFailed(const Enigma::Frameworks::IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<LoadPawnPrefabFailed>(e);
    if (!ev) return;
    if (!m_currentLoadingPawn) return;
    //if (ev->GetPrefabFilePath() != m_currentLoadingPawn->m_full_path) return;
    CommandBus::post(std::make_shared<OutputMessage>(string_format("Load Pawn Failed : %s", ev->GetError().message().c_str())));
    m_currentLoadingPawn = std::nullopt;
    m_loadedPawn = nullptr;
}

bool PawnEditService::tryPutPawnAt(const std::shared_ptr<Enigma::SceneGraph::Pawn>& pawn, const Enigma::MathLib::Vector3& position)
{
    auto world_transform = Matrix4::MakeTranslateTransform(position);
    BoundingVolume bv = BoundingVolume::CreateFromTransform(pawn->GetModelBound(), world_transform);
    auto query = std::make_shared<Enigma::WorldMap::QueryFittingNode>(bv);
    QueryDispatcher::dispatch(query);
    if (query->getResult() == nullptr) return false;
    auto node = query->getResult();
    auto inv_node_transform = node->GetWorldTransform().Inverse();
    CommandBus::post(std::make_shared<AttachNodeChild>(node->GetSpatialName(), pawn, inv_node_transform * world_transform));
    return true;
}

void PawnEditService::putPawnAt(const std::shared_ptr<Pawn>& pawn, const Vector3& position)
{
    assert(pawn);
    CommandBus::post(std::make_shared<AttachSceneRootChild>(pawn, Matrix4::MakeTranslateTransform(position)));
}
