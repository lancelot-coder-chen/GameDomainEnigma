﻿/********************************************************************
 * \file   IGraphicAPI.h
 * \brief  
 * 
 * \author Lancelot 'Robin' Chen
 * \date   June 2022
 *********************************************************************/
#ifndef GRAPHIC_API_INTERFACE_H
#define GRAPHIC_API_INTERFACE_H

#include "DeviceRequiredBits.h"
#include "ExtentTypesDefine.h"
#include <system_error>

namespace Enigma::Graphics
{
    using error = std::error_code;

    class GraphicThread;

    class IGraphicAPI
    {
    public:
        enum class APIVersion
        {
            API_Dx9,
            API_Dx11,
            API_EGL,
        };
        enum class AsyncType : bool
        {
            UseAsyncDevice = true,
            NotAsyncDevice = false
        };
    public:
        IGraphicAPI();
        IGraphicAPI(const IGraphicAPI&) = delete;
        virtual ~IGraphicAPI();
        IGraphicAPI& operator=(const IGraphicAPI&) = delete;

        static IGraphicAPI* Instance();

        /** @name create / cleanup device */
        //@{
        virtual error CreateDevice(const DeviceRequiredBits& rqb, void* hwnd) = 0;
        virtual error CleanupDevice() = 0;
        virtual future_error AsyncCreateDevice(const DeviceRequiredBits& rqb, void* hwnd);
        virtual future_error AsyncCleanupDevice();
        //@}

        virtual void TerminateGraphicThread();

    protected:
        static IGraphicAPI* m_instance;
        void* m_wnd;
        DeviceRequiredBits m_deviceRequiredBits;
        APIVersion m_apiVersion;

        GraphicFormat m_fmtBackSurface;
        GraphicFormat m_fmtDepthSurface;

        GraphicThread* m_workerThread;
    };
}

#endif // GRAPHIC_API_INTERFACE_H
