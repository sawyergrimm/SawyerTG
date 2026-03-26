#include <UI/InventoryBarUI.hpp>

#include <algorithm>

#include <Canis/App.hpp>
#include <Canis/AssetManager.hpp>
#include <Canis/ConfigHelper.hpp>

namespace
{
    constexpr float kSlotPadding = 8.0f;
}

ScriptConf inventoryBarUIConf = {};

void RegisterInventoryBarUIScript(Canis::App& _app)
{
    REGISTER_PROPERTY(inventoryBarUIConf, InventoryBarUI, slotCount);
    REGISTER_PROPERTY(inventoryBarUIConf, InventoryBarUI, slotSize);
    REGISTER_PROPERTY(inventoryBarUIConf, InventoryBarUI, slotGap);
    REGISTER_PROPERTY(inventoryBarUIConf, InventoryBarUI, labelFontSize);

    DEFAULT_CONFIG_AND_REQUIRED(inventoryBarUIConf, InventoryBarUI, Canis::RectTransform);

    inventoryBarUIConf.DEFAULT_DRAW_INSPECTOR(InventoryBarUI);

    _app.RegisterScript(inventoryBarUIConf);
}

DEFAULT_UNREGISTER_SCRIPT(inventoryBarUIConf, InventoryBarUI)

void InventoryBarUI::Create()
{
    m_fontAssetId = Canis::AssetManager::LoadText("assets/fonts/Antonio-Bold.ttf", std::max(labelFontSize, 8));
    m_cachedFontSize = labelFontSize;
    RebuildSlots();
    UpdateLayout();
    UpdateVisuals();
}

void InventoryBarUI::Ready() {}

void InventoryBarUI::Destroy()
{
    DestroySlots();
}

void InventoryBarUI::Update(float _dt)
{
    (void)_dt;

    if (slotCount < 1)
        slotCount = 1;

    if (m_fontAssetId < 0 || m_cachedSlotCount != slotCount || m_cachedFontSize != labelFontSize)
    {
        m_fontAssetId = Canis::AssetManager::LoadText("assets/fonts/Antonio-Bold.ttf", std::max(labelFontSize, 8));
        m_cachedFontSize = labelFontSize;
        RebuildSlots();
    }

    UpdateLayout();
    UpdateVisuals();
}

void InventoryBarUI::RebuildSlots()
{
    DestroySlots();

    m_cachedSlotCount = std::max(slotCount, 1);

    if (!entity.HasComponent<Canis::RectTransform>())
        return;

    std::vector<Canis::Entity*> staleSlots = entity.GetComponent<Canis::RectTransform>().children;
    for (Canis::Entity* child : staleSlots)
    {
        if (child != nullptr && child->name == "Inventory Slot")
            child->Destroy();
    }

    const Canis::TextureHandle squareTexture = Canis::AssetManager::GetTextureHandle("assets/defaults/textures/square.png");

    m_slots.reserve(static_cast<std::size_t>(m_cachedSlotCount));

    for (int i = 0; i < m_cachedSlotCount; i++)
    {
        SlotView slot = {};

        slot.root = entity.scene.CreateEntity("Inventory Slot");
        Canis::RectTransform* slotRect = slot.root->AddComponent<Canis::RectTransform>();
        slotRect->anchorMin = Canis::Vector2(0.0f, 0.0f);
        slotRect->anchorMax = Canis::Vector2(0.0f, 0.0f);
        slotRect->pivot = Canis::Vector2(0.0f, 0.0f);
        slotRect->SetParent(&entity);
        slotRect->position = Canis::Vector2(0.0f);

        Canis::Sprite2D* slotSprite = slot.root->AddComponent<Canis::Sprite2D>();
        slotSprite->textureHandle = squareTexture;
        slotSprite->color = Canis::Color(0.07f, 0.09f, 0.12f, 0.82f);

        slot.icon = entity.scene.CreateEntity("Inventory Icon");
        Canis::RectTransform* iconRect = slot.icon->AddComponent<Canis::RectTransform>();
        iconRect->anchorMin = Canis::Vector2(0.0f, 0.0f);
        iconRect->anchorMax = Canis::Vector2(1.0f, 1.0f);
        iconRect->pivot = Canis::Vector2(0.5f, 0.5f);
        iconRect->size = Canis::Vector2(-kSlotPadding * 2.0f, -kSlotPadding * 2.0f);
        iconRect->SetParent(slot.root);
        iconRect->position = Canis::Vector2(0.0f);

        Canis::Sprite2D* iconSprite = slot.icon->AddComponent<Canis::Sprite2D>();
        iconSprite->textureHandle = squareTexture;
        iconSprite->color = Canis::Color(0.0f, 0.0f, 0.0f, 0.0f);

        slot.count = entity.scene.CreateEntity("Inventory Count");
        Canis::RectTransform* countRect = slot.count->AddComponent<Canis::RectTransform>();
        countRect->anchorMin = Canis::Vector2(0.0f, 0.0f);
        countRect->anchorMax = Canis::Vector2(1.0f, 1.0f);
        countRect->pivot = Canis::Vector2(0.5f, 0.5f);
        countRect->size = Canis::Vector2(-6.0f, -6.0f);
        countRect->SetParent(slot.root);
        countRect->position = Canis::Vector2(0.0f);

        Canis::Text* countText = slot.count->AddComponent<Canis::Text>();
        countText->assetId = m_fontAssetId;
        countText->alignment = Canis::TextAlignment::CENTER;
        countText->horizontalBoundary = Canis::TextBoundary::TB_OVERFLOW;
        countText->color = Canis::Color(1.0f);

        m_slots.push_back(slot);
    }
}

void InventoryBarUI::DestroySlots()
{
    for (SlotView& slot : m_slots)
    {
        if (slot.count != nullptr)
            slot.count->Destroy();
        if (slot.icon != nullptr)
            slot.icon->Destroy();
        if (slot.root != nullptr)
            slot.root->Destroy();
    }

    m_slots.clear();
}

void InventoryBarUI::UpdateLayout()
{
    if (!entity.HasComponent<Canis::RectTransform>())
        return;

    Canis::RectTransform& rectTransform = entity.GetComponent<Canis::RectTransform>();
    const float width = (slotSize * m_cachedSlotCount) + (slotGap * std::max(m_cachedSlotCount - 1, 0));
    rectTransform.size = Canis::Vector2(width, slotSize);

    for (int i = 0; i < static_cast<int>(m_slots.size()); i++)
    {
        SlotView& slot = m_slots[static_cast<std::size_t>(i)];
        if (slot.root == nullptr || !slot.root->HasComponent<Canis::RectTransform>())
            continue;

        Canis::RectTransform& slotRect = slot.root->GetComponent<Canis::RectTransform>();
        slotRect.position = Canis::Vector2(i * (slotSize + slotGap), 0.0f);
        slotRect.size = Canis::Vector2(slotSize, slotSize);
    }
}

void InventoryBarUI::UpdateVisuals()
{
    SuperPupUtilities::Inventory* inventory = GetInventory();
    const int inventorySlots = (inventory == nullptr) ? 0 : inventory->GetSlotCount();

    for (int i = 0; i < static_cast<int>(m_slots.size()); i++)
    {
        SlotView& slot = m_slots[static_cast<std::size_t>(i)];

        if (slot.root == nullptr || slot.icon == nullptr || slot.count == nullptr)
            continue;
        if (!slot.root->HasComponent<Canis::Sprite2D>() ||
            !slot.icon->HasComponent<Canis::Sprite2D>() ||
            !slot.count->HasComponent<Canis::Text>())
        {
            continue;
        }

        Canis::Sprite2D& slotBackground = slot.root->GetComponent<Canis::Sprite2D>();
        Canis::Sprite2D& iconSprite = slot.icon->GetComponent<Canis::Sprite2D>();
        Canis::Text& countText = slot.count->GetComponent<Canis::Text>();
        countText.assetId = m_fontAssetId;

        if (i < inventorySlots)
        {
            const std::string itemName = inventory->GetSlotName(i);
            const int itemCount = inventory->GetSlotItemCount(i);

            slotBackground.color = Canis::Color(0.14f, 0.17f, 0.21f, 0.95f);
            iconSprite.color = GetItemColor(itemName);
            countText.SetText(std::to_string(itemCount));
            countText.color = Canis::Color(1.0f);
        }
        else
        {
            slotBackground.color = Canis::Color(0.07f, 0.09f, 0.12f, 0.82f);
            iconSprite.color = Canis::Color(0.0f, 0.0f, 0.0f, 0.0f);
            countText.SetText("");
        }
    }
}

SuperPupUtilities::Inventory* InventoryBarUI::GetInventory() const
{
    Canis::Entity* player = entity.scene.GetEntityWithTag("Player");
    if (player == nullptr)
        return nullptr;

    return player->GetScript<SuperPupUtilities::Inventory>();
}

Canis::Color InventoryBarUI::GetItemColor(const std::string& _itemName) const
{
    if (_itemName == "Rock")
        return Canis::Color(0.45f, 0.48f, 0.52f, 1.0f);
    if (_itemName == "Ice")
        return Canis::Color(0.44f, 0.88f, 0.98f, 1.0f);
    if (_itemName == "Gold" || _itemName == "Gold Ore" || _itemName == "Gold Coin")
        return Canis::Color(0.97f, 0.84f, 0.16f, 1.0f);
    if (_itemName == "Uranium")
        return Canis::Color(0.39f, 0.98f, 0.41f, 1.0f);

    return Canis::Color(0.93f, 0.38f, 0.67f, 1.0f);
}
