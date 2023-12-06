﻿/*********************************************************************
 * \file   GeometryDataPolicy.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   November 2022
 *********************************************************************/
#ifndef _GEOMETRY_DATA_POLICY_H
#define _GEOMETRY_DATA_POLICY_H

#include "GeometryDataDto.h"
#include "DtoDeserializer.h"
#include <string>
#include <optional>

namespace Enigma::Engine
{
    class GeometryDataPolicy
    {
    public:
        GeometryDataPolicy() = default;
        GeometryDataPolicy(const GeometryId& id, const GenericDto& dto) : m_id(id), m_dto(dto) {}
        GeometryDataPolicy(const GeometryId& id, const std::string& deserialize_param,
            const std::shared_ptr<IDtoDeserializer>& deserializer) : m_id(id), m_parameter(deserialize_param), m_deserializer(deserializer) {}

        [[nodiscard]] const GeometryId& id() const { return m_id; }
        GeometryId& id() { return m_id; }

        [[nodiscard]] const std::optional<GenericDto>& getDto() const { return m_dto; }

        [[nodiscard]] const std::string& parameter() const { return m_parameter; }
        [[nodiscard]] const std::shared_ptr<IDtoDeserializer>& getDeserializer() const { return m_deserializer; }

    protected:
        GeometryId m_id;
        std::optional<GenericDto> m_dto;
        std::string m_parameter;
        std::shared_ptr<IDtoDeserializer> m_deserializer;
    };
}

#endif // _GEOMETRY_DATA_POLICY_H
