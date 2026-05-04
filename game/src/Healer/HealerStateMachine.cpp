#include <Healer/HealerStateMachine.hpp>

#include <Canis/App.hpp>
#include <Canis/AudioManager.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/Debug.hpp>
#include <AICombat/Health.hpp>
#include <AICombat/Team.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace Healer
{
    namespace
    {
        ScriptConf healerStateMachineConf = {};
    }

    IdleState::IdleState(SuperPupUtilities::StateMachine& _stateMachine) :
        State(Name, _stateMachine) {}

    void IdleState::Enter()
    {
        //if (HealerStateMachine* healerStatMachine = dynamic_cast<HealerStateMachine*>(m_stateMachine))
            //healerStatMachine->ResetHammerPose();
    }

    void IdleState::Update(float)
    {
        if (HealerStateMachine* healerStatMachine = dynamic_cast<HealerStateMachine*>(m_stateMachine))
        {
            healerStatMachine->ReportHealth();
            if (healerStatMachine->myTarget = healerStatMachine->FindLowestTarget()){
                healerStatMachine->myTarget->GetComponent<AICombat::Health>().beingHealed = true;
                healerStatMachine->ChangeState(ChaseState::Name);
            }
        }
    }

    ChaseState::ChaseState(SuperPupUtilities::StateMachine& _stateMachine) :
        State(Name, _stateMachine) {}

    void ChaseState::Enter()
    {
        //if (HealerStateMachine* healerStatMachine = dynamic_cast<HealerStateMachine*>(m_stateMachine))
            //healerStatMachine->ResetHammerPose();
    }

    void ChaseState::Update(float _dt)
    {
        HealerStateMachine* healerStatMachine = dynamic_cast<HealerStateMachine*>(m_stateMachine);
        if (healerStatMachine == nullptr)
            return;

        Canis::Entity* target = healerStatMachine->myTarget;

        if (target == nullptr)
        {
            healerStatMachine->ChangeState(IdleState::Name);
            return;
        }

        healerStatMachine->FaceTarget(*target);
        Canis::Vector3 targetPosition = target->GetComponent<Canis::Transform>().position;
        if (glm::length(healerStatMachine->entity.GetComponent<Canis::Transform>().position - (targetPosition - target->GetComponent<Canis::Transform>().GetForward() * 2.0f)) < 0.5f)
        {
            Canis::Debug::Log("IM TRYNA CHAGE");
            healerStatMachine->ChangeState(HealState::Name);
            return;
        }

        healerStatMachine->MoveTowards(*target, moveSpeed, _dt);
    }

    HealState::HealState(SuperPupUtilities::StateMachine& _stateMachine) :
        State(Name, _stateMachine) {}

    void HealState::Enter()
    {
        HealerStateMachine* healerStatMachine = dynamic_cast<HealerStateMachine*>(m_stateMachine);
        healerStatMachine->countdown = 2.0f;
        //if (HealerStateMachine* healerStatMachine = dynamic_cast<HealerStateMachine*>(m_stateMachine))
            //healerStatMachine->SetHammerSwing(0.0f);
    }

    void HealState::Update(float _dt)
    {
        HealerStateMachine* healerStatMachine = dynamic_cast<HealerStateMachine*>(m_stateMachine);
        if (healerStatMachine == nullptr)
            return;

        if (Canis::Entity* target = healerStatMachine->myTarget)
            healerStatMachine->FaceTarget(*target);
        else {return;}


        if (healerStatMachine->countdown > 0.0f) {
            healerStatMachine->countdown -= _dt;
            if (Canis::Entity* target = healerStatMachine->myTarget) {
                if (target->GetComponent<AICombat::Health>().currentHealth < target->GetComponent<AICombat::Health>().maxHealth) {
                    healerStatMachine->entity.GetComponent<Canis::PointLight>().intensity = 10.0f * (2.0f - healerStatMachine->countdown / 2) - 8.0f;
                }
                if (healerStatMachine->DistanceTo(*target) > 3.0f) {
                    healerStatMachine->entity.GetComponent<Canis::PointLight>().intensity = 0.0f;
                    healerStatMachine->ChangeState(ChaseState::Name);
                }
            }
        }
        else {
            healerStatMachine->Heal(healerStatMachine->myTarget);
            healerStatMachine->countdown = 2.0f;
            healerStatMachine->entity.GetComponent<Canis::PointLight>().intensity = 0.0f;
            healerStatMachine->ChangeState(IdleState::Name);
        }

    }

    void HealState::Exit()
    {
        HealerStateMachine* healerStatMachine = dynamic_cast<HealerStateMachine*>(m_stateMachine);
        healerStatMachine->myTarget->GetComponent<AICombat::Health>().beingHealed = false;

    }

    HealerStateMachine::HealerStateMachine(Canis::Entity& _entity) :
        SuperPupUtilities::StateMachine(_entity),
        idleState(*this),
        chaseState(*this),
        healState(*this) {}

    void RegisterHealerStateMachineScript(Canis::App& _app)
    {
        REGISTER_PROPERTY(healerStateMachineConf, Healer::HealerStateMachine, detectionRange);
        REGISTER_PROPERTY(healerStateMachineConf, Healer::HealerStateMachine, bodyColliderSize);
        RegisterAccessorProperty(healerStateMachineConf, Healer::HealerStateMachine, chaseState, moveSpeed);
        RegisterAccessorProperty(healerStateMachineConf, Healer::HealerStateMachine, healState, healRange);
        RegisterAccessorProperty(healerStateMachineConf, Healer::HealerStateMachine, healState, healTime);
        RegisterAccessorProperty(healerStateMachineConf, Healer::HealerStateMachine, healState, healAmmount);
        REGISTER_PROPERTY(healerStateMachineConf, Healer::HealerStateMachine, maxHealth);
        REGISTER_PROPERTY(healerStateMachineConf, Healer::HealerStateMachine, logStateChanges);
        REGISTER_PROPERTY(healerStateMachineConf, Healer::HealerStateMachine, healSfxPath);
        REGISTER_PROPERTY(healerStateMachineConf, Healer::HealerStateMachine, healSfxVolume);
        REGISTER_PROPERTY(healerStateMachineConf, Healer::HealerStateMachine, deathEffectPrefab);

        DEFAULT_CONFIG_AND_REQUIRED(
            healerStateMachineConf,
            Healer::HealerStateMachine,
            Canis::Transform,
            Canis::Material,
            Canis::Model,
            Canis::Rigidbody,
            Canis::BoxCollider);

        healerStateMachineConf.DEFAULT_DRAW_INSPECTOR(Healer::HealerStateMachine);

        _app.RegisterScript(healerStateMachineConf);
    }

    DEFAULT_UNREGISTER_SCRIPT(healerStateMachineConf, HealerStateMachine)

    void HealerStateMachine::Create()
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

    void HealerStateMachine::Ready()
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

    void HealerStateMachine::Destroy()
    {
        SuperPupUtilities::StateMachine::Destroy();
    }

    void HealerStateMachine::Update(float _dt)
    {
        if (!IsAlive())
            return;

        m_stateTime += _dt;
        SuperPupUtilities::StateMachine::Update(_dt);
    }

    Canis::Entity* HealerStateMachine::FindLowestTarget() const
    {
        if (!entity.HasComponent<Canis::Transform>())
            return nullptr;

        const Canis::Transform& transform = entity.GetComponent<Canis::Transform>();
        const Canis::Vector3 origin = transform.GetGlobalPosition();
        Canis::Entity* lowestTarget = nullptr;
        int lowestHealth = 255;

        for (Canis::Entity* candidate : entity.scene.GetEntitiesWithTag("StateMachine"))
        {
            if (candidate->GetComponent<AICombat::Health>().beingHealed == true) {
                continue;
            }
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
                if (other->currentHealth <= 0 || other->beingHealed) {
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

    float HealerStateMachine::DistanceTo(const Canis::Entity& _other) const
    {
        if (!entity.HasComponent<Canis::Transform>() || !_other.HasComponent<Canis::Transform>())
            return std::numeric_limits<float>::max();

        const Canis::Vector3 selfPosition = entity.GetComponent<Canis::Transform>().GetGlobalPosition();
        const Canis::Vector3 targetPosition = _other.GetComponent<Canis::Transform>().GetGlobalPosition();
        return glm::distance(selfPosition, targetPosition);
    }

    void HealerStateMachine::FaceTarget(const Canis::Entity& _target)
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

    void HealerStateMachine::MoveTowards(const Canis::Entity& _target, float _speed, float _dt)
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

    void HealerStateMachine::ChangeState(const std::string& _stateName)
    {
        if (SuperPupUtilities::StateMachine::GetCurrentStateName() == _stateName)
            return;

        if (!SuperPupUtilities::StateMachine::ChangeState(_stateName))
            return;

        m_stateTime = 0.0f;

        if (logStateChanges)
            Canis::Debug::Log("%s -> %s", entity.name.c_str(), _stateName.c_str());
    }

    const std::string& HealerStateMachine::GetCurrentStateName() const
    {
        return SuperPupUtilities::StateMachine::GetCurrentStateName();
    }

    float HealerStateMachine::GetStateTime() const
    {
        return m_stateTime;
    }

    float HealerStateMachine::GetHealRange() const
    {
        return 2.0f;
    }

    int HealerStateMachine::GetCurrentHealth() const
    {
        return entity.GetComponent<AICombat::Health>().currentHealth;
    }


    void HealerStateMachine::PlayHealSFX()
    {
        /*
        const Canis::AudioAssetHandle& selectedSfx = m_useFirstHitSfx ? hitSfxPath1 : hitSfxPath2;
        m_useFirstHitSfx = !m_useFirstHitSfx;

        if (selectedSfx.Empty())
            return;
        */
        Canis::AudioManager::PlaySFX(healSfxPath, std::clamp(healSfxVolume, 0.0f, 1.0f));
    }

    void HealerStateMachine::SpawnDeathEffect()
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

    bool HealerStateMachine::IsAlive() const
    {
        return entity.GetComponent<AICombat::Health>().currentHealth > 0;
    }

    void HealerStateMachine::ReportHealth() {
    }

    void HealerStateMachine::Heal(Entity* target) {
        if (target->GetComponent<AICombat::Health>().currentHealth < target->GetComponent<AICombat::Health>().maxHealth) {
            PlayHealSFX();
            target->GetComponent<AICombat::Health>().currentHealth += 2;
            if (target->GetComponent<AICombat::Health>().currentHealth > target->GetComponent<AICombat::Health>().maxHealth) {
                target->GetComponent<AICombat::Health>().currentHealth = target->GetComponent<AICombat::Health>().maxHealth;
            }
        }
    }
}