#include <Mage/MageStateMachine.hpp>

#include <Canis/App.hpp>
#include <Canis/AudioManager.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/Debug.hpp>
#include <AICombat/Health.hpp>
#include <AICombat/Team.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace Mage
{
    namespace
    {
        ScriptConf mageStateMachineConf = {};
    }

    IdleState::IdleState(SuperPupUtilities::StateMachine& _stateMachine) :
        State(Name, _stateMachine) {}

    void IdleState::Enter()
    {
        //if (MageStateMachine* mageStatMachine = dynamic_cast<MageStateMachine*>(m_stateMachine))
            //mageStatMachine->ResetHammerPose();
    }

    void IdleState::Update(float)
    {
        if (MageStateMachine* mageStatMachine = dynamic_cast<MageStateMachine*>(m_stateMachine))
        {
            mageStatMachine->ReportHealth();
            if (mageStatMachine->FindLowestTarget() != nullptr)
                mageStatMachine->ChangeState(ChaseState::Name);
        }
    }

    ChaseState::ChaseState(SuperPupUtilities::StateMachine& _stateMachine) :
        State(Name, _stateMachine) {}

    void ChaseState::Enter()
    {
        //if (MageStateMachine* mageStatMachine = dynamic_cast<MageStateMachine*>(m_stateMachine))
            //mageStatMachine->ResetHammerPose();
    }

    void ChaseState::Update(float _dt)
    {
        MageStateMachine* mageStatMachine = dynamic_cast<MageStateMachine*>(m_stateMachine);
        if (mageStatMachine == nullptr)
            return;

        Canis::Entity* target = mageStatMachine->FindLowestTarget();

        if (target == nullptr)
        {
            mageStatMachine->ChangeState(IdleState::Name);
            return;
        }

        mageStatMachine->FaceTarget(*target);
        Canis::Vector3 targetPosition = target->GetComponent<Canis::Transform>().position;
        if (glm::length(mageStatMachine->entity.GetComponent<Canis::Transform>().position - (targetPosition - target->GetComponent<Canis::Transform>().GetForward() * 2.0f)) < 0.5f)
        {
            Canis::Debug::Log("IM TRYNA CHAGE");
            mageStatMachine->ChangeState(HealState::Name);
            return;
        }

        mageStatMachine->MoveTowards(*target, moveSpeed, _dt);
    }

    HealState::HealState(SuperPupUtilities::StateMachine& _stateMachine) :
        State(Name, _stateMachine) {}

    void HealState::Enter()
    {
        MageStateMachine* mageStatMachine = dynamic_cast<MageStateMachine*>(m_stateMachine);
        mageStatMachine->countdown = 2.0f;
        //if (MageStateMachine* mageStatMachine = dynamic_cast<MageStateMachine*>(m_stateMachine))
            //mageStatMachine->SetHammerSwing(0.0f);
    }

    void HealState::Update(float _dt)
    {
        MageStateMachine* mageStatMachine = dynamic_cast<MageStateMachine*>(m_stateMachine);
        if (mageStatMachine == nullptr)
            return;

        if (Canis::Entity* target = mageStatMachine->FindLowestTarget())
            mageStatMachine->FaceTarget(*target);
        else {return;}


        if (mageStatMachine->countdown > 0.0f) {
            mageStatMachine->countdown -= _dt;
            if (Canis::Entity* target = mageStatMachine->FindLowestTarget()) {
                if (target->GetComponent<AICombat::Health>().currentHealth < target->GetComponent<AICombat::Health>().maxHealth) {
                    mageStatMachine->entity.GetComponent<Canis::PointLight>().intensity = 10.0f * (2.0f - mageStatMachine->countdown / 2) - 8.0f;
                }
                if (mageStatMachine->DistanceTo(*target) > 3.0f) {
                    mageStatMachine->entity.GetComponent<Canis::PointLight>().intensity = 0.0f;
                    mageStatMachine->ChangeState(ChaseState::Name);
                }
            }
        }
        else {
            mageStatMachine->Heal(mageStatMachine->FindLowestTarget());
            mageStatMachine->countdown = 2.0f;
            mageStatMachine->entity.GetComponent<Canis::PointLight>().intensity = 0.0f;
            mageStatMachine->ChangeState(IdleState::Name);
        }

    }

    void HealState::Exit()
    {
    }

    MageStateMachine::MageStateMachine(Canis::Entity& _entity) :
        SuperPupUtilities::StateMachine(_entity),
        idleState(*this),
        chaseState(*this),
        healState(*this) {}

    void RegisterMageStateMachineScript(Canis::App& _app)
    {
        REGISTER_PROPERTY(mageStateMachineConf, Mage::MageStateMachine, detectionRange);
        REGISTER_PROPERTY(mageStateMachineConf, Mage::MageStateMachine, bodyColliderSize);
        RegisterAccessorProperty(mageStateMachineConf, Mage::MageStateMachine, chaseState, moveSpeed);
        RegisterAccessorProperty(mageStateMachineConf, Mage::MageStateMachine, healState, healRange);
        RegisterAccessorProperty(mageStateMachineConf, Mage::MageStateMachine, healState, healTime);
        RegisterAccessorProperty(mageStateMachineConf, Mage::MageStateMachine, healState, healAmmount);
        REGISTER_PROPERTY(mageStateMachineConf, Mage::MageStateMachine, maxHealth);
        REGISTER_PROPERTY(mageStateMachineConf, Mage::MageStateMachine, logStateChanges);
        REGISTER_PROPERTY(mageStateMachineConf, Mage::MageStateMachine, healSfxPath);
        REGISTER_PROPERTY(mageStateMachineConf, Mage::MageStateMachine, healSfxVolume);
        REGISTER_PROPERTY(mageStateMachineConf, Mage::MageStateMachine, deathEffectPrefab);

        DEFAULT_CONFIG_AND_REQUIRED(
            mageStateMachineConf,
            Mage::MageStateMachine,
            Canis::Transform,
            Canis::Material,
            Canis::Model,
            Canis::Rigidbody,
            Canis::BoxCollider);

        mageStateMachineConf.DEFAULT_DRAW_INSPECTOR(Mage::MageStateMachine);

        _app.RegisterScript(mageStateMachineConf);
    }

    DEFAULT_UNREGISTER_SCRIPT(mageStateMachineConf, MageStateMachine)

    void MageStateMachine::Create()
    {
        entity.GetComponent<Canis::Transform>();
        healthComponent = entity.GetComponent<AICombat::Health>();

        Canis::AudioAssetHandle healSfxPath = { .path = "assets/audio/sfx/heal_1.ogg" };
        healSfxVolume = 0.1f;
        Canis::Rigidbody& rigidbody = entity.GetComponent<Canis::Rigidbody>();
        rigidbody.motionType = Canis::RigidbodyMotionType::KINEMATIC;
        rigidbody.useGravity = false;
        rigidbody.allowSleeping = false;
        rigidbody.linearVelocity = Canis::Vector3(0.0f);
        rigidbody.angularVelocity = Canis::Vector3(0.0f);

        entity.GetComponent<Canis::BoxCollider>().size = bodyColliderSize;

        if (entity.HasComponent<Canis::Material>())
        {
            m_baseColor = entity.GetComponent<Canis::Material>().color;
            m_hasBaseColor = true;
        }
    }

    void MageStateMachine::Ready()
    {
        if (entity.HasComponent<Canis::Material>())
        {
            m_baseColor = entity.GetComponent<Canis::Material>().color;
            m_hasBaseColor = true;
        }

        entity.GetComponent<AICombat::Health>().currentHealth = maxHealth;
        m_stateTime = 0.0f;
        m_useFirstHitSfx = true;

        ClearStates();
        AddState(idleState);
        AddState(chaseState);
        AddState(healState);

        ChangeState(IdleState::Name);
    }

    void MageStateMachine::Destroy()
    {
        SuperPupUtilities::StateMachine::Destroy();
    }

    void MageStateMachine::Update(float _dt)
    {
        if (!IsAlive())
            return;

        m_stateTime += _dt;
        SuperPupUtilities::StateMachine::Update(_dt);
    }

    Canis::Entity* MageStateMachine::FindLowestTarget() const
    {
        if (!entity.HasComponent<Canis::Transform>())
            return nullptr;

        const Canis::Transform& transform = entity.GetComponent<Canis::Transform>();
        const Canis::Vector3 origin = transform.GetGlobalPosition();
        Canis::Entity* lowestTarget = nullptr;
        int lowestHealth = 255;

        for (Canis::Entity* candidate : entity.scene.GetEntitiesWithTag("StateMachine"))
        {
            if (candidate == nullptr || candidate == &entity || !candidate->active) {
                continue;
            }

            if (candidate->GetComponent<AICombat::Team>().team != entity.GetComponent<AICombat::Team>().team) {
                continue;
            }

            if (!candidate->HasComponent<Canis::Transform>() || !candidate->HasComponent<AICombat::Health>()) {
                continue;
            }

            if (const AICombat::Health* other = candidate->GetScript<AICombat::Health>())
            {
                if (other->currentHealth <= 0) {
                    continue;
                }
            }

            const Canis::Vector3 candidatePosition = candidate->GetComponent<Canis::Transform>().GetGlobalPosition();
            const float distance = glm::distance(origin, candidatePosition);

            if (candidate->GetComponent<AICombat::Health>().currentHealth >= lowestHealth) {
                continue;
            }

            lowestHealth = candidate->GetComponent<AICombat::Health>().currentHealth;
            lowestTarget = candidate;
        }

        return lowestTarget;
    }

    float MageStateMachine::DistanceTo(const Canis::Entity& _other) const
    {
        if (!entity.HasComponent<Canis::Transform>() || !_other.HasComponent<Canis::Transform>())
            return std::numeric_limits<float>::max();

        const Canis::Vector3 selfPosition = entity.GetComponent<Canis::Transform>().GetGlobalPosition();
        const Canis::Vector3 targetPosition = _other.GetComponent<Canis::Transform>().GetGlobalPosition();
        return glm::distance(selfPosition, targetPosition);
    }

    void MageStateMachine::FaceTarget(const Canis::Entity& _target)
    {
        if (!entity.HasComponent<Canis::Transform>() || !_target.HasComponent<Canis::Transform>())
            return;

        Canis::Transform& transform = entity.GetComponent<Canis::Transform>();
        const Canis::Vector3 selfPosition = transform.GetGlobalPosition();
        Canis::Vector3 otherPosition = _target.GetComponent<Canis::Transform>().GetGlobalPosition();
        Canis::Vector3 direction = otherPosition - selfPosition;
        direction.y = 0.0f;

        if (glm::dot(direction, direction) <= 0.0001f)
            return;

        direction = glm::normalize(direction);
        transform.rotation.y = std::atan2(-direction.x, direction.z);
    }

    void MageStateMachine::MoveTowards(const Canis::Entity& _target, float _speed, float _dt)
    {
        if (!entity.HasComponent<Canis::Transform>() || !_target.HasComponent<Canis::Transform>())
            return;

        Canis::Transform& transform = entity.GetComponent<Canis::Transform>();
        const Canis::Vector3 selfPosition = transform.GetGlobalPosition();
        Canis::Vector3 otherPosition = _target.GetComponent<Canis::Transform>().GetGlobalPosition();
        otherPosition = otherPosition - _target.GetComponent<Canis::Transform>().GetForward() * 2.0f;
        Canis::Vector3 direction = otherPosition - selfPosition;
        direction.y = 0.0f;

        if (glm::dot(direction, direction) <= 0.0001f)
            return;

        direction = glm::normalize(direction);
        transform.position += direction * _speed * _dt;
    }

    void MageStateMachine::ChangeState(const std::string& _stateName)
    {
        if (SuperPupUtilities::StateMachine::GetCurrentStateName() == _stateName)
            return;

        if (!SuperPupUtilities::StateMachine::ChangeState(_stateName))
            return;

        m_stateTime = 0.0f;

        if (logStateChanges)
            Canis::Debug::Log("%s -> %s", entity.name.c_str(), _stateName.c_str());
    }

    const std::string& MageStateMachine::GetCurrentStateName() const
    {
        return SuperPupUtilities::StateMachine::GetCurrentStateName();
    }

    float MageStateMachine::GetStateTime() const
    {
        return m_stateTime;
    }

    float MageStateMachine::GetHealRange() const
    {
        return 2.0f;
    }

    int MageStateMachine::GetCurrentHealth() const
    {
        return entity.GetComponent<AICombat::Health>().currentHealth;
    }


    void MageStateMachine::PlayHealSFX()
    {
        /*
        const Canis::AudioAssetHandle& selectedSfx = m_useFirstHitSfx ? hitSfxPath1 : hitSfxPath2;
        m_useFirstHitSfx = !m_useFirstHitSfx;

        if (selectedSfx.Empty())
            return;
        */
        Canis::AudioManager::PlaySFX(healSfxPath, std::clamp(healSfxVolume, 0.0f, 1.0f));
    }

    void MageStateMachine::SpawnDeathEffect()
    {
        if (deathEffectPrefab.Empty() || !entity.HasComponent<Canis::Transform>())
            return;

        const Canis::Transform& sourceTransform = entity.GetComponent<Canis::Transform>();
        const Canis::Vector3 spawnPosition = sourceTransform.GetGlobalPosition();
        const Canis::Vector3 spawnRotation = sourceTransform.GetGlobalRotation();

        for (Canis::Entity* spawnedEntity : entity.scene.Instantiate(deathEffectPrefab))
        {
            if (spawnedEntity == nullptr || !spawnedEntity->HasComponent<Canis::Transform>())
                continue;

            Canis::Transform& spawnedTransform = spawnedEntity->GetComponent<Canis::Transform>();
            spawnedTransform.position = spawnPosition;
            spawnedTransform.rotation = spawnRotation;
        }
    }

    bool MageStateMachine::IsAlive() const
    {
        return entity.GetComponent<AICombat::Health>().currentHealth > 0;
    }

    void MageStateMachine::ReportHealth() {
    }

    void MageStateMachine::Heal(Entity* target) {
        if (target->GetComponent<AICombat::Health>().currentHealth < target->GetComponent<AICombat::Health>().maxHealth) {
            PlayHealSFX();
            target->GetComponent<AICombat::Health>().currentHealth += 2;
            if (target->GetComponent<AICombat::Health>().currentHealth > target->GetComponent<AICombat::Health>().maxHealth) {
                target->GetComponent<AICombat::Health>().currentHealth = target->GetComponent<AICombat::Health>().maxHealth;
            }
        }
    }
}