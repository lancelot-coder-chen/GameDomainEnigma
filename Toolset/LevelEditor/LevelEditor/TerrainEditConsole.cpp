﻿#include "TerrainEditConsole.h"
#include "LevelEditorAppDelegate.h"
#include "LevelEditorEvents.h"
#include "LevelEditorCommands.h"
#include "Frameworks/EventPublisher.h"
#include "Frameworks/CommandBus.h"
#include "GameEngine/StandardGeometryDtoHelper.h"
#include "Renderer/RenderablePrimitiveDtos.h"
#include "GameEngine/EffectDtoHelper.h"
#include "SceneGraph/SceneGraphDtoHelper.h"
#include "SceneGraph/SceneGraphCommands.h"
#include "SceneGraph/SceneGraphEvents.h"
#include "GameCommon/GameSceneCommands.h"
#include "GameCommon/DeferredRenderingCommands.h"
#include "LevelEditorUiEvents.h"

using namespace LevelEditor;
using namespace Enigma::Frameworks;
using namespace Enigma::Engine;
using namespace Enigma::MathLib;
using namespace Enigma::Renderer;
using namespace Enigma::SceneGraph;

Rtti TerrainEditConsole::TYPE_RTTI{"LevelEditor.TerrainEditConsole", &ISystemService::TYPE_RTTI};

std::string BRUSH_SPHERE_TAG = "_brush_sphere_";
std::string SEMANTIC_DECAL_MAP = "DecalMap";

TerrainEditConsole::TerrainEditConsole(ServiceManager* srv_mngr) : ISystemService(srv_mngr)
{
    m_needTick = false;
    m_isEnabled = false;
    m_currMode = TerrainEditMode::Mode_Unknown;
    m_brushSize = 1;
    m_brushDensity = 0.0f;
    m_brushHeight = 0.3f;
}

TerrainEditConsole::~TerrainEditConsole()
{

}

ServiceResult TerrainEditConsole::OnInit()
{
    m_onEditorModeChanged = std::make_shared<EventSubscriber>([=](auto e) { OnEditorModeChanged(e); });
    EventPublisher::Subscribe(typeid(EditorModeChanged), m_onEditorModeChanged);
    m_onSceneGraphBuilt = std::make_shared<EventSubscriber>([=](auto e) { OnSceneGraphBuilt(e); });
    EventPublisher::Subscribe(typeid(FactorySceneGraphBuilt), m_onSceneGraphBuilt);
    m_onPawnPrimitiveBuilt = std::make_shared<EventSubscriber>([=](auto e) { OnPawnPrimitiveBuilt(e); });
    EventPublisher::Subscribe(typeid(PawnPrimitiveBuilt), m_onPawnPrimitiveBuilt);

    m_onTerrainBrushSizeChanged = std::make_shared<EventSubscriber>([=](auto e) { OnTerrainBrushSizeChanged(e); });
    EventPublisher::Subscribe(typeid(TerrainBrushSizeChanged), m_onTerrainBrushSizeChanged);
    m_onTerrainBrushHeightChanged = std::make_shared<EventSubscriber>([=](auto e) { OnTerrainBrushHeightChanged(e); });
    EventPublisher::Subscribe(typeid(TerrainBrushHeightChanged), m_onTerrainBrushHeightChanged);
    m_onTerrainBrushDensityChanged = std::make_shared<EventSubscriber>([=](auto e) { OnTerrainBrushDensityChanged(e); });
    EventPublisher::Subscribe(typeid(TerrainBrushDensityChanged), m_onTerrainBrushDensityChanged);
    m_onTerrainPaintingLayerChanged = std::make_shared<EventSubscriber>([=](auto e) { OnTerrainPaintingLayerChanged(e); });
    EventPublisher::Subscribe(typeid(TerrainPaintingLayerChanged), m_onTerrainPaintingLayerChanged);
    m_onTerrainToolSelected = std::make_shared<EventSubscriber>([=](auto e) { OnTerrainToolSelected(e); });
    EventPublisher::Subscribe(typeid(TerrainEditToolSelected), m_onTerrainToolSelected);

    m_onSceneCursorMoved = std::make_shared<EventSubscriber>([=](auto e) { OnSceneCursorMoved(e); });
    EventPublisher::Subscribe(typeid(SceneCursorMoved), m_onSceneCursorMoved);
    m_onSceneCursorDragged = std::make_shared<EventSubscriber>([=](auto e) { OnSceneCursorDragged(e); });
    EventPublisher::Subscribe(typeid(SceneCursorDragged), m_onSceneCursorDragged);

    return ServiceResult::Complete;
}

ServiceResult TerrainEditConsole::OnTerm()
{
    EventPublisher::Unsubscribe(typeid(EditorModeChanged), m_onEditorModeChanged);
    m_onEditorModeChanged = nullptr;
    EventPublisher::Unsubscribe(typeid(FactorySceneGraphBuilt), m_onSceneGraphBuilt);
    m_onSceneGraphBuilt = nullptr;
    EventPublisher::Unsubscribe(typeid(PawnPrimitiveBuilt), m_onPawnPrimitiveBuilt);
    m_onPawnPrimitiveBuilt = nullptr;

    EventPublisher::Unsubscribe(typeid(TerrainBrushSizeChanged), m_onTerrainBrushSizeChanged);
    m_onTerrainBrushSizeChanged = nullptr;
    EventPublisher::Unsubscribe(typeid(TerrainBrushHeightChanged), m_onTerrainBrushHeightChanged);
    m_onTerrainBrushHeightChanged = nullptr;
    EventPublisher::Unsubscribe(typeid(TerrainBrushDensityChanged), m_onTerrainBrushDensityChanged);
    m_onTerrainBrushDensityChanged = nullptr;
    EventPublisher::Unsubscribe(typeid(TerrainPaintingLayerChanged), m_onTerrainPaintingLayerChanged);
    m_onTerrainPaintingLayerChanged = nullptr;
    EventPublisher::Unsubscribe(typeid(TerrainEditToolSelected), m_onTerrainToolSelected);
    m_onTerrainToolSelected = nullptr;

    EventPublisher::Unsubscribe(typeid(SceneCursorMoved), m_onSceneCursorMoved);
    m_onSceneCursorMoved = nullptr;
    EventPublisher::Unsubscribe(typeid(SceneCursorDragged), m_onSceneCursorDragged);
    m_onSceneCursorDragged = nullptr;

    return ServiceResult::Complete;
}

void TerrainEditConsole::CreateBrushPawn()
{
    SphereDtoHelper sphere_helper(BRUSH_SPHERE_TAG);
    sphere_helper.Sphere(Vector3::ZERO, 1.0f, 32, 32).TextureCoord();
    EffectMaterialDtoHelper eff_dto_helper("DeferredShadingDecalGeometry");
    eff_dto_helper.FilenameAtPath("fx/DeferredShadingDecalGeometry.efx@APK_PATH");
    EffectTextureMapDtoHelper tex_dto_helper;
    tex_dto_helper.TextureMapping("image/terrain_brush_circle.png", "APK_PATH", "terrain_brush_circle", std::nullopt, SEMANTIC_DECAL_MAP);
    MeshPrimitiveDto mesh_dto;
    mesh_dto.Name() = BRUSH_SPHERE_TAG;
    mesh_dto.GeometryName() = BRUSH_SPHERE_TAG;
    mesh_dto.TheGeometry() = sphere_helper.ToGenericDto();
    mesh_dto.Effects().emplace_back(eff_dto_helper.ToGenericDto());
    mesh_dto.TextureMaps().emplace_back(tex_dto_helper.ToGenericDto());
    mesh_dto.RenderListID() = Renderer::RenderListID::DeferredLighting;

    PawnDtoHelper pawn_helper(BRUSH_SPHERE_TAG);
    pawn_helper.MeshPrimitive(mesh_dto).TopLevel(true);
    const Matrix4 local = Matrix4::MakeScaleTransform(static_cast<float>(m_brushSize), static_cast<float>(m_brushSize), static_cast<float>(m_brushSize));
    pawn_helper.LocalTransform(local);
    auto dtos = { pawn_helper.ToGenericDto() };
    CommandBus::Post(std::make_shared<BuildSceneGraph>(BRUSH_SPHERE_TAG, dtos));
}

void TerrainEditConsole::SendTerrainEditCommand(float elapse_time)
{
    if (m_brush.expired()) return;
    if (!m_isEnabled) return;
    switch (m_currMode)
    {
        case TerrainEditMode::Mode_Unknown:
            break;
        case TerrainEditMode::Mode_RaiseHeight:
            CommandBus::Post(std::make_shared<MoveUpTerrainVertex>(m_brush.lock()->GetWorldPosition(), static_cast<float>(m_brushSize), elapse_time * m_brushHeight));
            break;
        case TerrainEditMode::Mode_LowerHeight:
            CommandBus::Post(std::make_shared<MoveUpTerrainVertex>(m_brush.lock()->GetWorldPosition(), static_cast<float>(m_brushSize), -elapse_time * m_brushHeight));
            break;
        case TerrainEditMode::Mode_PaintTexture:
            CommandBus::Post(std::make_shared<PaintTerrainTextureLayer>(m_brush.lock()->GetWorldPosition(), (float)m_brushSize, m_brushDensity, m_layerIndex));
            break;
    }
}

void TerrainEditConsole::OnEditorModeChanged(const IEventPtr& e)
{
    if (!e) return;
    auto ev = std::dynamic_pointer_cast<EditorModeChanged, IEvent>(e);
    if (!ev) return;
    m_isEnabled = ev->GetCurrMode() == EditorMode::Terrain;
    if (m_isEnabled)
    {
        CommandBus::Post(std::make_shared<OutputMessage>("Mode : Terrain Edit Selected"));
        if (m_brush.expired())
        {
            CreateBrushPawn();
        }
        else
        {
            m_brush.lock()->RemoveSpatialFlag(Spatial::SpatialBit::Spatial_Hide);
        }
    }
    else
    {
        if (!m_brush.expired())
        {
            m_brush.lock()->AddSpatialFlag(Spatial::SpatialBit::Spatial_Hide);
        }
    }
}

void TerrainEditConsole::OnSceneGraphBuilt(const Enigma::Frameworks::IEventPtr& e)
{
    if (!e) return;
    auto ev = std::dynamic_pointer_cast<FactorySceneGraphBuilt, IEvent>(e);
    if (!ev) return;
    if (ev->GetSceneGraphId() != BRUSH_SPHERE_TAG) return;
    if (ev->GetTopLevelSpatial().empty()) return;
    auto pawn = std::dynamic_pointer_cast<Pawn, Spatial>(ev->GetTopLevelSpatial().front());
    m_brush = pawn;
    CommandBus::Post(std::make_shared<OutputMessage>("Brush Pawn Created"));
    CommandBus::Post(std::make_shared<Enigma::GameCommon::AttachSceneRootChild>(pawn, pawn->GetLocalTransform()));
}

void TerrainEditConsole::OnPawnPrimitiveBuilt(const Enigma::Frameworks::IEventPtr& e)
{
    if (m_brush.expired()) return;
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<PawnPrimitiveBuilt>(e);
    if (!ev) return;
    if (ev->GetPawn() != m_brush.lock()) return;
    CommandBus::Post(std::make_shared<Enigma::GameCommon::BindGBuffer>(ev->GetPawn()));
}

void TerrainEditConsole::OnTerrainBrushSizeChanged(const Enigma::Frameworks::IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<TerrainBrushSizeChanged>(e);
    if (!ev) return;
    m_brushSize = ev->GetSize();
    if (!m_brush.expired())
    {
        m_brush.lock()->SetLocalUniformScale(static_cast<float>(m_brushSize));
    }
}

void TerrainEditConsole::OnTerrainBrushHeightChanged(const Enigma::Frameworks::IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<TerrainBrushHeightChanged>(e);
    if (!ev) return;
    m_brushHeight = ev->GetHeight();
}

void TerrainEditConsole::OnTerrainBrushDensityChanged(const Enigma::Frameworks::IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<TerrainBrushDensityChanged>(e);
    if (!ev) return;
    m_brushDensity = ev->GetDensity();
}

void TerrainEditConsole::OnTerrainPaintingLayerChanged(const Enigma::Frameworks::IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<TerrainPaintingLayerChanged>(e);
    if (!ev) return;
    m_layerIndex = ev->GetLayer();
}

void TerrainEditConsole::OnTerrainToolSelected(const Enigma::Frameworks::IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<TerrainEditToolSelected>(e);
    if (!ev) return;
    switch (ev->GetTool())
    {
        case TerrainEditToolSelected::Tool::Raise:
            m_currMode = TerrainEditMode::Mode_RaiseHeight;
            break;
        case TerrainEditToolSelected::Tool::Lower:
            m_currMode = TerrainEditMode::Mode_LowerHeight;
            break;
        case TerrainEditToolSelected::Tool::Paint:
            m_currMode = TerrainEditMode::Mode_PaintTexture;
            break;
    }
    CommandBus::Post(std::make_shared<OutputMessage>("Terrain Tool Selected : " + std::to_string(static_cast<int>(m_currMode))));
}

void TerrainEditConsole::OnSceneCursorMoved(const Enigma::Frameworks::IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<SceneCursorMoved>(e);
    if (!ev) return;
    if (!ev->GetHoveredPawn()) return;
    //auto msg = string_format("hover position (%8.3f, %8.3f, %8.3f)\n", ev->GetPosition().X(), ev->GetPosition().Y(), ev->GetPosition().Z());
    //OutputDebugString(msg.c_str());
    if (!m_brush.expired()) m_brush.lock()->ChangeWorldPosition(ev->GetPosition(), std::nullopt);
}

void TerrainEditConsole::OnSceneCursorDragged(const Enigma::Frameworks::IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<SceneCursorDragged>(e);
    if (!ev) return;
    if (!ev->GetHoveredPawn()) return;
    //auto msg = string_format("hover position (%8.3f, %8.3f, %8.3f)\n", ev->GetPosition().X(), ev->GetPosition().Y(), ev->GetPosition().Z());
    //OutputDebugString(msg.c_str());
    if (!m_brush.expired()) m_brush.lock()->ChangeWorldPosition(ev->GetPosition(), std::nullopt);
    SendTerrainEditCommand(0.1f);
}
