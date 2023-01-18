﻿#include "AnimatorErrors.h"

using namespace Enigma::Animators;

const ErrorCategory ErrorCategory::ms_category;

std::string ErrorCategory::message(int err) const
{
    switch (static_cast<ErrorCode>(err))
    {
    case ErrorCode::ok: return "OK";
    case ErrorCode::policyIncomplete: return "policy incomplete error";
    case ErrorCode::deserializeFail: return "deserialize dto failed";
    case ErrorCode::dynamicCastFail: return "dynamic cast fail";
    case ErrorCode::nullAnimator: return "null animator";
    case ErrorCode::animatorMultiListening: return "animator multi listening";
    }
    return "Unknown";
}

const char* ErrorCategory::name() const noexcept
{
    return "Animators Error Category";
}
namespace Enigma::Animators
{
    std::error_code make_error_code(ErrorCode ec)
    {
        return std::error_code{ static_cast<int>(ec), ErrorCategory::get() };
    }
}
