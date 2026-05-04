#include <Mage/MagicProjectileScript.hpp>

#include <Canis/App.hpp>
#include <Canis/Entity.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/Debug.hpp>

namespace Mage
{
    namespace
    {
        Canis::ScriptConf scriptConf = {};
    }

    void RegisterMagicProjectileScriptScript(Canis::App& _app)
    {
        // REGISTER_PROPERTY(scriptConf, Mage::MagicProjectileScript, exampleProperty);

        DEFAULT_CONFIG(scriptConf, Mage::MagicProjectileScript);

        scriptConf.DEFAULT_DRAW_INSPECTOR(Mage::MagicProjectileScript);

        _app.RegisterScript(scriptConf);
    }

    DEFAULT_UNREGISTER_SCRIPT(scriptConf, MagicProjectileScript)

    void MagicProjectileScript::Create() {
        entity.GetComponent<Canis::Transform>();
    }

    void MagicProjectileScript::Ready() {
        Canis::AudioManager::PlaySFX(projectilePath, std::clamp(1.0f, 0.0f, 1.0f));
    }

    void MagicProjectileScript::Destroy() {}

    void MagicProjectileScript::Update(float _dt) {
        if (!entity.HasComponent<Canis::Transform>() || !entity.active) {
            Canis::Debug::Log("No transform or smth");
        }
        Canis::Transform& transform = entity.GetComponent<Canis::Transform>();

        transform.position += transform.GetForward() * 5.0f * _dt;

        const Canis::Vector3 start = transform.GetGlobalPosition();
        const Canis::Vector3 end = transform.GetGlobalPosition() + transform.GetForward() * 2.0f;
        CollisionCheck(start, end);

    }

    void MagicProjectileScript::CollisionCheck(const Canis::Vector3& _start, const Canis::Vector3& _end)
    {
        const Canis::Vector3 travel = _end - _start;
        const float distance = glm::length(travel);
        float hitImpulse = 1.0f;
        if (distance <= 0.0001f)
            return;

        const Canis::Vector3 direction = travel / distance;
        Canis::RaycastHit hit = {};

        if (!entity.scene.Raycast(_start, direction, hit, distance))
            return;
        

        if (hit.entity == nullptr || hit.entity == &entity)
            return;

        if (hit.entity->HasComponent<Canis::Rigidbody>())
        {
            if (hit.entity->HasComponent<AICombat::Team>()) {
                if (hit.entity->GetComponent<AICombat::Team>().team == entity.GetComponent<AICombat::Team>().team) {
                    return;
                }
            }
            hit.entity->GetComponent<Canis::Rigidbody>().AddForce(
                direction * hitImpulse,
                Canis::Rigidbody3DForceMode::IMPULSE);
            if (hit.entity->HasComponent<AICombat::Health>()) {
                hit.entity->GetComponent<AICombat::Health>().currentHealth -= 10;
                PlayHitSfx();
                if (hit.entity->GetComponent<AICombat::Health>().currentHealth <= 0) {
                    SpawnDeathEffect(hit.entity);
                    hit.entity->Destroy();
                }
                entity.Destroy();
            }
        }

    }

    void MagicProjectileScript::SpawnDeathEffect(Entity* other)
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

    void MagicProjectileScript::PlayHitSfx() {
        Canis::AudioManager::PlaySFX(hitSfxPath1, std::clamp(0.1f, 0.0f, 1.0f));
    }
}
