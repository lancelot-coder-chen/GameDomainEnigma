﻿#include "VisibilityManagedTest.h"
#include "Platforms/MemoryAllocMacro.h"
#include <windows.h>
#include <tchar.h>
#include <assert.h>

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPTSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    meInitMemoryCheck();
    //_CrtSetBreakAlloc(40098);
    VisibilityManagedTest* app = menew VisibilityManagedTest("visibility managed test");
    app->Initialize(Enigma::Graphics::IGraphicAPI::APIVersion::API_Dx11, Enigma::Graphics::IGraphicAPI::AsyncType::UseAsyncDevice);
    RECT rc;
    GetClientRect(app->GetWnd(), &rc);
    app->OnFrameSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
    //app->CreateScene();
    app->Run();
    //app->DestroyScene();
    app->Finalize();

    delete app;
    return 0;
}