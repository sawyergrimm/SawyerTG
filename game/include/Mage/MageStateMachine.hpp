#pragma once

#include <Canis/Entity.hpp>

#include <SuperPupUtilities/StateMachine.hpp>
#include <AICombat/Health.hpp>
#include <string>

namespace Mage
{
    class MageStateMachine;

    class IdleState : public SuperPupUtilities::State
    {
    public:
        static constexpr const char* Name = "IdleState";

        explicit IdleState(SuperPupUtilities::StateMachine& _stateMachine);
        void Enter() override;
        void Update(float _dt) override;
    };

    class ChaseState : public SuperPupUtilities::State
    {
    public:
        static constexpr const char* Name = "ChaseState";
        float moveSpeed = 4.0f;

        explicit ChaseState(SuperPupUtilities::StateMachine& _stateMachine);
        void Enter() override;
        void Update(float _dt) override;
    };

    class FireState : public SuperPupUtilities::State
    {
    public:
        static constexpr const char* Name = "FireState";
        float healRange = 2.00f;
        float healTime = 0.75f;
        float healAmmount = 0.25f;

        explicit FireState(SuperPupUtilities::StateMachine& _stateMachine);
        void Enter() override;
        void Update(float _dt) override;
        void Exit() override;
    };

    class MageStateMachine : public SuperPupUtilities::StateMachine
    {
    public:
        static constexpr const char* ScriptName = "Mage::MageStateMachine";

        float countdown = 2.0f;
        std::string teamTag = "";
        std::string team = "";
        float detectionRange = 20.0f;
        Canis::Vector3 bodyColliderSize = Canis::Vector3(1.0f);
        int maxHealth = 40;
        AICombat::Health healthComponent;
        bool logStateChanges = true;
        Canis::AudioAssetHandle healSfxPath = { .path = "assets/audio/sfx/heal_1.ogg" };
        float healSfxVolume = 1.0f;
        Canis::SceneAssetHandle deathEffectPrefab = { .path = "assets/prefabs/Mage_death_particles.scene" };
        float projectileSpeed;
        float projectileLifeTime;
        float projectileHitImpulse;
        Canis::SceneAssetHandle projectile;


        explicit MageStateMachine(Canis::Entity& _entity);

        IdleState idleState;
        ChaseState chaseState;
        FireState fireState;

        void Create() override;
        void Ready() override;
        void Destroy() override;
        void Update(float _dt) override;

        Canis::Entity* FindClosestTarget() const;
        float DistanceTo(const Canis::Entity& _other) const;
        void FaceTarget(const Canis::Entity& _target);
        void MoveTowards(const Canis::Entity& _target, float _speed, float _dt);
        void ChangeState(const std::string& _stateName);
        const std::string& GetCurrentStateName() const;
        float GetStateTime() const;
        float GetHealRange() const;
        int GetCurrentHealth() const;

        void TakeDamage(int _damage);
        bool IsAlive() const;
        void ReportHealth();
        void Heal(Canis::Entity*);
        void Fire(const Canis::Vector3& _position, const Canis::Vector3& _direction);
        void AltFire(Canis::SceneAssetHandle projectile);

    private:
        void PlayHealSFX();
        void SpawnDeathEffect();

        int m_currentHealth = 0;
        float m_stateTime = 0.0f;
        Canis::Vector4 m_baseColor = Canis::Vector4(1.0f);
        bool m_hasBaseColor = false;
        bool m_useFirstHitSfx = true;
    };

    void RegisterMageStateMachineScript(Canis::App& _app);
    void UnRegisterMageStateMachineScript(Canis::App& _app);
}