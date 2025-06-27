#include "EnemyManager.h"
#include <iostream>
#include <algorithm>
#include <cmath>



EnemyManager::EnemyManager(Map * map, int maxEnemyCount)
    : gameMap(map), maxEnemies(maxEnemyCount), currentEnemyCount(0)
{
    enemies.reserve(maxEnemies); // Reserve space for efficiency
}

EnemyManager::~EnemyManager()
{
    ClearAllEnemies();
}

void EnemyManager::Initialise()
{
    std::cout << "EnemyManager initialized" << '\n';
}


void EnemyManager::Update(float deltaTime, sf::Vector2f playerPosition)
{
    // Update all enemies
    for (auto& enemy : enemies) {
        if (enemy && enemy->isActive()) {
            enemy->Update(deltaTime, playerPosition);
        }
    }

    // Handle collisions between enemies
    HandleEnemyCollisions();

    // Remove dead enemies
    RemoveDeadEnemies();
}

void EnemyManager::Draw(sf::RenderWindow& window)
{
    for (auto& enemy : enemies) {
        if (enemy && enemy->isActive()) {
            enemy->Draw(window);
        }
    }
}

void EnemyManager::SpawnEnemy(EnemyType type, sf::Vector2f position)
{
    if (GetAliveEnemyCount() >= maxEnemies) {
        std::cout << "Cannot spawn enemy: max enemy limit reached (" << maxEnemies << ")" << '\n';
        return;
    }

    std::shared_ptr<Entity> newEnemy = nullptr;

    switch (type) {
    case EnemyType::POOKA: {
        auto pooka = std::make_shared<Pooka>(gameMap);
        pooka->Initialise();
        pooka->Load();
        newEnemy = pooka;
        std::cout << "Spawned Pooka at position (" << position.x << ", " << position.y << ")" << '\n';
        break;
    }
    case EnemyType::FYGAR: {
        
      // 
      // auto fygar = std::make_shared<Fygar>(gameMap);
      // fygar->Initialise();
      // fygar->Load();
      // newEnemy = fygar;
      // std::cout << "Spawned Fygar at position (" << position.x << ", " << position.y << ")" << '\n';
      //
        std::cout << "Fygar spawning not implemented yet!" << '\n';
        return;
    }
    default:
        std::cout << "Unknown enemy type!" << '\n';
        return;
    }

    if (newEnemy) {
        enemies.push_back(newEnemy);
        currentEnemyCount++;
    }
}

void EnemyManager::RemoveDeadEnemies()
{
    // Remove enemies that are no longer alive
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](const std::shared_ptr<Entity>& enemy) {
                return !enemy || !enemy->isActive();
            }),
        enemies.end()
    );
}

void EnemyManager::ClearAllEnemies()
{
    enemies.clear();
    currentEnemyCount = 0;
}

std::shared_ptr<Entity> EnemyManager::CheckCollisionWithPlayer(sf::Vector2f playerPosition, sf::Vector2f playerSize)
{
    sf::FloatRect playerBounds({ playerPosition.x - playerSize.x / 2.0f,
        playerPosition.y - playerSize.y / 2.0f },
        {playerSize.x, playerSize.y
});

    for (auto& enemy : enemies) {
        if (enemy && enemy->isActive()) {
            // Assuming enemies have a similar hitbox system
            // You might need to add a getBounds() method to Entity class
            sf::FloatRect enemyBounds({ playerPosition.x - 8.0f, // Half of enemy size
                playerPosition.y - 8.0f },
                {16.0f, 16.0f
        });

            if (playerBounds.findIntersection(enemyBounds)) {
                return enemy;
            }
        }
    }

    return nullptr; // No collision
}

void EnemyManager::HandleEnemyCollisions()
{
    // Handle collisions between enemies if needed
    for (size_t i = 0; i < enemies.size(); ++i) {
        for (size_t j = i + 1; j < enemies.size(); ++j) {
            if (enemies[i] && enemies[j] &&
                enemies[i]->isActive() && enemies[j]->isActive()) {

                // Call collision handlers
                enemies[i]->handleCollision(enemies[j]);
                enemies[j]->handleCollision(enemies[i]);
            }
        }
    }
}

int EnemyManager::GetAliveEnemyCount() const
{
    int count = 0;
    for (const auto& enemy : enemies) {
        if (enemy && enemy->isActive()) {
            count++;
        }
    }
    return count;
}