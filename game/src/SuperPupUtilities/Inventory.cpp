#include <SuperPupUtilities/Inventory.hpp>

#include <Canis/App.hpp>
#include <Canis/Debug.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/InputManager.hpp>

namespace SuperPupUtilities
{

    ScriptConf inventoryConf = {};

    void RegisterInventoryScript(App& _app)
    {
        DEFAULT_CONFIG(inventoryConf, Inventory);

        inventoryConf.DEFAULT_DRAW_INSPECTOR(Inventory,
            ImGui::Text("Slots:");

            const int slotCount = component->GetSlotCount();
            if (slotCount == 0)
            {
                ImGui::TextDisabled("none");
            }
            else
            {
                for (int i = 0; i < slotCount; i++)
                    ImGui::Text("%d: %s %d", i, component->GetSlotName(i).c_str(), component->GetSlotItemCount(i));
            }
        );

        _app.RegisterScript(inventoryConf);
    }

    DEFAULT_UNREGISTER_SCRIPT(inventoryConf, Inventory)

    void Inventory::Create() {}

    void Inventory::Ready() {}

    void Inventory::Destroy() {}

    void Inventory::Update(float _dt) {}

    void Inventory::Add(I_Item &_item, int _amount) {
        if (Slot* slot = GetSlot(_item.GetName())) {
            slot->count += _amount;
            return;
        }

        if (_amount <= 0)
            Debug::Warning("Called Inventory::Add with _amount: %i", _amount);
        
        Slot slot;
        slot.name = _item.GetName();
        slot.count = _amount;
        m_slots.push_back(slot);
    }

    bool Inventory::Remove(std::string _name, int _amount)
    {
        if (_amount <= 0)
        {
            Debug::Warning("Called Inventory::Remove with _amount: %i can not remove this amount", _amount);
            return false;
        }

        if (Slot* slot = GetSlot(_name))
        {
            if (slot->count - _amount < 0)
            {
                Debug::Warning("Called Inventory::Remove with _amount: %i can not remove this amount", _amount);
                return false;
            }

            slot->count -= _amount;

            if (slot->count == 0)
            {
                int index = -1;

                for (int i = 0; i < m_slots.size(); i++)
                    if (m_slots[i].name == _name)
                        index = i;

                if (index >= 0)
                    m_slots.erase(m_slots.begin() + index);
            }

            return true;
        }

        Debug::Warning("Inventory::Remove was called but item was not found in Inventory");
        return false;
    }

    bool Inventory::Remove(I_Item &_item, int _amount)
    {
        return Remove(_item.GetName(), _amount);
    }

    int Inventory::GetCount(I_Item &_item) {
        if (Slot* slot = GetSlot(_item.GetName()))
            return slot->count;

        return 0;
    }

    int Inventory::GetSlotCount() const
    {
        return static_cast<int>(m_slots.size());
    }

    std::string Inventory::GetSlotName(int _index) const
    {
        if (_index < 0 || _index >= static_cast<int>(m_slots.size()))
            return "";

        return m_slots[static_cast<size_t>(_index)].name;
    }

    int Inventory::GetSlotItemCount(int _index) const
    {
        if (_index < 0 || _index >= static_cast<int>(m_slots.size()))
            return 0;

        return m_slots[static_cast<size_t>(_index)].count;
    }

    Inventory::Slot* Inventory::GetSlot(std::string _name) {
        for (int i = 0; i < m_slots.size(); i++)
            if (m_slots[i].name == _name)
                return &m_slots[i];

        return nullptr;
    }
}
