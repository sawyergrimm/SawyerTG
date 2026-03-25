#pragma once

#include <Canis/Entity.hpp>

#include <I_Interactable.hpp>

class Furnace : public Canis::ScriptableEntity, public I_Interactable
{
private:

public:
    static constexpr const char* ScriptName = "Furnace";

    int goldOreCount = 0;
    float timeLeft = 0.0f;
    float processingTime = 5.0f;
    Canis::SceneAssetHandle dropPrefab;

    Furnace(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

    void Create();
    void Ready();
    void Destroy();
    void Update(float _dt);

    std::string GetMessage() override;
    bool HandleInteraction() override;
};

extern void RegisterFurnaceScript(Canis::App& _app);
extern void UnRegisterFurnaceScript(Canis::App& _app);