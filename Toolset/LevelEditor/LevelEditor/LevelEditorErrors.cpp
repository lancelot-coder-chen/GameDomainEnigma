﻿#include "LevelEditorErrors.h"

using namespace LevelEditor;

const ErrorCategory ErrorCategory::ms_category;

std::string ErrorCategory::message(int err) const
{
    switch (static_cast<ErrorCode>(err))
    {
    case ErrorCode::ok: return "OK";
    }
    return "Unknown";
}

const char* ErrorCategory::name() const noexcept
{
    return "LevelEditor Error Category";
}
namespace LevelEditor
{
    std::error_code make_error_code(ErrorCode ec)
    {
        return std::error_code(static_cast<int>(ec), ErrorCategory::get());
    }
}