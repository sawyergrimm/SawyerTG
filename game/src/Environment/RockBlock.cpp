#include <Environment/RockBlock.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/InputManager.hpp>

ScriptConf rockBlockConf = {};

void RegisterRockBlockScript(App& _app)
{
    DEFAULT_CONFIG(rockBlockConf, RockBlock);
    REGISTER_PROPERTY(rockBlockConf, RockBlock, dropPrefab);

    rockBlockConf.DEFAULT_DRAW_INSPECTOR(RockBlock);

    _app.RegisterScript(rockBlockConf);
}

DEFAULT_UNREGISTER_SCRIPT(rockBlockConf, RockBlock)

void RockBlock::Create() {}

void RockBlock::Ready() {}

void RockBlock::Destroy() {}

void RockBlock::Update(float _dt) {}

std::string RockBlock::GetName()
{
    return "Rock Block";
}

std::string RockBlock::GetMessage()
{
    return std::string("Left Click to Break ") + GetName();
}

bool RockBlock::HandleInteraction()
{
    InputManager& input = entity.scene.GetInputManager();
    if (!input.LeftClickReleased())
        return false;

    const Vector3 spawnOffset = entity.HasComponent<Transform>()
        ? entity.GetComponent<Transform>().GetGlobalPosition()
        : Vector3(0.0f);
    Scene& scene = entity.scene;

    entity.Destroy();

    for (Entity *root : scene.Instantiate(dropPrefab))
    {
        if (root != nullptr && root->HasComponent<Transform>())
            root->GetComponent<Transform>().position += spawnOffset;
    }

    return true;
}
