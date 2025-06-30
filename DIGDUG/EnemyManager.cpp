#include "EnemyManager.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include "Player.h"

EnemyManager::EnemyManager(Map* map, Player* player, int maxEnemyCount)
    : gameMap(map), player(player), maxEnemies(maxEnemyCount), currentEnemyCount(0)
{
    enemies.reserve(maxEnemies); // Reserve space
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
        auto pooka = std::make_shared<Pooka>(gameMap, player);
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
    // Count enemies before removal for debugging
    size_t initialCount = enemies.size();

    // Remove enemies that are no longer alive
    auto removedCount = std::remove_if(enemies.begin(), enemies.end(),
        [](const std::shared_ptr<Entity>& enemy) {
            if (!enemy) {
                std::cout << "Removing null enemy" << std::endl;
                return true;
            }
            if (!enemy->isActive()) {
                std::cout << "Removing dead enemy" << std::endl;
                return true;
            }
            return false;
        });

    // Actually erase the removed enemies
    enemies.erase(removedCount, enemies.end());

    // Update current count
    currentEnemyCount = static_cast<int>(enemies.size());

    // Debug output if enemies were removed
    if (enemies.size() != initialCount) {
        std::cout << "Removed " << (initialCount - enemies.size()) << " dead enemies. Current count: " << currentEnemyCount << std::endl;
    }
}

void EnemyManager::ClearAllEnemies()
{
    enemies.clear();
    currentEnemyCount = 0;
}

std::shared_ptr<Entity> EnemyManager::CheckCollisionWithPlayer(sf::Vector2f playerPosition, sf::Vector2f playerSize)
{
    sf::FloatRect playerBounds;
    playerBounds.position = sf::Vector2f(
        playerPosition.x - playerSize.x / 2.0f,
        playerPosition.y - playerSize.y / 2.0f
    );
    playerBounds.size = playerSize;

    for (auto& enemy : enemies) {
        if (enemy && enemy->isActive()) {
            sf::FloatRect enemyBounds = enemy->getBounds();

          // // Debug output to check bounds
          // std::cout << "Player bounds: (" << playerBounds.position.x << ", " << playerBounds.position.y
          //     << ") size (" << playerBounds.size.x << ", " << playerBounds.size.y << ")" << std::endl;
          // std::cout << "Enemy bounds: (" << enemyBounds.position.x << ", " << enemyBounds.position.y
          //     << ") size (" << enemyBounds.size.x << ", " << enemyBounds.size.y << ")" << std::endl;

            if (playerBounds.findIntersection(enemyBounds)) {
                std::cout << "enemy collided with player" << '\n';
                return enemy;
            }
        }
    }

    return nullptr; // No collision
}

void EnemyManager::HandleEnemyCollisions()
{
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