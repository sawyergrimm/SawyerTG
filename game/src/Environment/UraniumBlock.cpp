#include <Environment/UraniumBlock.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/InputManager.hpp>

ScriptConf uraniumBlockConf = {};

void RegisterUraniumBlockScript(App& _app)
{
    DEFAULT_CONFIG(uraniumBlockConf, UraniumBlock);
    REGISTER_PROPERTY(uraniumBlockConf, UraniumBlock, dropPrefab);

    uraniumBlockConf.DEFAULT_DRAW_INSPECTOR(UraniumBlock);

    _app.RegisterScript(uraniumBlockConf);
}

DEFAULT_UNREGISTER_SCRIPT(uraniumBlockConf, UraniumBlock)

void UraniumBlock::Create() {}

void UraniumBlock::Ready() {}

void UraniumBlock::Destroy() {}

void UraniumBlock::Update(float _dt) {}

std::string UraniumBlock::GetName()
{
    return "Uranium Block";
}

std::string UraniumBlock::GetMessage()
{
    return std::string("Left Click to Break ") + GetName();
}

bool UraniumBlock::HandleInteraction()
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
