#pragma once
#include <I_Interactable.hpp>
#include <SuperPupUtilities/I_Item.hpp>
#include <Canis/Entity.hpp>

class Gold : public Canis::ScriptableEntity, public SuperPupUtilities::I_Item, public I_Interactable
{
private:

public:
    static constexpr const char* ScriptName = "Gold";

    Gold(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

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

extern void RegisterGoldScript(Canis::App& _app);
extern void UnRegisterGoldScript(Canis::App& _app);