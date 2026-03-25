#include <Environment/GoldBlock.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/InputManager.hpp>

ScriptConf goldBlockConf = {};

void RegisterGoldBlockScript(App& _app)
{
    DEFAULT_CONFIG_AND_REQUIRED(goldBlockConf, GoldBlock);
    REGISTER_PROPERTY(goldBlockConf, GoldBlock, dropPrefab);

    goldBlockConf.DEFAULT_DRAW_INSPECTOR(GoldBlock);

    _app.RegisterScript(goldBlockConf);
}

DEFAULT_UNREGISTER_SCRIPT(goldBlockConf, GoldBlock)

void GoldBlock::Create() {}

void GoldBlock::Ready() {}

void GoldBlock::Destroy() {}

void GoldBlock::Update(float _dt) {}

std::string GoldBlock::GetName()
{
    return "Gold Block";
}

std::string GoldBlock::GetMessage()
{
    return std::string("Left Click to Break ") + GetName();
}

bool GoldBlock::HandleInteraction()
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
