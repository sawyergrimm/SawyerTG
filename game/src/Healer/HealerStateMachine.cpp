#include <Healer/HealerStateMachine.hpp>

#include <Canis/App.hpp>
#include <Canis/AudioManager.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/Debug.hpp>
#include <AICombat/Health.hpp>

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
            if (healerStatMachine->FindClosestTarget() != nullptr)
                healerStatMachine->ChangeState(ChaseState::Name);
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

        Canis::Entity* target = healerStatMachine->FindClosestTarget();

        if (target == nullptr)
        {
            healerStatMachine->ChangeState(IdleState::Name);
            return;
        }

        healerStatMachine->FaceTarget(*target);

        if (healerStatMachine->DistanceTo(*target) <= healerStatMachine->GetHealRange())
        {
            healerStatMachine->ChangeState(HealState::Name);
            return;
        }

        healerStatMachine->MoveTowards(*target, moveSpeed, _dt);
    }

    HealState::HealState(SuperPupUtilities::StateMachine& _stateMachine) :
        State(Name, _stateMachine) {}

    void HealState::Enter()
    {
        //if (HealerStateMachine* healerStatMachine = dynamic_cast<HealerStateMachine*>(m_stateMachine))
            //healerStatMachine->SetHammerSwing(0.0f);
    }

    void HealState::Update(float)
    {
        HealerStateMachine* healerStatMachine = dynamic_cast<HealerStateMachine*>(m_stateMachine);
        if (healerStatMachine == nullptr)
            return;

        if (Canis::Entity* target = healerStatMachine->FindClosestTarget())
            healerStatMachine->FaceTarget(*target);

        const float duration = std::max(healTime, 0.001f);

        if (healerStatMachine->GetStateTime() < duration)
            return;

        if (healerStatMachine->FindClosestTarget() != nullptr)
            healerStatMachine->ChangeState(ChaseState::Name);
        else
            healerStatMachine->ChangeState(IdleState::Name);
    }

    void HealState::Exit()
    {
    }

    HealerStateMachine::HealerStateMachine(Canis::Entity& _entity) :
        SuperPupUtilities::StateMachine(_entity),
        idleState(*this),
        chaseState(*this),
        healState(*this) {}

    void RegisterHealerStateMachineScript(Canis::App& _app)
    {
        REGISTER_PROPERTY(healerStateMachineConf, Healer::HealerStateMachine, targetTag);
        REGISTER_PROPERTY(healerStateMachineConf, Healer::HealerStateMachine, detectionRange);
        REGISTER_PROPERTY(healerStateMachineConf, Healer::HealerStateMachine, bodyColliderSize);
        RegisterAccessorProperty(healerStateMachineConf, Healer::HealerStateMachine, chaseState, moveSpeed);
        RegisterAccessorProperty(healerStateMachineConf, Healer::HealerStateMachine, healState, healRange);
        RegisterAccessorProperty(healerStateMachineConf, Healer::HealerStateMachine, healState, healTime);
        RegisterAccessorProperty(healerStateMachineConf, Healer::HealerStateMachine, healState, healAmmount);
        REGISTER_PROPERTY(healerStateMachineConf, Healer::HealerStateMachine, maxHealth);
        REGISTER_PROPERTY(healerStateMachineConf, Healer::HealerStateMachine, logStateChanges);
        REGISTER_PROPERTY(healerStateMachineConf, Healer::HealerStateMachine, hitSfxPath1);
        REGISTER_PROPERTY(healerStateMachineConf, Healer::HealerStateMachine, hitSfxPath2);
        REGISTER_PROPERTY(healerStateMachineConf, Healer::HealerStateMachine, hitSfxVolume);
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

        healthComponent.currentHealth = std::max(maxHealth, 1);
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

    Canis::Entity* HealerStateMachine::FindClosestTarget() const
    {
        if (targetTag.empty() || !entity.HasComponent<Canis::Transform>())
            return nullptr;

        const Canis::Transform& transform = entity.GetComponent<Canis::Transform>();
        const Canis::Vector3 origin = transform.GetGlobalPosition();
        Canis::Entity* closestTarget = nullptr;
        float closestDistance = detectionRange;
        float lowestHealth = 255.0f;

        for (Canis::Entity* candidate : entity.scene.GetEntitiesWithTag(targetTag))
        {
            if (candidate == nullptr || candidate == &entity || !candidate->active)
                continue;

            if (!candidate->HasComponent<Canis::Transform>())
                continue;

            if (const HealerStateMachine* other = candidate->GetScript<HealerStateMachine>())
            {
                if (!other->IsAlive())
                    continue;
            }

            const Canis::Vector3 candidatePosition = candidate->GetComponent<Canis::Transform>().GetGlobalPosition();
            const float distance = glm::distance(origin, candidatePosition);

            if (distance > detectionRange || distance >= closestDistance)
                continue;

            closestDistance = distance;
            closestTarget = candidate;
        }

        return closestTarget;
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
        Canis::Vector3 direction = _target.GetComponent<Canis::Transform>().GetGlobalPosition() - selfPosition;
        direction.y = 0.0f;

        if (glm::dot(direction, direction) <= 0.0001f)
            return;

        direction = glm::normalize(direction);
        transform.rotation.y = std::atan2(-direction.x, -direction.z);
    }

    void HealerStateMachine::MoveTowards(const Canis::Entity& _target, float _speed, float _dt)
    {
        if (!entity.HasComponent<Canis::Transform>() || !_target.HasComponent<Canis::Transform>())
            return;

        Canis::Transform& transform = entity.GetComponent<Canis::Transform>();
        const Canis::Vector3 selfPosition = transform.GetGlobalPosition();
        Canis::Vector3 direction = _target.GetComponent<Canis::Transform>().GetGlobalPosition() - selfPosition;
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
        return healthComponent.currentHealth;
    }


    void HealerStateMachine::TakeDamage(int _damage)
    {
        if (!IsAlive())
            return;

        const int damageToApply = std::max(_damage, 0);
        if (damageToApply <= 0)
            return;

        healthComponent.currentHealth = std::max(0, healthComponent.currentHealth - damageToApply);
        PlayHitSfx();

        if (m_hasBaseColor && entity.HasComponent<Canis::Material>())
        {
            Canis::Material& material = entity.GetComponent<Canis::Material>();
            const float healthRatio = (maxHealth > 0)
                ? (static_cast<float>(healthComponent.currentHealth) / static_cast<float>(maxHealth))
                : 0.0f;

            material.color = Canis::Vector4(
                m_baseColor.x * (0.5f + (0.5f * healthRatio)),
                m_baseColor.y * (0.5f + (0.5f * healthRatio)),
                m_baseColor.z * (0.5f + (0.5f * healthRatio)),
                m_baseColor.w);
        }

        if (healthComponent.currentHealth > 0)
            return;

        if (logStateChanges)
            Canis::Debug::Log("%s was defeated.", entity.name.c_str());

        SpawnDeathEffect();
        entity.Destroy();
    }

    void HealerStateMachine::PlayHitSfx()
    {
        const Canis::AudioAssetHandle& selectedSfx = m_useFirstHitSfx ? hitSfxPath1 : hitSfxPath2;
        m_useFirstHitSfx = !m_useFirstHitSfx;

        if (selectedSfx.Empty())
            return;

        Canis::AudioManager::PlaySFX(selectedSfx, std::clamp(hitSfxVolume, 0.0f, 1.0f));
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
        return healthComponent.currentHealth > 0;
    }
}
