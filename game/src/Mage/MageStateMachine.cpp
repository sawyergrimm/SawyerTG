#include <Mage/MageStateMachine.hpp>

#include <Canis/App.hpp>
#include <Canis/AudioManager.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/Debug.hpp>
#include <AICombat/Health.hpp>
#include <AICombat/Team.hpp>
#include <SuperPupUtilities/SimpleObjectPool.hpp>
#include <SuperPupUtilities/Bullet.hpp>
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
            if (mageStatMachine->FindClosestTarget() != nullptr)
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

        Canis::Entity* target = mageStatMachine->FindClosestTarget();

        if (target == nullptr)
        {
            mageStatMachine->ChangeState(IdleState::Name);
            return;
        }

        mageStatMachine->FaceTarget(*target);
        Canis::Vector3 targetPosition = target->GetComponent<Canis::Transform>().position;
        if (glm::length(mageStatMachine->entity.GetComponent<Canis::Transform>().position - targetPosition) < 8.0f)
        {
            mageStatMachine->ChangeState(FireState::Name);
            return;
        }

        mageStatMachine->MoveTowards(*target, moveSpeed, _dt);
    }

    FireState::FireState(SuperPupUtilities::StateMachine& _stateMachine) :
        State(Name, _stateMachine) {}

    void FireState::Enter()
    {
        MageStateMachine* mageStatMachine = dynamic_cast<MageStateMachine*>(m_stateMachine);
        mageStatMachine->countdown = 0.5f;
        //if (MageStateMachine* mageStatMachine = dynamic_cast<MageStateMachine*>(m_stateMachine))
            //mageStatMachine->SetHammerSwing(0.0f);
    }

    void FireState::Update(float _dt)
    {
        MageStateMachine* mageStatMachine = dynamic_cast<MageStateMachine*>(m_stateMachine);
        if (mageStatMachine == nullptr)
            return;

        if (Canis::Entity* target = mageStatMachine->FindClosestTarget())
            mageStatMachine->FaceTarget(*target);
        else {return;}


        if (mageStatMachine->countdown > 0.0f) {
            mageStatMachine->countdown -= _dt;
        }
        else {
            Canis::SceneAssetHandle projectile = { .path = "assets/prefabs/magic_projectile.scene" };
            mageStatMachine->AltFire(projectile);
            mageStatMachine->countdown = 0.5f;
            mageStatMachine->ChangeState(IdleState::Name);
        }

    }

    void FireState::Exit()
    {
    }

    MageStateMachine::MageStateMachine(Canis::Entity& _entity) :
        SuperPupUtilities::StateMachine(_entity),
        idleState(*this),
        chaseState(*this),
        fireState(*this) {}

    void RegisterMageStateMachineScript(Canis::App& _app)
    {
        REGISTER_PROPERTY(mageStateMachineConf, Mage::MageStateMachine, detectionRange);
        REGISTER_PROPERTY(mageStateMachineConf, Mage::MageStateMachine, bodyColliderSize);
        REGISTER_PROPERTY(mageStateMachineConf, Mage::MageStateMachine, projectile);
        RegisterAccessorProperty(mageStateMachineConf, Mage::MageStateMachine, chaseState, moveSpeed);
        RegisterAccessorProperty(mageStateMachineConf, Mage::MageStateMachine, fireState, healRange);
        RegisterAccessorProperty(mageStateMachineConf, Mage::MageStateMachine, fireState, healTime);
        RegisterAccessorProperty(mageStateMachineConf, Mage::MageStateMachine, fireState, healAmmount);
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
        AddState(fireState);

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
        transform.rotation.y = std::atan2(-direction.x, -direction.z);
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

    void MageStateMachine::AltFire(SceneAssetHandle projectile) {
        std::vector<Entity*> projectileObject = entity.scene.Instantiate(projectile);
        projectileObject[0]->GetComponent<Canis::Transform>().position = entity.GetComponent<Canis::Transform>().position;
        projectileObject[0]->GetComponent<Canis::Transform>().rotation = entity.GetComponent<Canis::Transform>().rotation;
        projectileObject[0]->GetComponent<AICombat::Team>().team = entity.GetComponent<AICombat::Team>().team;
        return;
    }

    void MageStateMachine::Fire(const Canis::Vector3& _position, const Canis::Vector3& _direction)
    {
        const Canis::Vector3 flatDirection = glm::normalize(Canis::Vector3(_direction.x, 0.0f, _direction.z));
        const float yaw = std::atan2(-flatDirection.x, -flatDirection.z);
        const Canis::Vector3 rotation = Canis::Vector3(0.0f, yaw, 0.0f);

        auto* pool = SuperPupUtilities::SimpleObjectPool::Instance;

        if (pool == nullptr) {
            Canis::Debug::Log("Failpoint 1");
            return;
        }

        Canis::Entity* projectile = pool->Spawn("magic_projectile", _position, rotation);

        if (projectile == nullptr) {
            Canis::Debug::Log("Failpoint 2");
            return;
        }

        if (SuperPupUtilities::Bullet* bullet = projectile->GetScript<SuperPupUtilities::Bullet>())
        {
            bullet->speed = projectileSpeed*10.0f;
            bullet->lifeTime = projectileLifeTime;
            bullet->hitImpulse = projectileHitImpulse;
            bullet->Launch();
            Canis::Debug::Log("Bullet should be launched.");
        } else {
            Canis::Debug::Log("Failpoint 3");
        }
    }

    Canis::Entity* MageStateMachine::FindClosestTarget() const
    {
        if (!entity.HasComponent<Canis::Transform>())
            return nullptr;

        const Canis::Transform& transform = entity.GetComponent<Canis::Transform>();
        const Canis::Vector3 origin = transform.GetGlobalPosition();
        Canis::Entity* closestTarget = nullptr;
        float closestDistance = detectionRange;
        float lowestHealth = 255.0f;

        for (Canis::Entity* candidate : entity.scene.GetEntitiesWithTag("StateMachine"))
        {
            if (candidate == nullptr || candidate == &entity || !candidate->active || !candidate->HasComponent<AICombat::Health>()) {
                continue;
            }


            if (!candidate->HasComponent<Canis::Transform>()) {
                continue;
            }

            if (candidate->GetComponent<AICombat::Team>().team == entity.GetComponent<AICombat::Team>().team) {
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

            if (distance > detectionRange || distance >= closestDistance) {
                continue;
            }

            closestDistance = distance;
            closestTarget = candidate;
        }

        return closestTarget;
    }
}