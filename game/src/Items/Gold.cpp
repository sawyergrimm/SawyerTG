#include <Items/Gold.hpp>

#include <SuperPupUtilities/Inventory.hpp>

#include <Canis/App.hpp>
#include <Canis/InputManager.hpp>
#include <Canis/ConfigHelper.hpp>


ScriptConf goldConf = {};

void RegisterGoldScript(App& _app)
{
    DEFAULT_CONFIG_AND_REQUIRED(goldConf, Gold, RectTransform);

    goldConf.DEFAULT_DRAW_INSPECTOR(Gold);

    _app.RegisterScript(goldConf);
}

DEFAULT_UNREGISTER_SCRIPT(goldConf, Gold)

void Gold::Create() {}

void Gold::Ready() {}

void Gold::Destroy() {}

void Gold::Update(float _dt) {}

std::string Gold::GetName() {
    return "Gold";
}

std::string Gold::GetMessage() {
    return std::string("Press E to Pickup ") + ScriptName;
}

bool Gold::HandleInteraction() {
    InputManager& input = entity.scene.GetInputManager();

    if (input.JustPressedKey(Key::E)) {
        if (Entity* playerEntity = entity.scene.GetEntityWithTag("Player"))
            if (SuperPupUtilities::Inventory* inventory = playerEntity->GetScript<SuperPupUtilities::Inventory>())
                inventory->Add(*this, 1);

        entity.Destroy();
        return true;
    }

    return false;
}