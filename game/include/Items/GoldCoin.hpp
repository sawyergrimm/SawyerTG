#pragma once

#include <I_Interactable.hpp>
#include <SuperPupUtilities/I_Item.hpp>
#include <Canis/Entity.hpp>

class GoldCoin : public Canis::ScriptableEntity, public SuperPupUtilities::I_Item, public I_Interactable
{
public:
    static constexpr const char* ScriptName = "GoldCoin";

    explicit GoldCoin(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

    void Create();
    void Ready();
    void Destroy();
    void Update(float _dt);

    std::string GetName() override;
    std::string GetMessage() override;
    bool HandleInteraction() override;
};

extern void RegisterGoldCoinScript(Canis::App& _app);
extern void UnRegisterGoldCoinScript(Canis::App& _app);
