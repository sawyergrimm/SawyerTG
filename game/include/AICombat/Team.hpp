#pragma once

#include <Canis/Entity.hpp>

namespace Canis
{
    class App;
}

namespace AICombat
{
    struct Team
    {
    public:
        static constexpr const char* ScriptName = "AICombat::Team";

        Team() = default;
        explicit Team(Canis::Entity& _entity) : entity(&_entity) {}

        void Create() {}
        Canis::Entity* entity = nullptr;
        bool active = true;
        std::string team = "";
    };

    void RegisterTeamComponent(Canis::App& _app);
    void UnRegisterTeamComponent(Canis::App& _app);
}
