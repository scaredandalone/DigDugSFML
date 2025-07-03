#include "Rock.h"
#include "EnemyManager.h"
#include "Player.h"
#include "Map.h"
#include <iostream>
#include <cmath>

Rock::Rock(Map* gameMap, EnemyManager* em, Player* p, sf::Vector2f pos, sf::Vector2i tileTypeSourceGrid)
    : Entity(EntityType::ROCK, true, sf::Vector2i(TILE_SIZE, TILE_SIZE)),
    map(gameMap), enemyManager(em), player(p),
    initialTileTypeSource(tileTypeSourceGrid),
    isFalling(false), fallTimer(0.0f), hasFallen(false),
    destroyAnimationStarted(false), destroyAnimationComplete(false),
    tileSprite(tileTexture), rockSprite(rockTexture), tileTypeTextureIndex(-1),
    shakeTimer(0.0f), isShaking(false), destroyTimer(0.0f),
    markedForDeletion(false)
{
    setPosition(pos);
}

void Rock::Initialise() {
    Entity::Initialise();
    hitbox.setSize(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    hitbox.setOrigin(sf::Vector2f(TILE_SIZE / 2.0f, TILE_SIZE / 2.0f));
    hitbox.setPosition(getPosition());
}

void Rock::Load() {
    if (!tileTexture.loadFromFile("Assets/Map/tilesheet.png")) {
        std::cout << "Failed to load rock tilesheet texture" << '\n';
        return;
    }
    if (!rockTexture.loadFromFile("Assets/Map/rock.png")) {
        std::cout << "Failed to load rock overlay texture" << '\n';
        return;
    }
    tileSprite.setTexture(tileTexture);
    tileSprite.setOrigin(sf::Vector2f(TILE_SIZE / 2.0f, TILE_SIZE / 2.0f));
    tileSprite.setScale(sf::Vector2f(1, 1));
    if (tileTypeTextureIndex != -1) {
        tileSprite.setTextureRect(sf::IntRect({ tileTypeTextureIndex * TILE_SIZE, 0 }, { TILE_SIZE, TILE_SIZE }));
        std::cout << "Rock tile texture set to index: " << tileTypeTextureIndex << '\n';
    }
    else {
        tileSprite.setTextureRect(sf::IntRect({ 1 * TILE_SIZE, 0 }, { TILE_SIZE, TILE_SIZE }));
        std::cout << "Rock using fallback tile texture (index 1)" << '\n';
    }
    rockSprite.setTexture(rockTexture);
    rockSprite.setOrigin(sf::Vector2f(TILE_SIZE / 2.0f, TILE_SIZE / 2.0f));
    rockSprite.setScale(sf::Vector2f(1, 1));
    rockSprite.setTextureRect(sf::IntRect({ 0, 0 }, { TILE_SIZE, TILE_SIZE }));
    tileSprite.setPosition(getPosition());
    rockSprite.setPosition(getPosition());
    std::cout << "Rock loaded successfully with layered rendering at position ("
        << getPosition().x << ", " << getPosition().y << ")" << '\n';
}

void Rock::Update(float deltaTime, sf::Vector2f playerPosition) {
    // Handle destruction animation first, regardless of alive status
    if (!isAlive && destroyAnimationStarted && !destroyAnimationComplete) {
        destroyTimer += deltaTime;
        if (destroyTimer >= DESTROY_ANIMATION_DURATION) {
            destroyAnimationComplete = true;
            markedForDeletion = true;
            std::cout << "Rock marked for deletion after destruction animation completed at position ("
                << getPosition().x << ", " << getPosition().y << ")" << std::endl;
        }
    }

    // If not alive and destruction animation is handled, don't process falling logic
    if (!isAlive) {
        return;
    }

    sf::Vector2f currentRockCenter = getPosition();
    // Check the tile below the rock's bottom edge, not center
    float rockBottomY = currentRockCenter.y + TILE_SIZE / 2.0f;
    float checkYBelow = rockBottomY + 1.0f; // Check just below the rock's bottom edge
    int tileBelowType = map->getTileAt(currentRockCenter.x, checkYBelow);

    if (tileBelowType == 0 && !isFalling && !hasFallen) {
        if (!isShaking) {
            isShaking = true;
            fallTimer = 0.0f;
            int rockGridX = static_cast<int>(std::round(currentRockCenter.x / TILE_SIZE));
            int rockGridY = static_cast<int>(std::round(currentRockCenter.y / TILE_SIZE));
            map->setTileAt(rockGridX * TILE_SIZE + TILE_SIZE / 2.0f, rockGridY * TILE_SIZE + TILE_SIZE / 2.0f, 0);
            std::cout << "Tile underneath rock removed (shaking started)!" << std::endl;
        }
        fallTimer += deltaTime;
        float shakeOffset = std::sin(shakeTimer * SHAKE_SPEED_MULTIPLIER) * SHAKE_AMPLITUDE;
        tileSprite.setPosition(sf::Vector2f(getPosition().x + shakeOffset, getPosition().y));
        rockSprite.setPosition(sf::Vector2f(getPosition().x + shakeOffset, getPosition().y));
        if (fallTimer >= FALL_DELAY) {
            isFalling = true;
            isShaking = false;
            std::cout << "Rock started falling!" << std::endl;
        }
    }
    else if (tileBelowType > 0) { // Changed condition to check for solid tiles (> 0)
        if (isFalling) {
            isFalling = false;
            hasFallen = true;
            std::cout << "Rock hit solid ground!" << std::endl;
            // Mark as dead and start destroy animation immediately
            isAlive = false;
            startDestroyAnimation();
        }
        fallTimer = 0.0f;
        isShaking = false;
        tileSprite.setPosition(getPosition());
        rockSprite.setPosition(getPosition());
    }
    else {
        isShaking = false;
        tileSprite.setPosition(getPosition());
        rockSprite.setPosition(getPosition());
    }

    if (isShaking) {
        shakeTimer += deltaTime;
    }

    if (isFalling) {
        updateFalling(deltaTime);
        checkAndSquashEntities();
    }
}
void Rock::Draw(sf::RenderWindow& window) {
    if (isAlive || (destroyAnimationStarted && !destroyAnimationComplete)) {
        if (!isShaking && !isFalling && !destroyAnimationStarted) {
            window.draw(tileSprite);
        }
        window.draw(rockSprite);
        window.draw(hitbox);
    }
}

void Rock::updateFalling(float deltaTime) {
    sf::Vector2f currentPos = getPosition();
    sf::Vector2f nextPosCandidate = sf::Vector2f(currentPos.x, currentPos.y + FALL_SPEED * deltaTime);

    // Check if the rock's bottom edge will hit a solid tile
    float rockBottomY = nextPosCandidate.y + TILE_SIZE / 2.0f;
    int tileBelowType = map->getTileAt(nextPosCandidate.x, rockBottomY);

    if (tileBelowType == 0 || tileBelowType == -1) {
        // No solid tile below, continue falling
        setPosition(nextPosCandidate);
    }
    else {
        // Hit a solid tile, snap to position where rock's bottom sits on tile's top
        float tileGridY = std::floor(rockBottomY / TILE_SIZE);
        float tileTopY = tileGridY * TILE_SIZE;
        float snappedRockCenterY = tileTopY - TILE_SIZE / 2.0f; // Position rock center so bottom sits on tile top
        sf::Vector2f snapPos(currentPos.x, snappedRockCenterY);
        setPosition(snapPos);
        isFalling = false;
        hasFallen = true;
        isAlive = false; // Mark as dead when it hits the ground
        std::cout << "Rock landed with bottom on top of solid tile at y=" << snappedRockCenterY << ". Tile type: " << tileBelowType << std::endl;
        startDestroyAnimation();
    }
}

void Rock::checkAndSquashEntities() {
    if (!isFalling) {
        return; // Only check for collisions when the rock is falling
    }

    sf::FloatRect rockBounds = hitbox.getGlobalBounds();
    bool hasSquashedSomething = false;

    if (player && player->isActive()) {
        sf::FloatRect playerBounds = player->getBounds();
        if (rockBounds.findIntersection(playerBounds)) {
            std::cout << "Rock squashed player!" << std::endl;
            player->setHealth(0);
            if (player->getLives() > 0) {
                player->setLives(player->getLives() - 1);
            }
            hasSquashedSomething = true;
        }
    }
    if (enemyManager) {
        const auto& enemies = enemyManager->GetEnemies();
        for (auto& enemy : enemies) {
            if (enemy && enemy->isActive()) {
                sf::FloatRect enemyBounds = enemy->getBounds();
                if (rockBounds.findIntersection(enemyBounds)) {
                    std::cout << "Rock squashed an enemy!" << std::endl;
                    enemy->setActive(false);
                    hasSquashedSomething = true;
                }
            }
        }
    }
    if (hasSquashedSomething) {
        isAlive = false;
        startDestroyAnimation();
    }
}

void Rock::startDestroyAnimation() {
    if (!destroyAnimationStarted) {
        destroyAnimationStarted = true;
        tileSprite.setColor(sf::Color(255, 255, 255, 128));
        rockSprite.setColor(sf::Color(255, 255, 255, 128));
        std::cout << "Rock destruction started" << std::endl;
        destroyTimer = 0.0f;
    }
}

bool Rock::isSolid(float x, float y) {
    int tileType = map->getTileAt(x, y);
    return tileType > 0;
}

void Rock::setPosition(sf::Vector2f pos) {
    Entity::setPosition(pos);
    tileSprite.setPosition(pos);
    rockSprite.setPosition(pos);
    hitbox.setPosition(pos);
}