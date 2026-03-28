#include <UI/GameUIController.hpp>

#include <Machines/Furnace.hpp>
#include <SuperPupUtilities/Inventory.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/Editor.hpp>
#include <Canis/InputManager.hpp>
#include <Canis/Window.hpp>

#include <algorithm>
#include <cmath>

namespace
{
    constexpr const char* kUraniumItemName = "Uranium";
    constexpr const char* kGoldOreItemName = "Gold Ore";

    Canis::Entity* FindEntity(Canis::Scene& _scene, const std::string& _name)
    {
        return _scene.FindEntityWithName(_name);
    }

    GameUIController::InventoryEntryUI BindInventoryEntry(Canis::Scene& _scene, const std::string& _itemName)
    {
        GameUIController::InventoryEntryUI entry = {};
        entry.itemName = _itemName;
        entry.root = FindEntity(_scene, "RuntimeUI_" + _itemName + "_Tile");
        entry.label = FindEntity(_scene, "RuntimeUI_" + _itemName + "_Label");
        entry.count = FindEntity(_scene, "RuntimeUI_" + _itemName + "_Count");
        return entry;
    }
}

ScriptConf gameUIControllerConf = {};

void RegisterGameUIControllerScript(Canis::App& _app)
{
    DEFAULT_CONFIG_AND_REQUIRED(gameUIControllerConf, GameUIController, Canis::RectTransform, Canis::Canvas);
    RegisterUIAction(gameUIControllerConf, "resume_pause", &GameUIController::OnResumeClicked);
    RegisterUIAction(gameUIControllerConf, "quit_game", &GameUIController::OnQuitClicked);
    RegisterUIAction(gameUIControllerConf, "close_furnace", &GameUIController::OnCloseFurnaceClicked);
    RegisterUIAction(gameUIControllerConf, "drop_furnace_fuel", &GameUIController::OnFurnaceFuelDropped);
    RegisterUIAction(gameUIControllerConf, "drop_furnace_ore", &GameUIController::OnFurnaceOreDropped);

    gameUIControllerConf.DEFAULT_DRAW_INSPECTOR(GameUIController);

    _app.RegisterScript(gameUIControllerConf);
}

DEFAULT_UNREGISTER_SCRIPT(gameUIControllerConf, GameUIController)

void GameUIController::Create()
{
    BindSceneUI();
    SetModalPage(ModalPage::None);
}

void GameUIController::Ready()
{
    if (m_pauseRoot == nullptr || m_furnaceRoot == nullptr || m_inventoryEntries.empty())
        BindSceneUI();
}

void GameUIController::Destroy()
{
    SetModalPage(ModalPage::None);
    ClearUIReferences();
    m_playerEntity = nullptr;
    m_furnaceEntity = nullptr;
    m_worldInteractionSuppressTimer = 0.0f;
}

void GameUIController::Update(float _dt)
{
    if (m_worldInteractionSuppressTimer > 0.0f)
        m_worldInteractionSuppressTimer = std::max(0.0f, m_worldInteractionSuppressTimer - _dt);

    if (m_pauseRoot == nullptr || m_furnaceRoot == nullptr || m_inventoryEntries.empty())
        BindSceneUI();

    if (m_playerEntity == nullptr || !m_playerEntity->active)
        m_playerEntity = entity.scene.GetEntityWithTag("Player");

    Canis::InputManager& input = entity.scene.GetInputManager();
    if (input.JustPressedKey(Canis::Key::ESCAPE))
    {
        if (m_modalPage == ModalPage::PauseMenu || m_modalPage == ModalPage::Furnace)
        {
            SetModalPage(ModalPage::None);
            m_furnaceEntity = nullptr;
        }
        else
        {
            SetModalPage(ModalPage::PauseMenu);
        }
    }

    if (m_modalPage == ModalPage::Furnace)
        RefreshFurnacePopup();
}

void GameUIController::OpenFurnace(Canis::Entity &_furnaceEntity, Canis::Entity &_playerEntity)
{
    if (m_furnaceRoot == nullptr || m_inventoryEntries.empty())
        BindSceneUI();

    m_playerEntity = &_playerEntity;
    m_furnaceEntity = &_furnaceEntity;
    RefreshFurnacePopup();
    SetModalPage(ModalPage::Furnace);
}

void GameUIController::OnResumeClicked(const Canis::UIActionContext &_context)
{
    (void)_context;
    SuppressWorldInteraction();
    SetModalPage(ModalPage::None);
}

void GameUIController::OnQuitClicked(const Canis::UIActionContext &_context)
{
    (void)_context;

    if (entity.scene.app != nullptr)
    {
        Canis::Editor& editor = entity.scene.app->GetEditor();
        const Canis::EditorMode mode = editor.GetMode();
        if (mode == Canis::EditorMode::PLAY || mode == Canis::EditorMode::PAUSE)
        {
            editor.StopPlayMode();
            return;
        }
    }

    entity.scene.GetWindow().RequestClose();
}

void GameUIController::OnCloseFurnaceClicked(const Canis::UIActionContext &_context)
{
    SuppressWorldInteraction();
    m_furnaceEntity = nullptr;
    SetModalPage(ModalPage::None);
}

void GameUIController::OnFurnaceFuelDropped(const Canis::UIActionContext &_context)
{
    if (_context.payloadValue != kUraniumItemName || m_playerEntity == nullptr || m_furnaceEntity == nullptr)
        return;

    SuperPupUtilities::Inventory* inventory = m_playerEntity->GetScript<SuperPupUtilities::Inventory>();
    Furnace* furnace = m_furnaceEntity->GetScript<Furnace>();
    if (inventory == nullptr || furnace == nullptr)
        return;

    if (inventory->Remove(kUraniumItemName, 1))
    {
        furnace->uraniumFuelCount++;
        furnace->TryStartProcessing();
        RefreshFurnacePopup();
    }
}

void GameUIController::OnFurnaceOreDropped(const Canis::UIActionContext &_context)
{
    if (_context.payloadValue != kGoldOreItemName || m_playerEntity == nullptr || m_furnaceEntity == nullptr)
        return;

    SuperPupUtilities::Inventory* inventory = m_playerEntity->GetScript<SuperPupUtilities::Inventory>();
    Furnace* furnace = m_furnaceEntity->GetScript<Furnace>();
    if (inventory == nullptr || furnace == nullptr)
        return;

    if (inventory->Remove(kGoldOreItemName, 1))
    {
        furnace->goldOreCount++;
        furnace->TryStartProcessing();
        RefreshFurnacePopup();
    }
}

void GameUIController::ClearUIReferences()
{
    m_pauseRoot = nullptr;
    m_furnaceRoot = nullptr;
    m_pauseTitleText = nullptr;
    m_resumeButton = nullptr;
    m_quitButton = nullptr;
    m_furnaceTitleText = nullptr;
    m_furnaceStatusText = nullptr;
    m_furnaceFuelCountText = nullptr;
    m_furnaceOreCountText = nullptr;
    m_furnaceCloseButton = nullptr;
    m_furnaceFuelSlot = nullptr;
    m_furnaceOreSlot = nullptr;
    m_inventoryEntries.clear();
}

void GameUIController::BindSceneUI()
{
    ClearUIReferences();

    m_pauseRoot = FindEntity(entity.scene, "RuntimeUI_PauseRoot");
    m_furnaceRoot = FindEntity(entity.scene, "RuntimeUI_FurnaceRoot");

    m_pauseTitleText = FindEntity(entity.scene, "RuntimeUI_PauseTitle");
    m_resumeButton = FindEntity(entity.scene, "RuntimeUI_ResumeButton");
    m_quitButton = FindEntity(entity.scene, "RuntimeUI_QuitButton");

    m_furnaceTitleText = FindEntity(entity.scene, "RuntimeUI_FurnaceTitle");
    m_furnaceStatusText = FindEntity(entity.scene, "RuntimeUI_FurnaceStatus");
    m_furnaceFuelCountText = FindEntity(entity.scene, "RuntimeUI_FuelCount");
    m_furnaceOreCountText = FindEntity(entity.scene, "RuntimeUI_OreCount");
    m_furnaceCloseButton = FindEntity(entity.scene, "RuntimeUI_FurnaceClose");
    m_furnaceFuelSlot = FindEntity(entity.scene, "RuntimeUI_FuelSlot");
    m_furnaceOreSlot = FindEntity(entity.scene, "RuntimeUI_OreSlot");

    m_inventoryEntries.reserve(2);
    m_inventoryEntries.push_back(BindInventoryEntry(entity.scene, kUraniumItemName));
    m_inventoryEntries.push_back(BindInventoryEntry(entity.scene, kGoldOreItemName));

    SetModalPage(m_modalPage);
}

void GameUIController::SuppressWorldInteraction(float _seconds)
{
    m_worldInteractionSuppressTimer = std::max(m_worldInteractionSuppressTimer, _seconds);
}

void GameUIController::SetModalPage(ModalPage _page)
{
    const bool wasModalOpen = (m_modalPage != ModalPage::None);
    m_modalPage = _page;

    if (m_pauseRoot != nullptr)
    {
        const bool showPause = (_page == ModalPage::PauseMenu);
        m_pauseRoot->active = showPause;
        if (m_pauseRoot->HasComponent<Canis::RectTransform>())
            m_pauseRoot->GetComponent<Canis::RectTransform>().active = showPause;
    }
    if (m_furnaceRoot != nullptr)
    {
        const bool showFurnace = (_page == ModalPage::Furnace);
        m_furnaceRoot->active = showFurnace;
        if (m_furnaceRoot->HasComponent<Canis::RectTransform>())
            m_furnaceRoot->GetComponent<Canis::RectTransform>().active = showFurnace;
    }

    const bool isModalOpen = (_page != ModalPage::None);
    entity.scene.SetPaused(isModalOpen);

    if (isModalOpen)
    {
        entity.scene.GetWindow().LockMouse(false);
    }
    else if (wasModalOpen)
    {
        entity.scene.GetWindow().LockMouse(true);
    }
}

void GameUIController::RefreshFurnacePopup()
{
    SuperPupUtilities::Inventory* inventory = (m_playerEntity == nullptr) ? nullptr : m_playerEntity->GetScript<SuperPupUtilities::Inventory>();
    Furnace* furnace = (m_furnaceEntity == nullptr) ? nullptr : m_furnaceEntity->GetScript<Furnace>();

    if (inventory == nullptr || furnace == nullptr)
    {
        if (m_modalPage == ModalPage::Furnace)
            SetModalPage(ModalPage::None);
        return;
    }

    if (m_furnaceFuelCountText != nullptr && m_furnaceFuelCountText->HasComponent<Canis::Text>())
        m_furnaceFuelCountText->GetComponent<Canis::Text>().SetText("Fuel: " + std::to_string(furnace->uraniumFuelCount));

    if (m_furnaceOreCountText != nullptr && m_furnaceOreCountText->HasComponent<Canis::Text>())
        m_furnaceOreCountText->GetComponent<Canis::Text>().SetText("Ore: " + std::to_string(furnace->goldOreCount));

    if (m_furnaceStatusText != nullptr && m_furnaceStatusText->HasComponent<Canis::Text>())
    {
        Canis::Text& statusText = m_furnaceStatusText->GetComponent<Canis::Text>();
        if (furnace->timeLeft > 0.0f)
            statusText.SetText("Smelting... " + std::to_string(static_cast<int>(std::ceil(furnace->timeLeft))) + "s");
        else if (furnace->goldOreCount > 0 && furnace->uraniumFuelCount <= 0)
            statusText.SetText("Need uranium fuel");
        else
            statusText.SetText("Idle");
    }

    for (InventoryEntryUI& entry : m_inventoryEntries)
    {
        if (entry.root == nullptr)
            continue;

        const int count = inventory->GetCount(entry.itemName);
        entry.root->active = count > 0;

        if (entry.count != nullptr && entry.count->HasComponent<Canis::Text>())
            entry.count->GetComponent<Canis::Text>().SetText("x" + std::to_string(count));
    }
}
