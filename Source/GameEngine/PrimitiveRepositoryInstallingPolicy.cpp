﻿#include "PrimitiveRepositoryInstallingPolicy.h"
#include "PrimitiveRepository.h"
#include "EngineErrors.h"
#include <cassert>

using namespace Enigma::Engine;

error PrimitiveRepositoryInstallingPolicy::install(Frameworks::ServiceManager* service_manager)
{
    assert(service_manager);
    service_manager->registerSystemService(std::make_shared<PrimitiveRepository>(service_manager, m_storeMapper));
    return ErrorCode::ok;
}

error PrimitiveRepositoryInstallingPolicy::shutdown(Frameworks::ServiceManager* service_manager)
{
    assert(service_manager);
    service_manager->shutdownSystemService(PrimitiveRepository::TYPE_RTTI);
    return ErrorCode::ok;
}

