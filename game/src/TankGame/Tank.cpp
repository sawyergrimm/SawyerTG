#include <TankGame/Tank.hpp>
#include <TankGame/Bullet.hpp>

#include <Canis/App.hpp>
#include <Canis/Time.hpp>
#include <Canis/Math.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Window.hpp>
#include <Canis/Editor.hpp>
#include <Canis/InputManager.hpp>
#include <Canis/AssetManager.hpp>

#include <Canis/ConfigHelper.hpp>

using namespace Canis;

namespace TankGame
{
    ScriptConf tankConf = {};

    void RegisterTankScript(Canis::App &_app)
    {
        REGISTER_PROPERTY(tankConf, TankGame::Tank, speed);
        REGISTER_PROPERTY(tankConf, TankGame::Tank, turnSpeed);
        REGISTER_PROPERTY(tankConf, TankGame::Tank, coolDownTime);

        DEFAULT_CONFIG_AND_REQUIRED(tankConf, TankGame::Tank, Canis::RectTransform, Canis::Sprite2D);

        tankConf.DEFAULT_DRAW_INSPECTOR(TankGame::Tank);

        _app.RegisterScript(tankConf);
    }

    DEFAULT_UNREGISTER_SCRIPT(tankConf, Tank)

    void Tank::Create() { }

    void Tank::Ready() {
        m_transform = entity.HasComponent<RectTransform>() ? &entity.GetComponent<RectTransform>() : nullptr;
        m_turret = nullptr;
        m_firePoint = nullptr;

        if (m_transform != nullptr && !m_transform->children.empty())
        {
            Entity* turretEntity = m_transform->children[0];
            m_turret = (turretEntity != nullptr && turretEntity->HasComponent<RectTransform>())
                ? &turretEntity->GetComponent<RectTransform>()
                : nullptr;
            if (m_turret != nullptr && !m_turret->children.empty())
            {
                Entity* firePointEntity = m_turret->children[0];
                m_firePoint = (firePointEntity != nullptr && firePointEntity->HasComponent<RectTransform>())
                    ? &firePointEntity->GetComponent<RectTransform>()
                    : nullptr;
            }
        }
    }

    void Tank::Destroy() { }

    void Tank::Update(float _dt) {
        m_transform = entity.HasComponent<RectTransform>() ? &entity.GetComponent<RectTransform>() : nullptr;
        if (m_transform == nullptr)
            return;

        m_turret = nullptr;
        m_firePoint = nullptr;

        if (!m_transform->children.empty())
        {
            Entity* turretEntity = m_transform->children[0];
            m_turret = (turretEntity != nullptr && turretEntity->HasComponent<RectTransform>())
                ? &turretEntity->GetComponent<RectTransform>()
                : nullptr;

            if (m_turret != nullptr && !m_turret->children.empty())
            {
                Entity* firePointEntity = m_turret->children[0];
                m_firePoint = (firePointEntity != nullptr && firePointEntity->HasComponent<RectTransform>())
                    ? &firePointEntity->GetComponent<RectTransform>()
                    : nullptr;
            }
        }

        Movement(_dt);
        
        Turret(_dt);

        UpdateGun(_dt);
    }

    void Tank::Movement(float _dt) {
        if (m_transform == nullptr)
            return;

        // movement
        if (entity.scene.GetInputManager().GetKey(Canis::Key::W))
            m_transform->Move(m_transform->GetRight() * speed * _dt);
        
        if (entity.scene.GetInputManager().GetKey(Canis::Key::S))
            m_transform->Move(-m_transform->GetRight() * speed * _dt);
        
        // turn
        if (entity.scene.GetInputManager().GetKey(Canis::Key::A))
            m_transform->rotation += turnSpeed * Canis::DEG2RAD * _dt;
        
        if (entity.scene.GetInputManager().GetKey(Canis::Key::D))
            m_transform->rotation += -turnSpeed * Canis::DEG2RAD * _dt;
    }

    void Tank::Turret(float _dt) {
        if (m_transform == nullptr || m_turret == nullptr)
            return;

        // turret
        Vector2 screenSize = Vector2(entity.scene.GetWindow().GetScreenWidth(), entity.scene.GetWindow().GetScreenHeight());
        // mouse screen space to world space
        Vector2 target = entity.scene.GetInputManager().mouse - screenSize * 0.5f;
        
        Vector2 direction = target - m_turret->GetPosition();
        float angleRadians = atan2(direction.y, direction.x);
        float angleDegrees = angleRadians * Canis::RAD2DEG;

        m_turret->rotation = angleRadians - m_transform->GetRotation();
    }

    void Tank::UpdateGun(float _dt) {
        if (m_firePoint == nullptr)
            return;
        
        m_time -= _dt;

        if (m_time > 0.0f)
            return;

        if (entity.scene.GetInputManager().GetLeftClick())
        {
            m_time = coolDownTime;

            Canis::Entity* bulletEntity = entity.scene.CreateEntity("Bullet");
            RectTransform& bulletTransform = *bulletEntity->AddComponent<RectTransform>();
            Sprite2D& bulletSprite = *bulletEntity->AddComponent<Sprite2D>();
            bulletSprite.textureHandle = AssetManager::GetTextureHandle("assets/textures/arrow_decorative_n.png");

            Bullet* bullet = bulletEntity->AddScript<Bullet>();
            bulletTransform.SetPosition(m_firePoint->GetPosition());
            bulletTransform.rotation = -(DEG2RAD*90.0f) + m_firePoint->GetRotation();

            bullet->speed = 550.0f;
            bullet->lifeTime = 5.0f;
        }
    }
}
