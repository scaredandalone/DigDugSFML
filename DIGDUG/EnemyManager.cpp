#include "EnemyManager.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include "Player.h"
#include "Pooka.h"
#include "Rock.h"
#include "GameState.h"

EnemyManager::EnemyManager(Map* map, Player* player, int maxEnemyCount)
    : gameMap(map), player(player), maxEnemies(maxEnemyCount), currentEnemyCount(0) {
    enemies.reserve(maxEnemies);
    rocks.reserve(10);
}

EnemyManager::~EnemyManager() {
    ClearAllEnemies();
    ClearAllRocks();
}

void EnemyManager::Initialise() {
    std::cout << "EnemyManager initialized" << '\n';
}

void EnemyManager::Update(float deltaTime, sf::Vector2f playerPosition) {
    States currentState = gameState->getGameState();

    // Only update enemies and rocks during GAME state
    if (currentState == States::GAME) {
        for (auto& enemy : enemies) {
            if (enemy && enemy->isActive()) {
                enemy->Update(deltaTime, playerPosition);
            }
        }

        // Update ALL rocks, not just active ones
        // Rocks need to update even when !isActive() to handle destroy animation
        for (auto& rock : rocks) {
            if (rock) {  // Only check if rock exists, not if it's active
                rock->Update(deltaTime, playerPosition);
            }
        }

        CheckCollisionWithPlayer(playerPosition, { 16,16 });
        RemoveDeadEnemies();
        RemoveDestroyedRocks();
    }
    // During START, WIN, and LOSS states, entities remain stationary but are still drawn
}

void EnemyManager::Draw(sf::RenderWindow& window) {
    States currentState = gameState->getGameState();

    // Draw enemies and rocks during GAME, START, and LOSS states CHANGE THIS IF YOU WANT TO HAVE IT NOT DRAW STUFF DURING A GAMESTATE
    if (currentState == States::GAME || currentState == States::START || currentState == States::LOSS) {
        for (auto& enemy : enemies) {
            if (enemy && enemy->isActive()) {
                enemy->Draw(window);
            }
        }
        for (auto& rock : rocks) {
            if (rock && rock->isActive()) {
                rock->Draw(window);
            }
            else if (rock && !rock->isActive() && !rock->getDestroyAnimationComplete() && !rock->isMarkedForDeletion()) {
                rock->Draw(window);
            }
        }
    }
}

void EnemyManager::SpawnEnemiesFromMap() {
    const auto& spawns = gameMap->getEntitySpawns();
    for (const auto& spawn : spawns) {
        char spawnType = spawn.first;
        sf::Vector2f position = spawn.second;
        if (spawnType == 'P') {
            SpawnEnemy(EnemyType::POOKA, position);
        }
    }
}

void EnemyManager::SpawnRocksFromMap() {
    const auto& rockSpawns = gameMap->getRockSpawns();
    for (const auto& rockSpawn : rockSpawns) {
        SpawnRock(rockSpawn.position, rockSpawn.textureIndex);
    }
}

void EnemyManager::SpawnRock(sf::Vector2f position, int textureIndex) {
    auto rock = std::make_shared<Rock>(gameMap, this, player, position, sf::Vector2i(0, 0));
    rock->setTextureIndex(textureIndex);
    rock->Initialise();
    rock->Load();
    rock->setPosition(position);
    rocks.push_back(rock);
    std::cout << "Spawned Rock at position (" << position.x << ", " << position.y << ") with texture index " << textureIndex << '\n';
}

void EnemyManager::SpawnEnemy(EnemyType type, sf::Vector2f position) {
    if (GetEnemyCount() >= maxEnemies) {
        std::cout << "Cannot spawn enemy: max enemy limit reached (" << maxEnemies << ")" << '\n';
        return;
    }
    std::shared_ptr<Entity> newEnemy = nullptr;
    switch (type) {
    case EnemyType::POOKA: {
        auto pooka = std::make_shared<Pooka>(gameMap, player);
        pooka->Initialise();
        pooka->Load();
        pooka->setPosition(position);
        newEnemy = pooka;
        std::cout << "Spawned Pooka at position (" << position.x << ", " << position.y << ")" << '\n';
        break;
    }
    case EnemyType::FYGAR: {
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

void EnemyManager::RemoveDestroyedRocks() {
    size_t initialCount = rocks.size();
    auto removedCount = std::remove_if(rocks.begin(), rocks.end(),
        [](const std::shared_ptr<Rock>& rock) {
            if (!rock) {
                std::cout << "Removing null rock" << std::endl;
                return true;
            }
            // Remove rocks that are marked for deletion OR have completed their destroy animation
            if (rock->isMarkedForDeletion() || (!rock->isActive() && rock->getDestroyAnimationComplete())) {
                std::cout << "Removing destroyed rock after animation completed" << std::endl;
                return true;
            }
            return false;
        });
    rocks.erase(removedCount, rocks.end());
    if (rocks.size() != initialCount) {
        std::cout << "Removed " << (initialCount - rocks.size()) << " destroyed rocks. Current count: " << rocks.size() << std::endl;
    }
}

void EnemyManager::RemoveRock(std::shared_ptr<Rock> rock) {
    if (!rock) {
        std::cout << "Attempted to remove null rock" << std::endl;
        return;
    }
    size_t initialCount = rocks.size();
    auto removedCount = std::remove_if(rocks.begin(), rocks.end(),
        [&rock](const std::shared_ptr<Rock>& r) {
            if (r == rock) {
                std::cout << "Removing specific rock at position (" << r->getPosition().x << ", " << r->getPosition().y << ")" << std::endl;
                return true;
            }
            return false;
        });
    rocks.erase(removedCount, rocks.end());
    if (rocks.size() != initialCount) {
        std::cout << "Removed 1 rock. Current count: " << rocks.size() << std::endl;
    }
    else {
        std::cout << "Failed to remove rock: not found in rocks vector" << std::endl;
    }
}

void EnemyManager::ClearAllEnemies() {
    enemies.clear();
    currentEnemyCount = 0;
}

void EnemyManager::ClearAllRocks() {
    rocks.clear();
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
                std::cout << "Enemy collided with player" << '\n';
                HandleEnemyCollisions(enemy);
                return enemy;
            }
        }
    }
    return nullptr;
}

void EnemyManager::HandleEnemyCollisions(std::shared_ptr<Entity> collidedEnemy) {
    if (collidedEnemy && collidedEnemy->getInflationStatus() == false) {
        player->setHealth(0);
        std::cout << "Player killed by enemy collision!" << std::endl;
    }
}

void EnemyManager::KillEnemy(std::shared_ptr<Entity> enemy) {
    if (enemy) {
        enemy->setActive(false);
        std::cout << "Enemy killed!" << std::endl;
    }
}

void EnemyManager::KillAllEnemiesAt(sf::Vector2f position, float radius) {
    sf::FloatRect killZone({ position.x - radius, position.y - radius }, { radius * 2, radius * 2 });
    for (auto& enemy : enemies) {
        if (enemy && enemy->isActive()) {
            sf::FloatRect enemyBounds = enemy->getBounds();
            if (killZone.findIntersection(enemyBounds)) {
                enemy->setActive(false);
                std::cout << "Enemy killed by external force at position (" << position.x << ", " << position.y << ")" << std::endl;
            }
        }
    }
}