#include <SuperPupUtilities/SimpleObjectPool.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/Debug.hpp>

#include <algorithm>
#include <string>

namespace SuperPupUtilities
{
    SimpleObjectPool* SimpleObjectPool::s_instance = nullptr;

    SimpleObjectPool* SimpleObjectPool::GetInstance()
    {
        return s_instance;
    }

    void SimpleObjectPool::Create()
    {
        if (s_instance == nullptr)
        {
            s_instance = this;
            return;
        }

        if (s_instance != this)
            Canis::Debug::Warning("SimpleObjectPool: multiple pool instances found. Only the first instance will be used.");
    }

    void SimpleObjectPool::Ready()
    {
        if (s_instance != this)
            return;

        BuildPools();
    }

    void SimpleObjectPool::Destroy()
    {
        if (s_instance == this)
            s_instance = nullptr;

        m_poolDictionary.clear();
        m_initialized = false;
    }

    void SimpleObjectPool::Update(float _dt)
    {
        
    }

    void SimpleObjectPool::ApplySpawnTransform(
        Canis::Entity& _entity,
        const Canis::Vector3& _position,
        const Canis::Vector3& _rotation) const
    {
        if (_entity.HasComponent<Canis::Transform>())
        {
            Canis::Transform& transform = _entity.GetComponent<Canis::Transform>();
            transform.position = _position;
            transform.rotation = _rotation;
            return;
        }

        if (_entity.HasComponent<Canis::RectTransform>())
        {
            Canis::RectTransform& rectTransform = _entity.GetComponent<Canis::RectTransform>();
            rectTransform.SetPosition(Canis::Vector2(_position.x, _position.y));
            rectTransform.rotation = _rotation.z;
        }
    }

    void SimpleObjectPool::ForEachEntityInHierarchy(
        Canis::Entity& _root,
        const std::function<void(Canis::Entity&)>& _func) const
    {
        _func(_root);

        if (_root.HasComponent<Canis::Transform>())
        {
            for (Canis::Entity* child : _root.GetComponent<Canis::Transform>().children)
            {
                if (child != nullptr)
                    ForEachEntityInHierarchy(*child, _func);
            }
        }
        else if (_root.HasComponent<Canis::RectTransform>())
        {
            for (Canis::Entity* child : _root.GetComponent<Canis::RectTransform>().children)
            {
                if (child != nullptr)
                    ForEachEntityInHierarchy(*child, _func);
            }
        }
    }

    void SimpleObjectPool::ResetHierarchyPhysics(Canis::Entity& _root) const
    {
        ForEachEntityInHierarchy(_root, [](Canis::Entity& _entity)
        {
            if (!_entity.HasComponent<Canis::Rigidbody>())
                return;

            Canis::Rigidbody& rigidbody = _entity.GetComponent<Canis::Rigidbody>();
            rigidbody.linearVelocity = Canis::Vector3(0.0f);
            rigidbody.angularVelocity = Canis::Vector3(0.0f);
            rigidbody.pendingForce = Canis::Vector3(0.0f);
            rigidbody.pendingAcceleration = Canis::Vector3(0.0f);
            rigidbody.pendingImpulse = Canis::Vector3(0.0f);
            rigidbody.pendingVelocityChange = Canis::Vector3(0.0f);
        });
    }

    void SimpleObjectPool::SetHierarchyActive(Canis::Entity& _root, bool _active) const
    {
        ForEachEntityInHierarchy(_root, [_active](Canis::Entity& _entity)
        {
            _entity.active = _active;
        });
    }

    void SimpleObjectPool::ForceReadyHierarchy(Canis::Entity& _root) const
    {
        ForEachEntityInHierarchy(_root, [](Canis::Entity& _entity)
        {
            _entity.scene.ForceReady(_entity);
        });
    }

    void SimpleObjectPool::BuildPools()
    {
        if (m_initialized)
            return;

        m_poolDictionary.clear();

        for (const Pool& pool : pools)
        {
            if (pool.code.empty())
            {
                if (logWarnings)
                    Canis::Debug::Warning("SimpleObjectPool: skipped a pool with an empty code.");
                continue;
            }

            std::queue<Canis::Entity*>& objectPool = m_poolDictionary[pool.code];
            const int poolSize = std::max(pool.size, 1);

            for (int i = 0; i < poolSize; ++i)
            {
                Canis::Entity* pooledEntity = CreatePooledEntity(pool);
                if (pooledEntity != nullptr)
                    objectPool.push(pooledEntity);
            }
        }

        m_initialized = true;
    }

    Canis::Entity* SimpleObjectPool::CreatePooledEntity(const Pool& _pool)
    {
        if (_pool.prefab.Empty())
        {
            if (logWarnings)
                Canis::Debug::Warning("SimpleObjectPool: pool '%s' is missing a prefab.", _pool.code.c_str());
            return nullptr;
        }

        std::vector<Canis::Entity*> roots = entity.scene.Instantiate(_pool.prefab);
        if (roots.empty())
        {
            if (logWarnings)
                Canis::Debug::Warning("SimpleObjectPool: pool '%s' could not instantiate its prefab.", _pool.code.c_str());
            return nullptr;
        }

        if (roots.size() > 1 && logWarnings)
        {
            Canis::Debug::Warning(
                "SimpleObjectPool: pool '%s' prefab spawned multiple roots. Only the first root will be pooled.",
                _pool.code.c_str());
        }

        Canis::Entity* pooledEntity = roots.front();
        if (pooledEntity == nullptr)
            return nullptr;

        ForceReadyHierarchy(*pooledEntity);
        ResetHierarchyPhysics(*pooledEntity);
        SetHierarchyActive(*pooledEntity, false);
        return pooledEntity;
    }

    Canis::Entity* SimpleObjectPool::SpawnFromPool(
        const std::string& _code,
        const Canis::Vector3& _position,
        const Canis::Vector3& _rotation)
    {
        if (s_instance != this)
            return nullptr;

        if (!m_initialized)
            BuildPools();

        auto poolIt = m_poolDictionary.find(_code);
        if (poolIt == m_poolDictionary.end() || poolIt->second.empty())
        {
            if (logWarnings)
                Canis::Debug::Warning("SimpleObjectPool: pool '%s' does not exist or is empty.", _code.c_str());
            return nullptr;
        }

        Canis::Entity* objectToSpawn = poolIt->second.front();
        poolIt->second.pop();
        poolIt->second.push(objectToSpawn);

        if (objectToSpawn == nullptr)
        {
            if (logWarnings)
                Canis::Debug::Warning("SimpleObjectPool: pool '%s' contained a null entity.", _code.c_str());
            return nullptr;
        }

        ApplySpawnTransform(*objectToSpawn, _position, _rotation);
        ResetHierarchyPhysics(*objectToSpawn);
        SetHierarchyActive(*objectToSpawn, true);
        return objectToSpawn;
    }

    namespace
    {
        YAML::Node EncodePool(const SimpleObjectPool::Pool& _pool)
        {
            YAML::Node node;
            node["code"] = _pool.code;
            node["prefab"] = _pool.prefab;
            node["size"] = _pool.size;
            return node;
        }

        SimpleObjectPool::Pool DecodePool(const YAML::Node& _node)
        {
            SimpleObjectPool::Pool pool = {};
            if (!_node)
                return pool;

            pool.code = _node["code"].as<std::string>(pool.code);
            pool.prefab = _node["prefab"].as<Canis::SceneAssetHandle>(pool.prefab);
            pool.size = _node["size"].as<int>(pool.size);
            return pool;
        }
    }
    
    Canis::ScriptConf simpleObjectPoolConf = {};

    void RegisterSimpleObjectPoolScript(Canis::App& _app)
    {
        DEFAULT_CONFIG(simpleObjectPoolConf, SuperPupUtilities::SimpleObjectPool);

        simpleObjectPoolConf.Encode = [](YAML::Node& _node, Canis::Entity& _entity) -> void
        {
            SimpleObjectPool* pool = _entity.GetScript<SimpleObjectPool>();
            if (pool == nullptr)
                return;

            YAML::Node component;
            component["logWarnings"] = pool->logWarnings;

            YAML::Node poolsNode(YAML::NodeType::Sequence);
            for (const SimpleObjectPool::Pool& config : pool->pools)
                poolsNode.push_back(EncodePool(config));

            component["pools"] = poolsNode;
            _node[SimpleObjectPool::ScriptName] = component;
        };

        simpleObjectPoolConf.Decode = [](YAML::Node& _node, Canis::Entity& _entity, bool _callCreate) -> void
        {
            YAML::Node component = _node[SimpleObjectPool::ScriptName];
            if (!component)
                return;

            SimpleObjectPool* pool = _entity.GetScript<SimpleObjectPool>();
            if (pool == nullptr)
                pool = _entity.AddScript<SimpleObjectPool>(_callCreate);

            if (pool == nullptr)
                return;

            pool->logWarnings = component["logWarnings"].as<bool>(pool->logWarnings);
            pool->pools.clear();

            YAML::Node poolsNode = component["pools"];
            if (!poolsNode || !poolsNode.IsSequence())
                return;

            for (const YAML::Node& poolNode : poolsNode)
                pool->pools.push_back(DecodePool(poolNode));
        };

        simpleObjectPoolConf.DrawInspector = [](Canis::Editor& _editor, Canis::Entity& _entity, const Canis::ScriptConf& _conf) -> void
        {
            SimpleObjectPool* component = _entity.GetScript<SimpleObjectPool>();
            if (component == nullptr)
                return;

            DrawInspectorField(_editor, "logWarnings", _conf.name.c_str(), component->logWarnings);

            if (ImGui::Button(("Add Pool##" + _conf.name).c_str()))
                component->pools.push_back(SimpleObjectPool::Pool{});

            for (size_t i = 0; i < component->pools.size(); ++i)
            {
                SimpleObjectPool::Pool& pool = component->pools[i];
                const std::string indexId = _conf.name + std::to_string(i);

                ImGui::Separator();
                ImGui::Text("Pool %zu", i + 1);
                DrawInspectorField(_editor, "code", indexId.c_str(), pool.code);
                DrawInspectorField(_editor, "prefab", indexId.c_str(), pool.prefab);
                DrawInspectorField(_editor, "size", indexId.c_str(), pool.size);

                if (ImGui::Button(("Remove##pool" + indexId).c_str()))
                {
                    component->pools.erase(component->pools.begin() + i);
                    --i;
                }
            }
        };

        _app.RegisterScript(simpleObjectPoolConf);
    }

    DEFAULT_UNREGISTER_SCRIPT(simpleObjectPoolConf, SimpleObjectPool)
}
