﻿#include "AsyncJsonFileContractDeserializer.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/Filename.h"
#include "FileSystem/IFile.h"
#include "Frameworks/ExtentTypesDefine.h"
#include "ContractJsonGateway.h"
#include "GameEngine/ContractEvents.h"
#include "Frameworks/EventPublisher.h"
#include "FileSystem/FileSystemErrors.h"

using namespace Enigma::Gateways;
using namespace Enigma::Engine;
using namespace Enigma::FileSystem;

AsyncJsonFileContractDeserializer::AsyncJsonFileContractDeserializer() : IContractDeserializer()
{
}

AsyncJsonFileContractDeserializer::~AsyncJsonFileContractDeserializer()
{
}

void AsyncJsonFileContractDeserializer::InvokeDeserialize(const Frameworks::Ruid& ruid_deserializing, const std::string& param)
{
    m_ruid = ruid_deserializing;
    m_parameter = param;
    m_deserializing = std::async(std::launch::async, [lifetime = shared_from_this(), this]() { DeserializeProcedure(); });
}

void AsyncJsonFileContractDeserializer::DeserializeProcedure()
{
    FutureFile readingFile = FileSystem::FileSystem::Instance()->AsyncOpenFile(Filename(m_parameter), "rb");
    while (!readingFile.valid() || (readingFile.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready)) {}
    IFilePtr readFile = readingFile.get();
    if (!readFile)
    {
        Frameworks::EventPublisher::Post(std::make_shared<DeserializeContractFailed>(m_ruid, FileSystem::ErrorCode::fileOpenError));
        return;
    }
    size_t filesize = readFile->Size();
    IFile::FutureRead read = readFile->AsyncRead(0, filesize);
    while (!read.valid() || (read.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready)) {}
    auto buff = read.get();
    if (!buff)
    {
        Frameworks::EventPublisher::Post(std::make_shared<DeserializeContractFailed>(m_ruid, FileSystem::ErrorCode::readFail));
        return;
    }
    std::string read_json = convert_to_string(buff.value(), buff->size());
    Frameworks::EventPublisher::Post(std::make_shared<ContractDeserialized>(m_ruid, ContractJsonGateway::Deserialize(read_json)));
}
