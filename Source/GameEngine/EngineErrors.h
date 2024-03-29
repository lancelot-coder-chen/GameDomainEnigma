﻿/********************************************************************
 * \file   EngineErrors.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   June 2022
 *********************************************************************/
#ifndef ENGINE_ERRORS_H
#define ENGINE_ERRORS_H

#include <system_error>

namespace Enigma::Engine
{
    enum class ErrorCode
    {
        ok = 0,

        findStashedAssetFail = 101,
        policyIncomplete,
        deserializeFail,
        fileIOError,

        primitiveFactoryNotExists = 151,
        primitiveEntityAlreadyExists,
        effectAlreadyCompiled,

        effectPassEnd = 201,
        compilingEmptyEffectTech,
        noSelectedTechnique,
        nullMaterial,
        textureSemantic,

        targetTextureNotExists = 301,
        textureAlreadyExists,
        textureAlreadyLoaded,
        textureNotReady,
    };
    class ErrorCategory : public std::error_category
    {
    public:
        std::string message(int err) const override;
        const char* name() const noexcept override;
        static const std::error_category& get()
        {
            return ms_category;
        }
    private:
        const static ErrorCategory ms_category;
    };

    extern std::error_code make_error_code(ErrorCode ec);
}

namespace std
{
    // let compiler know that ErrorCode is compatible with std::error_code
    template <>
    struct is_error_code_enum<Enigma::Engine::ErrorCode> : true_type {};
}


#endif // ENGINE_ERRORS_H
