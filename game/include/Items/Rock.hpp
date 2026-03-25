#pragma once
#include <I_Interactable.hpp>
#include <SuperPupUtilities/I_Item.hpp>
#include <Canis/Entity.hpp>

class Rock : public Canis::ScriptableEntity, public SuperPupUtilities::I_Item, public I_Interactable
{
public:
    static constexpr const char* ScriptName = "Rock";

    Rock(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

    void Create();
    void Ready();
    void Destroy();
    void Update(float _dt);

    // SuperPupUtilities::I_Item
    std::string GetName() override;

    // I_Interactable
    std::string GetMessage() override;
    bool HandleInteraction() override;
};

extern void RegisterRockScript(Canis::App& _app);
extern void UnRegisterRockScript(Canis::App& _app);
