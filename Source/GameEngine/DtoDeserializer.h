﻿/*********************************************************************
 * \file   DtoDeserializer.h
 * \brief  dto deserializer, a base class, will implement by gateways
 *
 * \author Lancelot 'Robin' Chen
 * \date   November 2022
 *********************************************************************/
#ifndef _DTO_DESERIALIZER_H
#define _DTO_DESERIALIZER_H

#include "Frameworks/ruid.h"
#include <memory>

namespace Enigma::Engine
{
    class IDtoDeserializer : public std::enable_shared_from_this<IDtoDeserializer>
    {
    public:
        IDtoDeserializer() = default;
        virtual ~IDtoDeserializer() = default;

        virtual void InvokeDeserialize(const Frameworks::Ruid& ruid_deserializing, const std::string& param) = 0;
    };
}

#endif // _DTO_DESERIALIZER_H