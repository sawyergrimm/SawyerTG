#include <AICombat/Team.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>

namespace AICombat
{
    namespace
    {
        Canis::ComponentConf componentConf = {};
    }

    void RegisterTeamComponent(Canis::App& _app)
    {
        // REGISTER_PROPERTY(componentConf, AICombat::Team, exampleProperty);
        REGISTER_PROPERTY(componentConf, AICombat::Team, team);

        DEFAULT_COMPONENT_CONFIG(componentConf, AICombat::Team);

        componentConf.DEFAULT_DRAW_COMPONENT_INSPECTOR(AICombat::Team);

        _app.RegisterComponent(componentConf);
    }

    DEFAULT_UNREGISTER_COMPONENT(componentConf, Team)
}
