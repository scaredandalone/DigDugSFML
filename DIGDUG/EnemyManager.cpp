#include "EnemyManager.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include "Player.h"
#include "Pooka.h"
#include "GameState.h"

EnemyManager::EnemyManager(Map* map, Player* player, int maxEnemyCount)
    : gameMap(map), player(player), maxEnemies(maxEnemyCount), currentEnemyCount(0) {
    enemies.reserve(maxEnemies); 
}

EnemyManager::~EnemyManager() {
    ClearAllEnemies();
}

void EnemyManager::Initialise() {
    std::cout << "EnemyManager initialized" << '\n';
}

void EnemyManager::Update(float deltaTime, sf::Vector2f playerPosition) {
    // Update all enemie
    for (auto& enemy : enemies) {
        if (enemy && enemy->isActive()) {
            enemy->Update(deltaTime, playerPosition);
        }
    }

    // Handle collisions between enemies
    CheckCollisionWithPlayer(playerPosition, {16,16});

    // Remove dead enemies
    RemoveDeadEnemies();
}

void EnemyManager::Draw(sf::RenderWindow& window) {
    States currentState = gameState->getGameState();
    if (currentState == States::GAME) {
        for (auto& enemy : enemies) {
            if (enemy && enemy->isActive()) {
                enemy->Draw(window);
            }
        }
    }
}

void EnemyManager::SpawnEnemy(EnemyType type, sf::Vector2f position) {
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
        pooka->setPosition(position); // 
        newEnemy = pooka;
        std::cout << "Spawned Pooka at position (" << position.x << ", " << position.y << ")" << '\n';
        break;
    }
    case EnemyType::FYGAR: {
        // Placeholder for Fygar spawning
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

void EnemyManager::RemoveDeadEnemies() {
    size_t initialCount = enemies.size();
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

    enemies.erase(removedCount, enemies.end());
    currentEnemyCount = static_cast<int>(enemies.size());

    if (enemies.size() != initialCount) {
        std::cout << "Removed " << (initialCount - enemies.size()) << " dead enemies. Current count: " << currentEnemyCount << std::endl;
    }
}

void EnemyManager::ClearAllEnemies() {
    enemies.clear();
    currentEnemyCount = 0;
}

std::shared_ptr<Entity> EnemyManager::CheckCollisionWithPlayer(sf::Vector2f playerPosition, sf::Vector2f playerSize) {
    sf::FloatRect playerBounds;
    playerBounds.position = sf::Vector2f(
        playerPosition.x - playerSize.x / 2.0f,
        playerPosition.y - playerSize.y / 2.0f
    );
    playerBounds.size = playerSize;

    for (auto& enemy : enemies) {
        if (enemy && enemy->isActive()) {
            sf::FloatRect enemyBounds = enemy->getBounds();
            if (playerBounds.findIntersection(enemyBounds)) {
                std::cout << "enemy collided with player" << '\n';
                return enemy;
            }
        }
    }

    return nullptr; // No collision
}

void EnemyManager::HandleEnemyCollisions() {
    // Placeholder for enemy-enemy collision logic
}

int EnemyManager::GetAliveEnemyCount() const {
    int count = 0;
    for (const auto& enemy : enemies) {
        if (enemy && enemy->isActive()) {
            count++;
        }
    }
    return count;
}