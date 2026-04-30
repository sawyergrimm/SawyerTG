#pragma once

#include <Canis/Entity.hpp>

namespace Canis
{
    class App;
}

namespace AICombat
{
    struct Health
    {
    public:
        static constexpr const char* ScriptName = "AICombat::Health";

        Health() = default;
        explicit Health(Canis::Entity& _entity) : entity(&_entity) {}

        void Create() {}
        Canis::Entity* entity = nullptr;
        bool active = true;
        int currentHealth;
        int maxHealth;
    };

    void RegisterHealthComponent(Canis::App& _app);
    void UnRegisterHealthComponent(Canis::App& _app);
}
