#pragma once

#include <Canis/Entity.hpp>

#include <vector>

namespace AICombat
{
    class BrawlerStateMachine;

    class HammerDamage : public Canis::ScriptableEntity
    {
    public:
        static constexpr const char* ScriptName = "AICombat::HammerDamage";

        Canis::Entity* owner = nullptr;
        Canis::Vector3 sensorSize = Canis::Vector3(1.0f);
        int damage = 10;
        std::string teamTag = "";
        Canis::AudioAssetHandle hitSfxPath1 = { .path = "assets/audio/sfx/hit_1.ogg" };
        Canis::AudioAssetHandle hitSfxPath2 = { .path = "assets/audio/sfx/hit_2.ogg" };
        float hitSfxVolume = 0.1f;
        Canis::SceneAssetHandle deathEffectPrefab = { .path = "assets/prefabs/brawler_death_particles.scene" };

        explicit HammerDamage(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}

        void Create() override;
        void Ready() override;
        void Update(float _dt) override;

        void CheckSensorEnter();
        void PlayHitSfx();
        void SpawnDeathEffect(Canis::Entity* other);

    private:
        BrawlerStateMachine* GetOwnerStateMachine();
        Canis::Entity* FindOwnerFromHierarchy() const;
        bool HasDamagedThisSwing(Canis::Entity& _target) const;
        bool m_useFirstHitSfx = true;

        std::vector<Canis::Entity*> m_hitTargetsThisSwing = {};
    };

    void RegisterHammerDamageScript(Canis::App& _app);
    void UnRegisterHammerDamageScript(Canis::App& _app);
}
