#include <AICombat/HammerDamage.hpp>

#include <AICombat/BrawlerStateMachine.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/Debug.hpp>
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
    }

    void HammerDamage::Ready()
    {
        if (owner == nullptr)
            owner = FindOwnerFromHierarchy();

        static AICombat::Team teamComponent = entity.GetComponent<AICombat::Team>();
        teamComponent.team = owner->GetComponent<AICombat::Team>().team;
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
            if (other->GetComponent<AICombat::Health>().currentHealth <= 0) {
                other->SpawnDeathEffect();
                other->Destroy();
            }

            m_hitTargetsThisSwing.push_back(other);
            Canis::Debug::Log("Checkpoint %d", std::to_string(other->GetComponent<AICombat::Health>().currentHealth).c_str());
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
}
