#include <Machines/Furnace.hpp>

#include <SuperPupUtilities/Inventory.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/InputManager.hpp>

ScriptConf furnaceConf = {};

void RegisterFurnaceScript(App& _app)
{
    DEFAULT_CONFIG(furnaceConf, Furnace);
    REGISTER_PROPERTY(furnaceConf, Furnace, goldOreCount);
    REGISTER_PROPERTY(furnaceConf, Furnace, timeLeft);
    REGISTER_PROPERTY(furnaceConf, Furnace, processingTime);
    REGISTER_PROPERTY(furnaceConf, Furnace, dropPrefab);

    furnaceConf.DEFAULT_DRAW_INSPECTOR(Furnace);

    _app.RegisterScript(furnaceConf);
}

DEFAULT_UNREGISTER_SCRIPT(furnaceConf, Furnace)

void Furnace::Create() {}

void Furnace::Ready() {}

void Furnace::Destroy() {}

void Furnace::Update(float _dt) {
    if (timeLeft == 0.0f)
        return;
    
    timeLeft -= _dt;

    if (timeLeft <= 0.0f){
        timeLeft = 0.0f;
        goldOreCount--;

        if (goldOreCount > 0)
            timeLeft = processingTime;

        Vector3 spawnOffset = entity.HasComponent<Transform>()
        ? entity.GetComponent<Transform>().GetGlobalPosition() + Vector3(0.0f, 0.0f, 1.0f)
        : Vector3(0.0f);

        for (Entity *root : entity.scene.Instantiate(dropPrefab))
        {
            if (root != nullptr && root->HasComponent<Transform>())
                root->GetComponent<Transform>().position += spawnOffset;
        }
    }
}

std::string Furnace::GetMessage()
{
    return std::string("Left Click to add Gold Ore.");
}

bool Furnace::HandleInteraction()
{
    InputManager& input = entity.scene.GetInputManager();
    if (!input.LeftClickReleased())
        return false;
    
    if (Entity* playerEntity = entity.scene.GetEntityWithTag("Player"))
        if (SuperPupUtilities::Inventory* inventory = playerEntity->GetScript<SuperPupUtilities::Inventory>())
            if (inventory->Remove("Gold Ore", 1))
                goldOreCount++;

    if (timeLeft == 0.0f && goldOreCount != 0)
        timeLeft = processingTime;

    return false;
}
