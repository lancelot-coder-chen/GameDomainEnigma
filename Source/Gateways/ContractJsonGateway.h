﻿/*********************************************************************
 * \file   ContractJsonGateway.h
 * \brief  
 * 
 * \author Lancelot 'Robin' Chen
 * \date   October 2022
 *********************************************************************/
#ifndef _CONTRACT_JSON_GATEWAY_H
#define _CONTRACT_JSON_GATEWAY_H

#define RAPIDJSON_HAS_STDSTRING 1

#include "GameEngine/Contract.h"
#include <vector>
#include <string>

namespace Enigma::Gateways
{
    class ContractJsonGateway
    {
    public:
        static std::vector<Engine::Contract> Deserialize(const std::string& json);
        static std::string Serialize(const std::vector<Engine::Contract>& contracts);
    };
}

#endif // _CONTRACT_JSON_GATEWAY_H