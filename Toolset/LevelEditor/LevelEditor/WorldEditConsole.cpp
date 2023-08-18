﻿#include "WorldEditConsole.h"
#include "Frameworks/CommandBus.h"
#include "LevelEditorCommands.h"
#include "Platforms/PlatformLayer.h"
#include "WorldMap/WorldMapEvents.h"
#include "EditorUtilities.h"
#include "Gateways/DtoJsonGateway.h"
#include "WorldEditService.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/StdMountPath.h"
#include "Frameworks/EventPublisher.h"
#include <filesystem>

using namespace LevelEditor;
using namespace Enigma::Frameworks;
using namespace Enigma::FileSystem;
using namespace Enigma::WorldMap;

Rtti WorldEditConsole::TYPE_RTTI{"LevelEditor.WorldEditConsole", &ISystemService::TYPE_RTTI};

WorldEditConsole::WorldEditConsole(ServiceManager* srv_mngr, const std::shared_ptr<WorldEditService>& world_service) : ISystemService(srv_mngr)
{
    m_worldEditService = world_service;
    m_needTick = false;
}

WorldEditConsole::~WorldEditConsole()
{
}

ServiceResult WorldEditConsole::OnInit()
{
    m_onWorldMapCreated = std::make_shared<EventSubscriber>([=](auto e) { OnWorldMapCreated(e); });
    EventPublisher::Subscribe(typeid(WorldMapCreated), m_onWorldMapCreated);
    return ServiceResult::Complete;
}

ServiceResult WorldEditConsole::OnTerm()
{
    EventPublisher::Unsubscribe(typeid(WorldMapCreated), m_onWorldMapCreated);
    m_onWorldMapCreated = nullptr;
    return ServiceResult::Complete;
}

void WorldEditConsole::SetWorldMapRootFolder(const std::filesystem::path& folder, const std::string& world_map_path_id)
{
    auto path = std::filesystem::current_path();
    auto mediaPath = path / "../../../Media/";
    m_mapFileRootPath = mediaPath / folder;
    m_worldMapPathId = world_map_path_id;
    FileSystem::Instance()->AddMountPath(std::make_shared<StdMountPath>(m_mapFileRootPath.string(), world_map_path_id));
}

bool WorldEditConsole::CheckWorldMapFolder(const std::string& world_folder)
{
    if (world_folder.empty()) return false;
    const auto world_path = m_mapFileRootPath / world_folder;
    return is_directory(world_path);
}

void WorldEditConsole::DeleteWorldMapFolder(const std::string& world_folder)
{
    if (world_folder.empty()) return;
    const auto world_path = m_mapFileRootPath / world_folder;
    std::error_code ec;
    remove_all(world_path, ec);
    if (ec)
    {
        Enigma::Platforms::Debug::ErrorPrintf("remove folder all error : %s", ec.message().c_str());
    }
}

void WorldEditConsole::CreateWorldMapFolder(const std::string& folder_name)
{
    if (folder_name.empty()) return;
    m_currentWorldFolder = folder_name;
    const auto world_path = m_mapFileRootPath / folder_name;
    if (const bool is_created = create_directory(world_path); is_created)
    {
        CommandBus::Post(std::make_shared<OutputMessage>("[WorldEditorService] World map folder " + folder_name + " is created!"));
    }
}

void WorldEditConsole::SaveWorldMap()
{
    assert(!m_worldEditService.expired());
    auto [map_graph, node_graphs] = m_worldEditService.lock()->SerializeWorldMapAndNodeGraphs(m_worldMapPathId);
    if (!map_graph.empty())
    {
        std::string json = Enigma::Gateways::DtoJsonGateway::Serialize(map_graph);
        IFilePtr iFile = FileSystem::Instance()->OpenFile(Filename(map_graph[0].GetRtti().GetPrefab()), "w+b");
        if (FATAL_LOG_EXPR(!iFile)) return;
        iFile->Write(0, convert_to_buffer(json));
        FileSystem::Instance()->CloseFile(iFile);
    }
    for (auto& dtos : node_graphs)
    {
        std::string json = Enigma::Gateways::DtoJsonGateway::Serialize(dtos);
        IFilePtr iFile = FileSystem::Instance()->OpenFile(Filename(dtos[0].GetRtti().GetPrefab()), "w+b");
        if (FATAL_LOG_EXPR(!iFile)) return;
        iFile->Write(0, convert_to_buffer(json));
        FileSystem::Instance()->CloseFile(iFile);
    }
}

void WorldEditConsole::LoadWorldMap(const std::filesystem::path& map_filepath)
{
    std::string path_string = FilePathCombinePathID(map_filepath, m_worldMapPathId) + "@" + m_worldMapPathId;

    Enigma::Frameworks::CommandBus::Post(std::make_shared<OutputMessage>("Load World File " + path_string + "..."));
    IFilePtr iFile = FileSystem::Instance()->OpenFile(path_string, "rb");
    size_t file_size = iFile->Size();

    auto read_buf = iFile->Read(0, file_size);
    FileSystem::Instance()->CloseFile(iFile);
    auto dtos = Enigma::Gateways::DtoJsonGateway::Deserialize(convert_to_string(read_buf.value(), file_size));
    if (!m_worldEditService.expired())
    {
        m_worldEditService.lock()->DeserializeWorldMap(dtos);
    }
}

void WorldEditConsole::OnWorldMapCreated(const Enigma::Frameworks::IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<Enigma::WorldMap::WorldMapCreated, IEvent>(e);
    if (!ev) return;
    m_currentWorldName = ev->GetName();
}
