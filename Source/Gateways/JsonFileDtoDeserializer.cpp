﻿#include "JsonFileDtoDeserializer.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/Filename.h"
#include "FileSystem/IFile.h"
#include "Frameworks/ExtentTypesDefine.h"
#include "DtoJsonGateway.h"
#include "FileSystem/FileSystemErrors.h"
#include "GameEngine/DtoEvents.h"
#include "Frameworks/EventPublisher.h"
#include "GameEngine/EngineErrors.h"

using namespace Enigma::Gateways;
using namespace Enigma::FileSystem;
using namespace Enigma::Engine;

JsonFileDtoDeserializer::JsonFileDtoDeserializer() : IDtoDeserializer()
{

}

void JsonFileDtoDeserializer::InvokeDeserialize(const Frameworks::Ruid& ruid_deserializing, const std::string& param)
{
    IFilePtr readFile = FileSystem::FileSystem::instance()->openFile(Filename(param), FileSystem::read | FileSystem::binary);
    size_t filesize = readFile->size();
    auto read_buff = readFile->read(0, filesize);
    if (read_buff)
    {
        std::string read_json = convert_to_string(read_buff.value(), read_buff->size());
        Frameworks::EventPublisher::post(std::make_shared<GenericDtoDeserialized>(ruid_deserializing, DtoJsonGateway::Deserialize(read_json)));
    }
    else
    {
        Frameworks::EventPublisher::post(std::make_shared<DeserializeDtoFailed>(ruid_deserializing, Engine::ErrorCode::fileIOError));
    }
}
