#pragma once
#include "Entity.h"
#include "Map.h"
#include "ScoreManager.h"
#include <vector>
#include <memory>

class Player;
class Rock;
class GameState;

enum class EnemyType {
    POOKA,
    FYGAR
};

class EnemyManager {
private:
    Map* gameMap;
    Player* player;
    std::vector<std::shared_ptr<Entity>> enemies;
    std::vector<std::shared_ptr<Rock>> rocks;
    int maxEnemies;
    int currentEnemyCount;
    GameState* gameState;

    void RemoveDeadEnemies();
    void RemoveDestroyedRocks();
    std::shared_ptr<Entity> CheckCollisionWithPlayer(sf::Vector2f playerPosition, sf::Vector2f playerSize);
    void HandleEnemyCollisions(std::shared_ptr<Entity> collidedEnemy);

public:
    EnemyManager(Map* map, Player* player, int maxEnemyCount);
    ~EnemyManager();

    void Initialise();
    void Update(float deltaTime, sf::Vector2f playerPosition);
    void Draw(sf::RenderWindow& window);

    void SpawnEnemiesFromMap();
    void SpawnRocksFromMap();
    void SpawnEnemy(EnemyType type, sf::Vector2f position);
    void SpawnRock(sf::Vector2f position, int textureIndex);

    void ClearAllEnemies();
    void ClearAllRocks();
    void KillAllEnemiesAt(sf::Vector2f position, float radius);


    const std::vector<std::shared_ptr<Entity>>& GetEnemies() const { return enemies; }
    const std::vector<std::shared_ptr<Rock>>& GetRocks() const { return rocks; }
    int GetEnemyCount() const { return currentEnemyCount; }
    void SetGameState(GameState* gs) { gameState = gs; }
};