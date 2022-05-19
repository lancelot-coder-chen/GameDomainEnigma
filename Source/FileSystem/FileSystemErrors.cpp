﻿#include "FileSystemErrors.h"

using namespace Enigma::FileSystem;

const ErrorCategory ErrorCategory::ms_category;
std::error_code ErrorCategory::ms_last_error = ErrorCode::ok;

std::string ErrorCategory::message(int err) const
{
    switch (static_cast<ErrorCode>(err))
    {
    case ErrorCode::ok: return "OK";
    case ErrorCode::emptyFilePath: return "Empty file path";
    case ErrorCode::emptyRWOption: return "Empty R/W Option";
    case ErrorCode::fileOpenError: return "File open error";
    case ErrorCode::fileStatusError: return "File status error";
    case ErrorCode::readOffsetError: return "Read offset error";
    case ErrorCode::readFail: return "Read fail";
    case ErrorCode::writeFail: return "Write fail";
    }
    return "Unknown";
}

const char* ErrorCategory::name() const noexcept
{
    return "FileSystem Error Category";
}
namespace Enigma::AssetPackage
{
    inline std::error_code make_error_code(ErrorCode ec)
    {
        std::error_code er = std::error_code(static_cast<int>(ec), ErrorCategory::get());
        ErrorCategory::last_error(er);
        return er;
    }
    inline const std::error_code& last_error()
    {
        return ErrorCategory::last_error();
    }

}