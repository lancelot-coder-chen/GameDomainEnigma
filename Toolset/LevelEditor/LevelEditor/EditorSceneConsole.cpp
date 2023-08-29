#include "EditorSceneConsole.h"
#include "Frameworks/EventPublisher.h"
#include "InputHandlers/MouseInputEvents.h"
#include "Renderer/RendererEvents.h"
#include "GameCommon/GameSceneEvents.h"
#include "GameCommon/GameCameraEvents.h"
#include "ScenePicker.h"
#include "LevelEditorEvents.h"

using namespace LevelEditor;
using namespace Enigma::Frameworks;
using namespace Enigma::InputHandlers;
using namespace Enigma::Renderer;
using namespace Enigma::GameCommon;
using namespace Enigma::MathLib;

Rtti EditorSceneConsole::TYPE_RTTI{ "LevelEditor.EditorSceneConsole", &ISystemService::TYPE_RTTI };

EditorSceneConsole::EditorSceneConsole(Enigma::Frameworks::ServiceManager* srv_mngr) : ISystemService(srv_mngr)
{
    m_needTick = false;
}

EditorSceneConsole::~EditorSceneConsole()
{
}

Enigma::Frameworks::ServiceResult EditorSceneConsole::OnInit()
{
    m_onGameCameraCreated = std::make_shared<EventSubscriber>([=](auto e) { OnGameCameraCreated(e); });
    EventPublisher::Subscribe(typeid(GameCameraCreated), m_onGameCameraCreated);
    m_onSceneRootCreated = std::make_shared<EventSubscriber>([=](auto e) { OnSceneRootCreated(e); });
    EventPublisher::Subscribe(typeid(SceneRootCreated), m_onSceneRootCreated);
    m_onTargetViewportChanged = std::make_shared<EventSubscriber>([=](auto e) { OnTargetViewportChanged(e); });
    EventPublisher::Subscribe(typeid(TargetViewPortChanged), m_onTargetViewportChanged);
    m_onMouseMoved = std::make_shared<EventSubscriber>([=](auto e) { OnMouseMoved(e); });
    EventPublisher::Subscribe(typeid(MouseMoved), m_onMouseMoved);

    return ServiceResult::Complete;
}

Enigma::Frameworks::ServiceResult EditorSceneConsole::OnTerm()
{
    EventPublisher::Unsubscribe(typeid(GameCameraCreated), m_onGameCameraCreated);
    m_onGameCameraCreated = nullptr;
    EventPublisher::Unsubscribe(typeid(SceneRootCreated), m_onSceneRootCreated);
    m_onSceneRootCreated = nullptr;
    EventPublisher::Unsubscribe(typeid(TargetViewPortChanged), m_onTargetViewportChanged);
    m_onTargetViewportChanged = nullptr;
    EventPublisher::Unsubscribe(typeid(MouseMoved), m_onMouseMoved);
    m_onMouseMoved = nullptr;

    return ServiceResult::Complete;
}

void EditorSceneConsole::OnGameCameraCreated(const Enigma::Frameworks::IEventPtr& e)
{
    if (!e) return;
    auto ev = std::dynamic_pointer_cast<Enigma::GameCommon::GameCameraCreated, IEvent>(e);
    if (!ev) return;
    m_camera = ev->GetCamera();
}

void EditorSceneConsole::OnSceneRootCreated(const Enigma::Frameworks::IEventPtr& e)
{
    if (!e) return;
    auto ev = std::dynamic_pointer_cast<Enigma::GameCommon::SceneRootCreated, IEvent>(e);
    if (!ev) return;
    m_sceneRoot = ev->GetSceneRoot();
}
void EditorSceneConsole::OnTargetViewportChanged(const Enigma::Frameworks::IEventPtr& e)
{
    if (!e) return;
    auto ev = std::dynamic_pointer_cast<Enigma::Renderer::TargetViewPortChanged, IEvent>(e);
    if (!ev) return;
    m_targetViewport = ev->GetViewPort();
}

void EditorSceneConsole::OnMouseMoved(const Enigma::Frameworks::IEventPtr& e)
{
    if (!e) return;
    const auto ev = std::dynamic_pointer_cast<MouseMoved>(e);
    if (!ev) return;
    auto clipping_pos = m_targetViewport.ViewportPositionToClippingPosition(
        Enigma::MathLib::Vector2(static_cast<float>(ev->m_param.m_x), static_cast<float>(ev->m_param.m_y)));
    auto [pickedPawn, picked_pos] = PickingOnSceneView(clipping_pos);
    if (pickedPawn)
    {
        EventPublisher::Post(std::make_shared<SceneCursorMoved>(picked_pos, pickedPawn));
    }
}

std::tuple<std::shared_ptr<Enigma::SceneGraph::Pawn>, Enigma::MathLib::Vector3> EditorSceneConsole::PickingOnSceneView(const Enigma::MathLib::Vector2& clip_pos)
{
    if (m_camera.expired()) return { nullptr, Enigma::MathLib::Vector3::ZERO };
    if (m_sceneRoot.expired()) return { nullptr, Vector3::ZERO };

    ScenePicker scenePicker;
    scenePicker.SetAssociatedCamera(m_camera.lock());
    scenePicker.SetClippingCoord(clip_pos);
    scenePicker.Picking(m_sceneRoot.lock());

    unsigned int count = scenePicker.GetRecordCount();
    if (count == 0)
    {
        return { nullptr, Vector3::ZERO };
    }

    std::shared_ptr<Enigma::SceneGraph::Pawn> pickedPawn = nullptr;
    Vector3 picked_pos;
    for (unsigned int i = 0; i < count; i++)
    {
        auto pick_record = scenePicker.GetPickRecord(i);
        if (!pick_record) continue;
        if (pick_record->m_spatial.expired()) continue;
        pickedPawn = std::dynamic_pointer_cast<Enigma::SceneGraph::Pawn, Enigma::SceneGraph::Spatial>(pick_record->m_spatial.lock());
        picked_pos = pick_record->m_vecIntersection;
        if (pickedPawn) break;
    }
    return { pickedPawn, picked_pos };
}