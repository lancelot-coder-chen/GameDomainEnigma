﻿/********************************************************************
 * \file   GraphicMain.h
 * \brief  
 * 
 * \author Lancelot 'Robin' Chen
 * \date   June 2022
 *********************************************************************/
#ifndef GRAPHIC_MAIN_H
#define GRAPHIC_MAIN_H

#include <system_error>
#include <memory>
#include "Frameworks/ServiceManager.h"
#include "GraphicKernel/IGraphicAPI.h"

namespace Enigma::Renderer
{
    class RendererManager;
};

namespace Enigma::Controllers
{
    using error = std::error_code;
    class InstallingPolicy;
    class DeviceCreatingPolicy;
    class InstallingDefaultRendererPolicy;
    class SceneGraphBuildingPolicy;

    /** Graphic Kernel Main class \n singleton */
    class GraphicMain
    {
    public:
        GraphicMain();
        GraphicMain(const GraphicMain&) = delete;
        ~GraphicMain();
        GraphicMain& operator=(const GraphicMain&) = delete;

        /** get singleton instance */
        static GraphicMain* Instance();

        error InstallFrameworks();
        error ShutdownFrameworks();

        error InstallRenderEngine(const std::vector<std::shared_ptr<InstallingPolicy>>& policies);
        error ShutdownRenderEngine();

        /** frame update (service manager call run one to update) */
        void FrameUpdate();

        Frameworks::ServiceManager* GetServiceManager() { return m_serviceManager; };

    private:
        std::shared_ptr<InstallingPolicy> FindDeviceCreatingPolicy();
        std::shared_ptr<InstallingPolicy> FindRendererInstallingPolicy();
        std::shared_ptr<InstallingPolicy> FindSceneGraphBuildingPolicy();

        error CreateRenderEngineDevice(const std::shared_ptr<DeviceCreatingPolicy>& policy);
        error CleanupRenderEngineDevice();

        error InstallDefaultRenderer(const std::shared_ptr<InstallingDefaultRendererPolicy>& policy);
        error ShutdownDefaultRenderer();

        error InstallRenderer(const std::string& renderer_name, const std::string render_target_name, bool is_primary);
        error ShutdownRenderer(const std::string& renderer_name, const std::string render_target_name);

        error InstallGeometryManagers();
        error ShutdownGeometryManagers();

        error InstallShaderManagers();
        error ShutdownShaderManagers();

        error InstallRenderBufferManagers();
        error ShutdownRenderBufferManagers();

        error InstallTextureManagers();
        error ShutdownTextureManagers();

        error InstallSceneGraphManagers(const std::shared_ptr<SceneGraphBuildingPolicy>& policy);
        error ShutdownSceneGraphManagers();

        error InstallAnimationServices();
        error ShutdownAnimationServices();

    private:
        static GraphicMain* m_instance;

        Frameworks::ServiceManager* m_serviceManager;

        std::vector<std::shared_ptr<InstallingPolicy>> m_policies;

        Renderer::RendererManager* m_renderer;
    };

};


#endif // GRAPHIC_MAIN_H
