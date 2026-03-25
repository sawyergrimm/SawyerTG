#pragma once

#include <Environment/I_Block.hpp>
#include <I_Interactable.hpp>
#include <Canis/AssetHandle.hpp>
#include <Canis/Entity.hpp>

class GoldBlock : public Canis::ScriptableEntity, public I_Block, public I_Interactable
{
public:
    static constexpr const char* ScriptName = "GoldBlock";

    explicit GoldBlock(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

    Canis::SceneAssetHandle dropPrefab = {"assets/prefabs/gold_ore_item.scene"};

    void Create();
    void Ready();
    void Destroy();
    void Update(float _dt);

    std::string GetName() override;
    std::string GetMessage() override;
    bool HandleInteraction() override;
};

extern void RegisterGoldBlockScript(Canis::App& _app);
extern void UnRegisterGoldBlockScript(Canis::App& _app);
