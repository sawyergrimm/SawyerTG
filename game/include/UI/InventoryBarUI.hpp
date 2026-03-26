#pragma once

#include <Canis/Entity.hpp>

#include <SuperPupUtilities/Inventory.hpp>

class InventoryBarUI : public Canis::ScriptableEntity
{
public:
    static constexpr const char* ScriptName = "InventoryBarUI";

    int slotCount = 8;
    float slotSize = 56.0f;
    float slotGap = 10.0f;
    int labelFontSize = 22;

    InventoryBarUI(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

    void Create();
    void Ready();
    void Destroy();
    void Update(float _dt);

private:
    struct SlotView
    {
        Canis::Entity* root = nullptr;
        Canis::Entity* icon = nullptr;
        Canis::Entity* count = nullptr;
    };

    std::vector<SlotView> m_slots = {};
    int m_cachedSlotCount = -1;
    int m_cachedFontSize = -1;
    int m_fontAssetId = -1;

    void RebuildSlots();
    void DestroySlots();
    void UpdateLayout();
    void UpdateVisuals();
    SuperPupUtilities::Inventory* GetInventory() const;
    Canis::Color GetItemColor(const std::string& _itemName) const;
};

extern void RegisterInventoryBarUIScript(Canis::App& _app);
extern void UnRegisterInventoryBarUIScript(Canis::App& _app);
