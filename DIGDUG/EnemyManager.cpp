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

    if (currentState == States::GAME) {
        for (auto& enemy : enemies) {
            if (enemy && enemy->isActive()) {
                enemy->Update(deltaTime, playerPosition);
            }
        }
        for (auto& rock : rocks) {
            if (rock) {
                rock->Update(deltaTime, playerPosition);
            }
        }

        CheckCollisionWithPlayer(playerPosition, { 16, 16 });
        RemoveDeadEnemies();
        RemoveDestroyedRocks();
    }
}


void EnemyManager::Draw(sf::RenderWindow& window) {
    States currentState = gameState->getGameState();

    // Draw enemies and rocks during GAME, START, LOSS, WIN states
    if (currentState == States::GAME || currentState == States::START || currentState == States::LOSS || currentState == States::WIN || currentState == States::HIGHSCORE) {
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
    // Clear previously scored enemies when starting a new level
    if (player && player->getScoreManager()) {
        player->getScoreManager()->ClearScoredEnemies();
    }

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

    // First, handle scoring for dead enemies
    for (auto& enemy : enemies) {
        if (enemy && !enemy->isActive()) {
            // Determine kill method based on enemy state
            KillMethod killMethod = KillMethod::INFLATION; // Default

            // Check if it's a Pooka and determine kill method
            auto pooka = std::dynamic_pointer_cast<Pooka>(enemy);
            if (pooka) {
                if (pooka->getPumpState() >= 4) {
                    killMethod = KillMethod::INFLATION;
                }
                else {
                    killMethod = KillMethod::ROCK; // Likely killed by rock if not fully inflated
                }
            }
            // Award points through ScoreManager
            if (player && player->getScoreManager()) {
                player->getScoreManager()->OnEnemyKilled(enemy, killMethod);
            }
        }
    }

    // Then remove dead enemies
    auto removed = std::remove_if(enemies.begin(), enemies.end(),
        [](const std::shared_ptr<Entity>& enemy) {
            return !enemy || !enemy->isActive();
        });
    enemies.erase(removed, enemies.end());
    currentEnemyCount = static_cast<int>(enemies.size());

    if (enemies.size() != initialCount) {
        std::cout << "Removed " << (initialCount - enemies.size()) << " dead enemies. Current count: " << currentEnemyCount << std::endl;
    }
}


void EnemyManager::RemoveDestroyedRocks() {
    size_t initialCount = rocks.size();
    auto removed = std::remove_if(rocks.begin(), rocks.end(),
        [](const std::shared_ptr<Rock>& rock) {
            return !rock || rock->isMarkedForDeletion() ||
                (!rock->isActive() && rock->getDestroyAnimationComplete());
        });
    rocks.erase(removed, rocks.end());

    if (rocks.size() != initialCount) {
        std::cout << "Removed " << (initialCount - rocks.size()) << " destroyed rocks. Current count: " << rocks.size() << std::endl;
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

void EnemyManager::KillAllEnemiesAt(sf::Vector2f position, float radius) {
    sf::FloatRect killZone({ position.x - radius, position.y - radius }, { radius * 2, radius * 2 });

    for (auto& enemy : enemies) {
        if (enemy && enemy->isActive()) {
            sf::FloatRect enemyBounds = enemy->getBounds();
            if (killZone.findIntersection(enemyBounds)) {
                std::cout << "Enemy killed by external force at position (" << position.x << ", " << position.y << ")" << std::endl;

                // Kill the enemy (scoring will be handled in RemoveDeadEnemies)
                enemy->setActive(false);

                // Award points immediately for rock kills
                if (player && player->getScoreManager()) {
                    player->getScoreManager()->OnEnemyKilled(enemy, KillMethod::ROCK);
                }
            }
        }
    }
}