#pragma once

#include <Canis/Entity.hpp>

#include <cstdint>
#include <deque>
#include <future>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class VoxelTerrainChunk;

class GenerateTerrain : public Canis::ScriptableEntity
{
private:
    struct PendingChunkJob
    {
        std::uint64_t chunkKey = 0;
        int chunkX = 0;
        int chunkZ = 0;
        std::future<std::vector<unsigned char>> future = {};
    };

    bool m_generated = false;
    Canis::Entity* m_playerEntity = nullptr;
    int m_lastCenterChunkX = std::numeric_limits<int>::max();
    int m_lastCenterChunkZ = std::numeric_limits<int>::max();
    int m_rockMaterialId = -1;
    int m_iceMaterialId = -1;
    int m_goldMaterialId = -1;
    int m_uraniumMaterialId = -1;
    std::unordered_map<std::uint64_t, int> m_loadedChunkEntities = {};
    std::unordered_set<std::uint64_t> m_desiredChunkKeys = {};
    std::unordered_set<std::uint64_t> m_queuedChunkKeys = {};
    std::unordered_set<std::uint64_t> m_pendingChunkKeys = {};
    std::deque<std::pair<int, int>> m_queuedChunkRequests = {};
    std::vector<PendingChunkJob> m_pendingChunkJobs = {};

public:
    static constexpr const char* ScriptName = "GenerateTerrain";

    int seed = 1337;
    int chunksX = 4;
    int chunksZ = 4;
    int chunkSize = 16;
    int chunkHeight = 34;
    int baseHeight = 6;
    int maxHeightVariation = 13;
    int hillHeightBoost = 14;
    int bedrockLayerHeight = 1;
    int surfaceIceHeight = 13;
    float heightNoiseScale = 0.06f;
    float detailNoiseScale = 0.14f;
    float hillNoiseScale = 0.022f;
    float caveNoiseScale = 0.12f;

    Canis::SceneAssetHandle rockDropPrefab = {};
    Canis::SceneAssetHandle iceDropPrefab = {};
    Canis::SceneAssetHandle goldDropPrefab = {};
    Canis::SceneAssetHandle uraniumDropPrefab = {};

    GenerateTerrain(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

    void Create();
    void Ready();
    void Destroy();
    void Update(float _dt);

private:
    void CacheMaterials();
    void EnsurePlayerEntity();
    Canis::Vector3 GetStreamingFocusPosition() const;
    void RefreshLoadedChunks(bool _forceRefresh, bool _loadMissingAsync);
    void QueueChunkRequest(int _chunkX, int _chunkZ);
    void LaunchQueuedChunkJobs();
    void FinalizePendingChunkJobs();
    void WaitForPendingChunkJobs();
    Canis::Entity* CreateChunkEntity(int _chunkX, int _chunkZ, std::vector<unsigned char>&& _blocks);
    std::vector<unsigned char> GenerateChunkBlocks(int _chunkX, int _chunkZ) const;
};

extern void RegisterGenerateTerrainScript(Canis::App& _app);
extern void UnRegisterGenerateTerrainScript(Canis::App& _app);
