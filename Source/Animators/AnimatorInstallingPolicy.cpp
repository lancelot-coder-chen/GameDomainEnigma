﻿#include "AnimatorInstallingPolicy.h"
#include "ModelAnimationRepository.h"
#include "AnimationFrameListener.h"
#include "GameEngine/TimerService.h"
#include "AnimatorErrors.h"
#include <cassert>

using namespace Enigma::Animators;

error AnimatorInstallingPolicy::install(Frameworks::ServiceManager* service_manager)
{
    assert(service_manager);
    service_manager->registerSystemService(std::make_shared<AnimationRepository>(service_manager));
    auto timer = service_manager->getSystemServiceAs<Engine::TimerService>();
    service_manager->registerSystemService(std::make_shared<AnimationFrameListener>(service_manager, timer));
    return ErrorCode::ok;
}

error AnimatorInstallingPolicy::shutdown(Frameworks::ServiceManager* service_manager)
{
    assert(service_manager);
    service_manager->shutdownSystemService(AnimationFrameListener::TYPE_RTTI);
    service_manager->shutdownSystemService(AnimationRepository::TYPE_RTTI);
    return ErrorCode::ok;
}

