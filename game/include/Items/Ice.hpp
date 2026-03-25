#pragma once
#include <I_Interactable.hpp>
#include <SuperPupUtilities/I_Item.hpp>
#include <Canis/Entity.hpp>

class Ice : public Canis::ScriptableEntity, public SuperPupUtilities::I_Item, public I_Interactable
{
public:
    static constexpr const char* ScriptName = "Ice";

    Ice(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

    void Create();
    void Ready();
    void Destroy();
    void Update(float _dt);

    std::string GetName() override;
    std::string GetMessage() override;
    bool HandleInteraction() override;
};

extern void RegisterIceScript(Canis::App& _app);
extern void UnRegisterIceScript(Canis::App& _app);
