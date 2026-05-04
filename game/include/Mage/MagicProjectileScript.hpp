#pragma once

#include <Canis/Entity.hpp>
#include <AICombat/Team.hpp>
#include <AICombat/Health.hpp>
#include <Canis/AudioManager.hpp>
#include <algorithm>
namespace Canis
{
    class App;
}

namespace Mage
{
    class MagicProjectileScript : public Canis::ScriptableEntity
    {
    public:
        static constexpr const char* ScriptName = "Mage::MagicProjectileScript";
        Canis::SceneAssetHandle deathEffectPrefab = { .path = "assets/prefabs/brawler_death_particles.scene" };
        Canis::AudioAssetHandle hitSfxPath1 = { .path = "assets/audio/sfx/hit_1.ogg" };
        Canis::AudioAssetHandle projectilePath = { .path = "assets/audio/sfx/projectile.ogg" };

        explicit MagicProjectileScript(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}

        void Create() override;
        void Ready() override;
        void Destroy() override;
        void Update(float _dt) override;
        void CollisionCheck(const Canis::Vector3& _start, const Canis::Vector3& _end);
        void SpawnDeathEffect(Canis::Entity* other);
        void PlayHitSfx();
    };

    void RegisterMagicProjectileScriptScript(Canis::App& _app);
    void UnRegisterMagicProjectileScriptScript(Canis::App& _app);
}
