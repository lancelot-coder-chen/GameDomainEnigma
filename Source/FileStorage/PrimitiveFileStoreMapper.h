﻿/*********************************************************************
 * \file   PrimitiveFileStoreMapper.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   December 2023
 *********************************************************************/
#ifndef PRIMITIVE_FILE_STORE_MAPPER_H
#define PRIMITIVE_FILE_STORE_MAPPER_H

#include "GameEngine/FactoryDesc.h"
#include "Gateways/DtoGateway.h"
#include "Primitives/PrimitiveStoreMapper.h"
#include <mutex>

namespace Enigma::FileStorage
{
    class PrimitiveFileStoreMapper : public Primitives::PrimitiveStoreMapper
    {
    public:
        PrimitiveFileStoreMapper(const std::string& mapper_filename, const std::shared_ptr<Gateways::IDtoGateway>& gateway);
        ~PrimitiveFileStoreMapper() override;
        PrimitiveFileStoreMapper(const PrimitiveFileStoreMapper&) = delete;
        PrimitiveFileStoreMapper(PrimitiveFileStoreMapper&&) = delete;
        PrimitiveFileStoreMapper& operator=(const PrimitiveFileStoreMapper&) = delete;
        PrimitiveFileStoreMapper& operator=(PrimitiveFileStoreMapper&&) = delete;

        virtual std::error_code connect() override;
        virtual std::error_code disconnect() override;

        virtual bool hasPrimitive(const Primitives::PrimitiveId& id) override;
        virtual std::optional<Engine::GenericDto> queryPrimitive(const Primitives::PrimitiveId& id) override;
        virtual std::error_code removePrimitive(const Primitives::PrimitiveId& id) override;
        virtual std::error_code putPrimitive(const Primitives::PrimitiveId& id, const Engine::GenericDto& dto) override;

        virtual std::uint64_t nextSequenceNumber() override;

    protected:
        std::error_code serializeMapperFile();
        void deserializeMapperFile(const std::string& mapper_file_content);
        std::string extractFilename(const Primitives::PrimitiveId& id, const Engine::FactoryDesc& factory_desc);

        std::error_code serializeDataTransferObject(const std::string& filename, const Engine::GenericDto& dto);
        Engine::GenericDto deserializeDataTransferObject(const std::string& filename);

    protected:
        bool m_has_connected;
        std::shared_ptr<Gateways::IDtoGateway> m_gateway;
        std::string m_mapper_filename;
        std::unordered_map<Primitives::PrimitiveId, std::string, Primitives::PrimitiveId::hash> m_filename_map;
        mutable std::recursive_mutex m_fileMapLock;
        std::uint64_t m_sequence_number;
    };
}

#endif // PRIMITIVE_FILE_STORE_MAPPER_H
