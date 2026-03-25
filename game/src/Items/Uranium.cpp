#include <Items/Uranium.hpp>

#include <SuperPupUtilities/Inventory.hpp>

#include <Canis/App.hpp>
#include <Canis/InputManager.hpp>
#include <Canis/ConfigHelper.hpp>

ScriptConf uraniumConf = {};

void RegisterUraniumScript(App& _app)
{
    DEFAULT_CONFIG_AND_REQUIRED(uraniumConf, Uranium, RectTransform);

    uraniumConf.DEFAULT_DRAW_INSPECTOR(Uranium);

    _app.RegisterScript(uraniumConf);
}

DEFAULT_UNREGISTER_SCRIPT(uraniumConf, Uranium)

void Uranium::Create() {}

void Uranium::Ready() {}

void Uranium::Destroy() {}

void Uranium::Update(float _dt) {}

std::string Uranium::GetName()
{
    return "Uranium";
}

std::string Uranium::GetMessage()
{
    return std::string("Press E to Pickup ") + ScriptName;
}

bool Uranium::HandleInteraction()
{
    InputManager& input = entity.scene.GetInputManager();

    if (input.JustPressedKey(Key::E))
    {
        if (Entity* playerEntity = entity.scene.GetEntityWithTag("Player"))
            if (SuperPupUtilities::Inventory* inventory = playerEntity->GetScript<SuperPupUtilities::Inventory>())
                inventory->Add(*this, 1);

        entity.Destroy();
        return true;
    }

    return false;
}
