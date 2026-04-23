#include <AICombat/Health.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>

namespace AICombat
{
    namespace
    {
        Canis::ComponentConf componentConf = {};
    }

    void RegisterHealthComponent(Canis::App& _app)
    {
        // REGISTER_PROPERTY(componentConf, AICombat::Health, exampleProperty);

        DEFAULT_COMPONENT_CONFIG(componentConf, AICombat::Health);

        componentConf.DEFAULT_DRAW_COMPONENT_INSPECTOR(AICombat::Health);

        _app.RegisterComponent(componentConf);
    }

    DEFAULT_UNREGISTER_COMPONENT(componentConf, Health)
}
