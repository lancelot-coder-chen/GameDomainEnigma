﻿#include "JsonFileContractDeserializer.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/Filename.h"
#include "FileSystem/IFile.h"
#include "Frameworks/ExtentTypesDefine.h"
#include "ContractJsonGateway.h"
#include "GameEngine/ContractEvents.h"
#include "Frameworks/EventPublisher.h"

using namespace Enigma::Gateways;
using namespace Enigma::FileSystem;
using namespace Enigma::Engine;

JsonFileContractDeserializer::JsonFileContractDeserializer() : IContractDeserializer()
{

}

JsonFileContractDeserializer::~JsonFileContractDeserializer()
{

}

void JsonFileContractDeserializer::InvokeDeserialize(const Frameworks::Ruid& ruid_deserializing, const std::string& param)
{
    IFilePtr readFile = FileSystem::FileSystem::Instance()->OpenFile(Filename(param), "rb");
    size_t filesize = readFile->Size();
    auto read_buff = readFile->Read(0, filesize);
    std::string read_json = convert_to_string(read_buff.value(), read_buff->size());
    Frameworks::EventPublisher::Post(std::make_shared<ContractDeserialized>(ruid_deserializing, ContractJsonGateway::Deserialize(read_json)));
}
