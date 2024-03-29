﻿#include "MainForm.h"
#include "ViewerAppDelegate.h"
#include "SchemeColorDef.h"
#include "RenderPanel.h"
#include "OutputPanel.h"
#include "ModelInfoPanel.h"
#include "AnimationInfoPanel.h"
#include "Platforms/MemoryMacro.h"
#include "GraphicKernel/IGraphicAPI.h"
#include "nana/gui/filebox.hpp"
#include "DaeParser.h"

using namespace EnigmaViewer;
using namespace Enigma::Graphics;
using namespace std::chrono_literals;

MainForm::MainForm()
{
    m_menubar = nullptr;
    m_tabbar = nullptr;
    m_appDelegate = nullptr;
    m_timer = nullptr;
    m_renderPanel = nullptr;
    m_outputPanel = nullptr;
    m_modelInfoPanel = nullptr;
    m_animationInfoPanel = nullptr;
}

MainForm::~MainForm()
{
    SAFE_DELETE(m_tabbar);
    SAFE_DELETE(m_menubar);
    SAFE_DELETE(m_timer);
    SAFE_DELETE(m_appDelegate);
    SAFE_DELETE(m_renderPanel);
    SAFE_DELETE(m_outputPanel);
    SAFE_DELETE(m_modelInfoPanel);
    SAFE_DELETE(m_animationInfoPanel);
}

void MainForm::initialize()
{
    UISchemeColors::ApplySchemaColors(scheme());
    get_place().div("vert<menubar weight=28>< <renderpanel weight=70%> | <vert<toolsbar weight=28> <toolsframe> > > <outputpanel weight=100>");
    initMenu();

    m_renderPanel = menew RenderPanel{ *this };
    get_place().field("renderpanel") << *m_renderPanel;

    m_tabbar = menew nana::tabbar<int>{ *this };
    UISchemeColors::ApplySchemaColors(m_tabbar->scheme());
    get_place().field("toolsbar") << *m_tabbar;

    m_modelInfoPanel = menew ModelInfoPanel{ *this };
    m_modelInfoPanel->initialize(this);
    get_place().field("toolsframe").fasten(*m_modelInfoPanel);
    m_tabbar->append("Model Info", *m_modelInfoPanel);

    m_animationInfoPanel = menew AnimationInfoPanel{ *this };
    m_animationInfoPanel->initialize();
    get_place().field("toolsframe").fasten(*m_animationInfoPanel);
    m_tabbar->append("Animation Info", *m_animationInfoPanel);

    m_outputPanel = menew OutputPanel{ *this };
    m_outputPanel->initialize();
    get_place()["outputpanel"] << *m_outputPanel;

    m_appDelegate = new ViewerAppDelegate();
    m_appDelegate->initialize(IGraphicAPI::APIVersion::API_Dx11, IGraphicAPI::AsyncType::UseAsyncDevice, "viewer_log.log",
        reinterpret_cast<HWND>(m_renderPanel->native_handle()));
    m_timer = new nana::timer{ 1ms };
    m_timer->elapse([this] { m_appDelegate->onTimerElapsed(); });
    m_timer->start();
    events().destroy([this] { this->finalize(); });

    m_renderPanel->initInputHandler(m_appDelegate->inputHandler());
    m_renderPanel->subscribeHandlers();
    m_outputPanel->subscribeHandlers();
    m_modelInfoPanel->subscribeHandlers();
    m_animationInfoPanel->subscribeHandlers();

    get_place().collocate();
}

void MainForm::initMenu()
{
    m_menubar = menew nana::menubar{ *this };
    m_menubar->scheme().background = UISchemeColors::BACKGROUND;
    m_menubar->scheme().body_selected = UISchemeColors::SELECT_BG;
    m_menubar->scheme().body_highlight = UISchemeColors::HIGHLIGHT_BG;
    m_menubar->scheme().text_fgcolor = UISchemeColors::FOREGROUND;
    m_menubar->push_back("&File");
    m_menubar->at(0).append("Import DAE File", [this](auto item) { onImportDaeFile(item); });
    m_menubar->at(0).append("Save Animated Pawn", [this](auto item) { onSaveAnimatedPawn(item); });
    m_menubar->at(0).append("Load Animated Pawn", [this](auto item) { onLoadAnimatedPawn(item); });
    m_menubar->at(0).append_splitter();
    m_menubar->at(0).append("Exit", [this](auto item) { onCloseCommand(item); });
    get_place().field("menubar") << *m_menubar;
}

void MainForm::onImportDaeFile(const nana::menu::item_proxy& menu_item)
{
    nana::filebox fb{ *this, true };
    fb.add_filter({ {"Collada File(*.dae)", "*.dae"} });
    auto paths = fb.show();
    if (paths.size() > 0)
    {
        if (m_outputPanel)
        {
            m_outputPanel->addMessage(std::string{ "open Collada file " } + paths[0].string());
        }
        DaeParser* parser = new DaeParser(Enigma::Controllers::GraphicMain::instance()->getSystemServiceAs<Enigma::Geometries::GeometryRepository>());
        parser->loadDaeFile(paths[0].string());
        m_appDelegate->loadPawn(parser->pawnDto());
        m_modelInfoPanel->setModelFileName(paths[0].stem().string());
        delete parser;
    }
}

void MainForm::onSaveAnimatedPawn(const nana::menu::item_proxy& menu_item)
{
    nana::filebox fb{ *this, false };
    fb.add_filter({ {"Pawn File(*.pawn)", "*.pawn"} });
    auto paths = fb.show();
    if (paths.size() > 0)
    {
        m_appDelegate->savePawnFile(paths[0].string());
    }
}

void MainForm::onLoadAnimatedPawn(const nana::menu::item_proxy& menu_item)
{
    nana::filebox fb{ *this, true };
    fb.add_filter({ {"Pawn File(*.pawn)", "*.pawn"} });
    auto paths = fb.show();
    if (paths.size() > 0)
    {
        m_appDelegate->loadPawnFile(paths[0].string());
        m_modelInfoPanel->setModelFileName(paths[0].stem().string());
        /*m_modelInfoPanel->SetModelFileName(paths[0].stem().string());
        m_modelInfoPanel->EnumModelMeshNode(std::dynamic_pointer_cast<Enigma::ModelPrimitive, Enigma::Primitive>
            (m_appDelegate->GetEntityPrimitive()));
        Enigma::ActionClipMapPtr clip_map = std::dynamic_pointer_cast<Enigma::AnimatedEntity, Enigma::Entity>
            (m_appDelegate->GetEntity())->GetActionClipMap();
        m_animInfoPanel->RefreshActionTable(clip_map);
        m_animInfoPanel->RefreshActionCombo(clip_map);*/
    }
}

void MainForm::onCloseCommand(const nana::menu::item_proxy& menu_item)
{
    close();
}

void MainForm::finalize()
{
    if (m_renderPanel) m_renderPanel->unsubscribeHandlers();
    if (m_outputPanel) m_outputPanel->unsubscribeHandlers();
    if (m_modelInfoPanel) m_modelInfoPanel->unsubscribeHandlers();
    if (m_animationInfoPanel) m_animationInfoPanel->unsubscribeHandlers();
    if (m_appDelegate) m_appDelegate->finalize();
}
