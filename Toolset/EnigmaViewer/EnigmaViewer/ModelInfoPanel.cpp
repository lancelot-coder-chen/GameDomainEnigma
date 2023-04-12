﻿#include "ModelInfoPanel.h"
#include "SchemeColorDef.h"
#include "Platforms/MemoryMacro.h"
#include "ViewerCommands.h"
#include "Frameworks/CommandBus.h"

using namespace EnigmaViewer;

ModelInfoPanel::ModelInfoPanel(const nana::window& wd) : panel<false>{ wd }
{
    m_place = nullptr;
    m_modelNamePrompt = nullptr;
    m_modelNameLabel = nullptr;
    m_meshNodeTree = nullptr;
    m_popupMenu = nullptr;
}

ModelInfoPanel::~ModelInfoPanel()
{
    SAFE_DELETE(m_place);
    SAFE_DELETE(m_modelNamePrompt);
    SAFE_DELETE(m_modelNameLabel);
    SAFE_DELETE(m_meshNodeTree);
    SAFE_DELETE(m_popupMenu);
}

void ModelInfoPanel::Initialize(MainForm* main_form)
{
    m_main = main_form;
    m_place = menew nana::place{ *this };
    m_place->div("margin=[4,4,4,4] vert<modelnameprompt weight=28><modelname weight=28><meshnodetree>");
    m_modelNamePrompt = menew nana::label{ *this, "Model Name" };
    UISchemeColors::ApplySchemaColors(m_modelNamePrompt->scheme());
    m_modelNameLabel = menew nana::label{ *this, "xxx" };
    UISchemeColors::ApplySchemaColors(m_modelNameLabel->scheme());
    m_meshNodeTree = menew nana::treebox{ *this };
    m_meshNodeTree->events().mouse_down([this](const nana::arg_mouse& arg) { this->OnMeshNodeTreeMouseDown(arg); });
    UISchemeColors::ApplySchemaColors(m_meshNodeTree->scheme());
    m_meshNodeTree->scheme().item_bg_highlighted = UISchemeColors::HIGHLIGHT_BG;
    m_meshNodeTree->scheme().item_bg_selected = UISchemeColors::SELECT_BG;
    (*m_place)["modelnameprompt"] << *m_modelNamePrompt;
    (*m_place)["modelname"] << *m_modelNameLabel;
    (*m_place)["meshnodetree"] << *m_meshNodeTree;

    m_place->collocate();

    m_popupMenu = menew nana::menu{};
    m_popupMenu->append("Add Particles");
    m_popupMenu->append_splitter();
    m_popupMenu->append("Avatar Command");
    nana::menu* particleMenu = m_popupMenu->create_sub_menu(0);
    particleMenu->append("Cloud Particle", [this](auto item) { this->OnAddCloudParticle(item); });
    particleMenu->append("Super Spray Particle", [this](auto item) { this->OnAddSuperSprayParticle(item); });
    nana::menu* avatarMenu = m_popupMenu->create_sub_menu(2);
    avatarMenu->append("Change Texture", [this](auto item) { this->OnChangeMeshTexture(item); });
    //m_meshNodeTree->events().mouse_down(nana::menu_popuper(*m_popupParticleMenu));
}

void ModelInfoPanel::SetModelFileName(const std::string& filename)
{
    if (m_modelNameLabel) m_modelNameLabel->caption(filename);
}

void ModelInfoPanel::EnumModelMeshNode(const std::shared_ptr<Enigma::Renderer::ModelPrimitive>& model)
{
    m_meshNodeTree->clear();
    if (!model) return;
    if (model->GetMeshNodeTree().GetMeshNodeCount() == 0) return;

    std::unordered_map<std::string, nana::treebox::item_proxy> mesh_pos_map;

    unsigned int total_node_count = model->GetMeshNodeTree().GetMeshNodeCount();
    unsigned int total_mesh_count = model->GetMeshPrimitiveCount();
    unsigned int mesh_count = 0;

    nana::treebox::item_proxy root_pos = m_meshNodeTree->insert("root", model->GetName());
    mesh_pos_map.emplace("root", root_pos);
    for (unsigned int ni = 0; ni < total_node_count; ni++)
    {
        auto mesh_node = model->GetMeshNodeTree().GetMeshNode(ni);
        if (!mesh_node) continue;
        nana::treebox::item_proxy parent_pos;
        if (auto parent_index = mesh_node.value().get().GetParentIndexInArray())
        {
            auto parent_mesh_node = model->GetMeshNodeTree().GetMeshNode(parent_index.value());
            if (parent_mesh_node)
            {
                parent_pos = mesh_pos_map.find(parent_mesh_node.value().get().GetName())->second;
            }
            else
            {
                parent_pos = mesh_pos_map.find("root")->second;
            }
        }
        else
        {
            parent_pos = mesh_pos_map.find("root")->second;
        }
        nana::treebox::item_proxy node_pos = m_meshNodeTree->insert(
            parent_pos, mesh_node.value().get().GetName(), mesh_node.value().get().GetName());
        mesh_pos_map.emplace(mesh_node.value().get().GetName(), node_pos);
        if (mesh_node.value().get().GetMeshPrimitive())
        {
            m_meshNodeTree->insert(node_pos,
                mesh_node.value().get().GetMeshPrimitive()->GetName(), mesh_node.value().get().GetMeshPrimitive()->GetName());
        }
    }
}

void ModelInfoPanel::OnMeshNodeTreeMouseDown(const nana::arg_mouse& arg)
{
    if (!arg.right_button) return;
    if (m_meshNodeTree->selected().empty()) return;
    auto poper = menu_popuper(*m_popupMenu);
    poper(arg);
}

void ModelInfoPanel::OnAddCloudParticle(const nana::menu::item_proxy& menu_item)
{
    if (m_meshNodeTree->selected().empty()) return;
    //m_main->GetAppDelegate()->AddCloudParticleToMeshNode(m_meshNodeTree->selected().key());
}

void ModelInfoPanel::OnAddSuperSprayParticle(const nana::menu::item_proxy& menu_item)
{
    if (m_meshNodeTree->selected().empty()) return;
    //m_main->GetAppDelegate()->AddSprayParticleToMeshNode(m_meshNodeTree->selected().key());
}

void ModelInfoPanel::OnChangeMeshTexture(const nana::menu::item_proxy& menu_item)
{
    if (m_meshNodeTree->selected().empty()) return;
    //m_main->GetAppDelegate()->ChangeMeshTexture(m_meshNodeTree->selected().key());
}

void ModelInfoPanel::SubscribeHandlers()
{
    m_doRefreshingModelNodeTree = std::make_shared<Enigma::Frameworks::CommandSubscriber>([=](auto c) { DoRefreshingModelNodeTree(c); });
    Enigma::Frameworks::CommandBus::Subscribe(typeid(RefreshModelNodeTree), m_doRefreshingModelNodeTree);
}

void ModelInfoPanel::UnsubscribeHandlers()
{
    Enigma::Frameworks::CommandBus::Unsubscribe(typeid(RefreshModelNodeTree), m_doRefreshingModelNodeTree);
    m_doRefreshingModelNodeTree = nullptr;
}

void ModelInfoPanel::DoRefreshingModelNodeTree(const Enigma::Frameworks::ICommandPtr& c)
{
    if (!c) return;
    auto cmd = std::dynamic_pointer_cast<RefreshModelNodeTree, Enigma::Frameworks::ICommand>(c);
    if (!cmd) return;
    EnumModelMeshNode(cmd->GetModel());
}
