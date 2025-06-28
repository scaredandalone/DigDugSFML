#pragma once
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include "Entity.h"
#include "Pooka.h"
#include "Map.h"

class Player;

enum class EnemyType {
    POOKA,
    FYGAR
};

class EnemyManager
{
private:
    std::vector<std::shared_ptr<Entity>> enemies;
    Map* gameMap;
    Player* player;


    int maxEnemies;
    int currentEnemyCount;

public:
    EnemyManager(Map* map, Player* player, int maxEnemyCount = 10);
    ~EnemyManager();

    void Initialise();
    void Update(float deltaTime, sf::Vector2f playerPosition);
    void Draw(sf::RenderWindow& window);

    void SpawnEnemy(EnemyType type, sf::Vector2f position);
    void RemoveDeadEnemies();
    void ClearAllEnemies();


    std::shared_ptr<Entity> CheckCollisionWithPlayer(sf::Vector2f playerPosition, sf::Vector2f playerSize);
    void HandleEnemyCollisions();

    int GetEnemyCount() const { return enemies.size(); }
    int GetAliveEnemyCount() const;
    const std::vector<std::shared_ptr<Entity>>& GetEnemies() const { return enemies; }

    void SetMaxEnemies(int max) { maxEnemies = max; }
};