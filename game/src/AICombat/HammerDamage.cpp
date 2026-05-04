#include <AICombat/HammerDamage.hpp>

#include <AICombat/BrawlerStateMachine.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/Debug.hpp>
#include <Canis/AudioManager.hpp>
#include <algorithm>

namespace AICombat
{
    namespace
    {
        ScriptConf hammerDamageConf = {};
    }

    void RegisterHammerDamageScript(Canis::App& _app)
    {
        REGISTER_PROPERTY(hammerDamageConf, AICombat::HammerDamage, owner);
        REGISTER_PROPERTY(hammerDamageConf, AICombat::HammerDamage, sensorSize);
        REGISTER_PROPERTY(hammerDamageConf, AICombat::HammerDamage, damage);
        REGISTER_PROPERTY(hammerDamageConf, AICombat::HammerDamage, hitSfxPath1);
        REGISTER_PROPERTY(hammerDamageConf, AICombat::HammerDamage, hitSfxPath2);
        REGISTER_PROPERTY(hammerDamageConf, AICombat::HammerDamage, hitSfxVolume);

        DEFAULT_CONFIG_AND_REQUIRED(
            hammerDamageConf,
            AICombat::HammerDamage,
            Canis::Transform,
            Canis::Rigidbody,
            Canis::BoxCollider);

        hammerDamageConf.DEFAULT_DRAW_INSPECTOR(AICombat::HammerDamage);

        _app.RegisterScript(hammerDamageConf);
    }

    DEFAULT_UNREGISTER_SCRIPT(hammerDamageConf, HammerDamage)

    void HammerDamage::Create()
    {
        entity.GetComponent<Canis::Transform>();

        Canis::Rigidbody& rigidbody = entity.GetComponent<Canis::Rigidbody>();
        rigidbody.motionType = Canis::RigidbodyMotionType::STATIC;
        rigidbody.useGravity = false;
        rigidbody.isSensor = true;
        rigidbody.allowSleeping = false;
        rigidbody.linearVelocity = Canis::Vector3(0.0f);
        rigidbody.angularVelocity = Canis::Vector3(0.0f);

        entity.GetComponent<Canis::BoxCollider>().size = sensorSize;
        Canis::AudioAssetHandle hitSfxPath1 = { .path = "assets/audio/sfx/hit_1.ogg" };
        Canis::AudioAssetHandle hitSfxPath2 = { .path = "assets/audio/sfx/hit_2.ogg" };
    }

    void HammerDamage::Ready()
    {
        if (owner == nullptr)
            owner = FindOwnerFromHierarchy();

        static AICombat::Team teamComponent = entity.GetComponent<AICombat::Team>();
        teamComponent.team = owner->GetComponent<AICombat::Team>().team;
        m_useFirstHitSfx = true;
    }

    void HammerDamage::Update(float)
    {
        CheckSensorEnter();
    }

    void HammerDamage::CheckSensorEnter()
    {
        if (!entity.HasComponents<Canis::BoxCollider, Canis::Rigidbody>())
            return;

        BrawlerStateMachine* ownerStateMachine = GetOwnerStateMachine();
        if (ownerStateMachine == nullptr || !ownerStateMachine->IsAlive())
        {
            m_hitTargetsThisSwing.clear();
            return;
        }

        const bool damageWindowOpen =
            ownerStateMachine->GetCurrentStateName() == HammerTimeState::Name &&
            ownerStateMachine->GetStateTime() >= ownerStateMachine->hammerTimeState.attackDamageTime;

        if (!damageWindowOpen)
        {
            m_hitTargetsThisSwing.clear();
            return;
        }
        for (Canis::Entity* other : entity.GetComponent<Canis::BoxCollider>().entered)
        {
            if (other == nullptr || !other->active || other == owner || HasDamagedThisSwing(*other))
                continue;

            if (other->GetComponent<AICombat::Health>().currentHealth <= 0)
                continue;

            if (other->GetComponent<AICombat::Team>().team == entity.GetComponent<AICombat::Team>().team)
                continue;

            other->GetComponent<AICombat::Health>().currentHealth -= damage;
            PlayHitSfx();
            if (other->GetComponent<AICombat::Health>().currentHealth <= 0) {
                SpawnDeathEffect(other);
                other->Destroy();
            }

            m_hitTargetsThisSwing.push_back(other);
        }
    }

    BrawlerStateMachine* HammerDamage::GetOwnerStateMachine()
    {
        if (owner == nullptr)
            owner = FindOwnerFromHierarchy();

        if (owner == nullptr || !owner->active)
            return nullptr;

        return owner->GetScript<BrawlerStateMachine>();
    }

    Canis::Entity* HammerDamage::FindOwnerFromHierarchy() const
    {
        if (!entity.HasComponent<Canis::Transform>())
            return nullptr;

        Canis::Entity* current = entity.GetComponent<Canis::Transform>().parent;
        while (current != nullptr)
        {
            if (current->HasScript<BrawlerStateMachine>())
                return current;

            if (!current->HasComponent<Canis::Transform>())
                break;

            current = current->GetComponent<Canis::Transform>().parent;
        }

        return nullptr;
    }

    bool HammerDamage::HasDamagedThisSwing(Canis::Entity& _target) const
    {
        return std::find(m_hitTargetsThisSwing.begin(), m_hitTargetsThisSwing.end(), &_target)
            != m_hitTargetsThisSwing.end();
    }

    void HammerDamage::PlayHitSfx()
    {
        Canis::AudioAssetHandle& selectedSfx = m_useFirstHitSfx ? hitSfxPath1 : hitSfxPath2;
        m_useFirstHitSfx = !m_useFirstHitSfx;

        if (selectedSfx.Empty())
            return;

        Canis::AudioManager::PlaySFX(selectedSfx, std::clamp(hitSfxVolume, 0.0f, 1.0f));
    }

    void HammerDamage::SpawnDeathEffect(Entity* other)
    {
        if (deathEffectPrefab.Empty() || !other->HasComponent<Canis::Transform>())
            return;

        const Canis::Transform& sourceTransform = other->GetComponent<Canis::Transform>();
        const Canis::Vector3 spawnPosition = sourceTransform.GetGlobalPosition();
        const Canis::Vector3 spawnRotation = sourceTransform.GetGlobalRotation();

        for (Canis::Entity* spawnedEntity : other->scene.Instantiate(deathEffectPrefab))
        {
            if (spawnedEntity == nullptr || !spawnedEntity->HasComponent<Canis::Transform>())
                continue;

            Canis::Transform& spawnedTransform = spawnedEntity->GetComponent<Canis::Transform>();
            spawnedTransform.position = spawnPosition;
            spawnedTransform.rotation = spawnRotation;
        }
    }
}
