﻿/*********************************************************************
 * \file   ViewerAppDelegate.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   October 2022
 *********************************************************************/
#ifndef VIEWER_APP_DELEGATE_H
#define VIEWER_APP_DELEGATE_H

#include <windows.h>
#include "GraphicKernel/IGraphicAPI.h"
#include "Controllers/GraphicMain.h"
#include "InputHandlers/InputHandlerService.h"
#include "GameCommon/SceneRendererService.h"
#include "GameCommon/AnimatedPawnDto.h"
#include "Frameworks/EventSubscriber.h"
#include "GameCommon/AnimatedPawn.h"
#include "Frameworks/CommandSubscriber.h"
#include "ShadowMap/ShadowMapService.h"
#include <filesystem>

namespace EnigmaViewer
{
    class ViewerAppDelegate
    {
    public:
        ViewerAppDelegate();
        ~ViewerAppDelegate();

        void initialize(Enigma::Graphics::IGraphicAPI::APIVersion api_ver, Enigma::Graphics::IGraphicAPI::AsyncType useAsyncDevice,
            const std::string& log_filename, HWND hwnd);
        virtual void initializeMountPaths();
        void installEngine();
        void registerMediaMountPaths(const std::string& media_path);
        void shutdownEngine();
        void finalize();

        void frameUpdate();
        void prepareRender();
        void renderFrame();

        void onTimerElapsed();

        void loadPawn(const Enigma::GameCommon::AnimatedPawnDto& pawn_dto);

        void savePawnFile(const std::filesystem::path& filepath);
        void loadPawnFile(const std::filesystem::path& filepath);

        std::shared_ptr<Enigma::InputHandlers::InputHandlerService> inputHandler() const { return m_inputHandler.lock(); }

    protected:
        void onPawnPrimitiveBuilt(const Enigma::Frameworks::IEventPtr& e);
        void onSceneGraphRootCreated(const Enigma::Frameworks::IEventPtr& e);
        void onSceneGraphBuilt(const Enigma::Frameworks::IEventPtr& e);
        void changeMeshTexture(const Enigma::Frameworks::ICommandPtr& c);
        void addAnimationClip(const Enigma::Frameworks::ICommandPtr& c);
        void deleteAnimationClip(const Enigma::Frameworks::ICommandPtr& c);
        void playAnimationClip(const Enigma::Frameworks::ICommandPtr& c);
        void changeAnimationTimeValue(const Enigma::Frameworks::ICommandPtr& c);

        void onViewingPawnPrimitiveBuilt();
        void onFloorPrimitiveBuilt();
        void createFloorReceiver();

    protected:
        HWND m_hwnd;
        bool m_hasLogFile;

        std::string m_mediaRootPath;

        Enigma::Controllers::GraphicMain* m_graphicMain;

        std::weak_ptr<Enigma::InputHandlers::InputHandlerService> m_inputHandler;
        std::weak_ptr<Enigma::GameCommon::SceneRendererService> m_sceneRenderer;
        std::weak_ptr<Enigma::ShadowMap::ShadowMapService> m_shadowMapService;

        Enigma::Frameworks::EventSubscriberPtr m_onPawnPrimitiveBuilt;
        Enigma::Frameworks::EventSubscriberPtr m_onSceneGraphRootCreated;
        Enigma::Frameworks::EventSubscriberPtr m_onSceneGraphBuilt;

        Enigma::Frameworks::CommandSubscriberPtr m_changeMeshTexture;
        Enigma::Frameworks::CommandSubscriberPtr m_addAnimationClip;
        Enigma::Frameworks::CommandSubscriberPtr m_deleteAnimationClip;
        Enigma::Frameworks::CommandSubscriberPtr m_playAnimationClip;
        Enigma::Frameworks::CommandSubscriberPtr m_changeAnimationTimeValue;

        std::shared_ptr<Enigma::SceneGraph::Node> m_sceneRoot;
        std::shared_ptr<Enigma::GameCommon::AnimatedPawn> m_pawn;
        std::shared_ptr<Enigma::SceneGraph::Pawn> m_floor;
    };
}

#endif // VIEWER_APP_DELEGATE_H
