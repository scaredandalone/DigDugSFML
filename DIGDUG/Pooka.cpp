#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include "Pooka.h"
#include "Player.h"

Pooka::Pooka(Map* gameMap, Player* player, EnemyManager* em): Entity(EntityType::POOKA, true, sf::Vector2i(16, 16), 300),
    health(4), speed(25.0f), status(0), sprite(texture),
    pumpSound(pumpBuffer), map(gameMap), player(player), enemyManager(em) {
}

void Pooka::Initialise() {
    Entity::Initialise();
}

void Pooka::Load() {
    if (!texture.loadFromFile("Assets/Sprites/Pooka/spritesheet.png")) {
        std::cout << "failed to load pooka sprite" << '\n';
    }
    sprite.setTexture(texture);
    sprite.setTextureRect(sf::IntRect({ 0, 0 }, { size.x, size.y }));
    sprite.setOrigin(sf::Vector2f(size.x / 2.0f, size.y / 2.0f));
    sprite.setScale(sf::Vector2f(1, 1));
    sprite.setPosition({ 0, 0 });

    if (!pumpBuffer.loadFromFile("Assets/Sounds/SFX/pump.mp3")) {
        std::cout << "failed to load pump sound" << '\n';
    }
    pumpSound.setBuffer(pumpBuffer);

    std::cout << "pooka loaded successfully" << '\n';
    animation = std::make_unique<Animation>(&texture, sf::Vector2u(2, 2), 0.25f, size.x, size.y, true);
}

void Pooka::Update(float deltaTime, sf::Vector2f playerPosition) {
    if (health <= 0 || !isAlive) return;

    updatePumpState(deltaTime);
    updatePumpCooldown(deltaTime);

    if (canMove()) {
        updateMovement(deltaTime, playerPosition);
    }
}

void Pooka::updatePumpState(float deltaTime) {
    if (!harpoonStuck) return;

    pumpTimer += deltaTime;
    if (pumpTimer >= PUMP_DURATION) {
        handleDeflation();
        pumpTimer = 0.0f;
        updateInflationSprite();
    }
}

void Pooka::handleDeflation() {
    if (pumpState > 0) {
        pumpState = std::max(0, pumpState - 1);
        std::cout << "Pooka deflated to state: " << pumpState << std::endl;

        if (pumpState == 0) {
            regenerateHealth();
            DetachHarpoon();
        }
    }
    else {
        DetachHarpoon();
    }
}

void Pooka::regenerateHealth() {
    const int MAX_HEALTH = 4;
    if (health < MAX_HEALTH) {
        health += 1;
        std::cout << "Pooka health regen to: " << health << std::endl;
    }
}

void Pooka::updatePumpCooldown(float deltaTime) {
    if (pumpCooldownTimer > 0.0f) {
        pumpCooldownTimer -= deltaTime;
    }
}

bool Pooka::canMove() const {
    return health > 0 && isAlive && pumpState == 0 && !harpoonStuck;
}

void Pooka::updateMovement(float deltaTime, sf::Vector2f playerPosition) {
    sf::Vector2f currentPosition = sprite.getPosition();
    movementTimer += deltaTime;
    stuckTimer += deltaTime;

    if (!isMoving) {
        updateMovementTimer(deltaTime, playerPosition);
    }

    if (isMoving) {
        performMovement(deltaTime, currentPosition);
    }

    updateAnimationAndHitbox(deltaTime);
}

void Pooka::updateMovementTimer(float deltaTime, sf::Vector2f playerPosition) {
    if (movementTimer >= movementDelay) {
        movementTimer = 0.0f;
        movementDelay = 0.0f;

        sf::Vector2f newTarget = calculateNextTarget(playerPosition);

        if (newTarget != targetPosition) {
            targetPosition = newTarget;
            isMoving = true;
        }
    }
}

sf::Vector2f Pooka::calculateNextTarget(sf::Vector2f playerPosition) {
    if (status == 1) {
        return calculateGhostModeTarget();
    }
    else {
        return calculateNormalModeTarget(playerPosition);
    }
}

sf::Vector2f Pooka::calculateGhostModeTarget() {
    sf::Vector2f currentPosition = sprite.getPosition();
    sf::Vector2f directionToGhost = ghostTarget - currentPosition;
    sf::Vector2f newTarget = targetPosition;

    if (abs(directionToGhost.x) >= abs(directionToGhost.y)) {
        newTarget = moveHorizontallyToward(directionToGhost);
    }
    else {
        newTarget = moveVerticallyToward(directionToGhost);
    }

    return newTarget;
}

sf::Vector2f Pooka::moveHorizontallyToward(sf::Vector2f direction) {
    sf::Vector2f newTarget = targetPosition;

    if (direction.x < 0) {
        newTarget.x -= TILE_SIZE;
        sprite.setScale(sf::Vector2f(-1, 1));
    }
    else if (direction.x > 0) {
        newTarget.x += TILE_SIZE;
        sprite.setScale(sf::Vector2f(1, 1));
    }

    return newTarget;
}

sf::Vector2f Pooka::moveVerticallyToward(sf::Vector2f direction) {
    sf::Vector2f newTarget = targetPosition;

    if (direction.y < 0) {
        newTarget.y -= TILE_SIZE;
    }
    else if (direction.y > 0) {
        newTarget.y += TILE_SIZE;
    }

    sprite.setScale(sf::Vector2f(1, 1));
    return newTarget;
}

sf::Vector2f Pooka::calculateNormalModeTarget(sf::Vector2f playerPosition) {
    sf::Vector2f directionToPlayer = playerPosition - sprite.getPosition();

    sf::Vector2f horizontalTarget = targetPosition;
    sf::Vector2f verticalTarget = targetPosition;

    bool canMoveHorizontal = tryMoveHorizontally(directionToPlayer, horizontalTarget);
    bool canMoveVertical = tryMoveVertically(directionToPlayer, verticalTarget);

    if (canMoveHorizontal && canMoveVertical) {
        return chooseBestTarget(horizontalTarget, verticalTarget, playerPosition);
    }
    else if (canMoveHorizontal) {
        return horizontalTarget;
    }
    else if (canMoveVertical) {
        return verticalTarget;
    }

    // No valid move toward player - try patrol or ghost mode
    return handleStuckState(playerPosition);
}

bool Pooka::tryMoveHorizontally(sf::Vector2f direction, sf::Vector2f& target) {
    if (abs(direction.x) <= 1.0f) return false;

    if (direction.x < 0) {
        target.x -= TILE_SIZE;
        sprite.setScale(sf::Vector2f(-1, 1));
    }
    else {
        target.x += TILE_SIZE;
        sprite.setScale(sf::Vector2f(1, 1));
    }

    return canMoveTo(target);
}

bool Pooka::tryMoveVertically(sf::Vector2f direction, sf::Vector2f& target) {
    if (abs(direction.y) <= 1.0f) return false;

    if (direction.y < 0) {
        target.y -= TILE_SIZE;
    }
    else {
        target.y += TILE_SIZE;
    }

    return canMoveTo(target);
}

sf::Vector2f Pooka::chooseBestTarget(sf::Vector2f horizontal, sf::Vector2f vertical, sf::Vector2f playerPos) {
    if (enemyManager != nullptr) {
        const auto& enemies = enemyManager->GetEnemies();
        bool horizontalBlocked = false;
        bool verticalBlocked = false;

        for (const auto& enemy : enemies) {
            if (enemy.get() == this || enemy->getEntityType() != EntityType::POOKA || !enemy->isActive()) continue;

            sf::Vector2f enemyPos = enemy->getPosition();
            if (calculateDistance(enemyPos, horizontal) < TILE_SIZE) horizontalBlocked = true;
            if (calculateDistance(enemyPos, vertical) < TILE_SIZE) verticalBlocked = true;
        }

        if (horizontalBlocked && !verticalBlocked) return vertical;
        if (verticalBlocked && !horizontalBlocked) return horizontal;
        if (horizontalBlocked && verticalBlocked) {
            // Both blocked - try to spread out by going away from nearest Pooka
            return (rand() % 2 == 0) ? horizontal : vertical;
        }

        // Second check: Balance pack distribution around player
        int pookasAbove = 0, pookasBelow = 0, pookasLeft = 0, pookasRight = 0;
        sf::Vector2f myPos = sprite.getPosition();

        for (const auto& enemy : enemies) {
            if (enemy.get() == this || enemy->getEntityType() != EntityType::POOKA || !enemy->isActive()) continue;

            sf::Vector2f enemyPos = enemy->getPosition();
            if (enemyPos.y < playerPos.y - TILE_SIZE) pookasAbove++;
            if (enemyPos.y > playerPos.y + TILE_SIZE) pookasBelow++;
            if (enemyPos.x < playerPos.x - TILE_SIZE) pookasLeft++;
            if (enemyPos.x > playerPos.x + TILE_SIZE) pookasRight++;
        }

        // Move to balance pack
        if ((myPos.y < playerPos.y && pookasBelow < pookasAbove) ||
            (myPos.y > playerPos.y && pookasAbove < pookasBelow)) {
            return vertical;
        }
        if ((myPos.x < playerPos.x && pookasRight < pookasLeft) ||
            (myPos.x > playerPos.x && pookasLeft < pookasRight)) {
            return horizontal;
        }
    }

    // Default: move toward player
    float horizontalDist = calculateDistance(horizontal, playerPos);
    float verticalDist = calculateDistance(vertical, playerPos);
    return (horizontalDist < verticalDist) ? horizontal : vertical;
}

float Pooka::calculateDistance(sf::Vector2f from, sf::Vector2f to) {
    float dx = to.x - from.x;
    float dy = to.y - from.y;
    return std::sqrt(dx * dx + dy * dy);
}

sf::Vector2f Pooka::handleStuckState(sf::Vector2f playerPosition) {
    if (stuckTimer < ghostModeDelay) {
        return tryPatrolMove();
    }
    else {
        enterGhostMode(playerPosition);
        return targetPosition;
    }
}

sf::Vector2f Pooka::tryPatrolMove() {
    std::vector<sf::Vector2f> possibleMoves = findValidTunnelMoves();

    if (!possibleMoves.empty()) {
        int randomIndex = rand() % possibleMoves.size();
        sf::Vector2f newTarget = possibleMoves[randomIndex];
        updateSpriteDirection(newTarget);
        return newTarget;
    }

    return targetPosition;
}

std::vector<sf::Vector2f> Pooka::findValidTunnelMoves() {
    std::vector<sf::Vector2f> possibleMoves;
    sf::Vector2f moves[4] = {
        sf::Vector2f(targetPosition.x + TILE_SIZE, targetPosition.y),
        sf::Vector2f(targetPosition.x - TILE_SIZE, targetPosition.y),
        sf::Vector2f(targetPosition.x, targetPosition.y + TILE_SIZE),
        sf::Vector2f(targetPosition.x, targetPosition.y - TILE_SIZE)
    };

    for (const auto& move : moves) {
        if (canMoveTo(move) && map->getTileAt(move.x, move.y) == 0) {
            possibleMoves.push_back(move);
        }
    }

    return possibleMoves;
}

void Pooka::updateSpriteDirection(sf::Vector2f newTarget) {
    if (newTarget.x < targetPosition.x) {
        sprite.setScale(sf::Vector2f(-1, 1));
    }
    else if (newTarget.x > targetPosition.x) {
        sprite.setScale(sf::Vector2f(1, 1));
    }
}

void Pooka::enterGhostMode(sf::Vector2f playerPosition) {
    ghostTarget = findClosestTunnelToPooka(playerPosition);
    status = 1;
    stuckTimer = 0.0f;
}

void Pooka::performMovement(float deltaTime, sf::Vector2f currentPosition) {
    sf::Vector2f direction = targetPosition - currentPosition;
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (distance < 0.1f) {
        arriveAtTarget();
    }
    else {
        moveTowardTarget(direction, distance, deltaTime);
    }
}

void Pooka::arriveAtTarget() {
    sprite.setPosition(targetPosition);
    isMoving = false;
    checkExitGhostMode();
}

void Pooka::moveTowardTarget(sf::Vector2f direction, float distance, float deltaTime) {
    direction.x /= distance;
    direction.y /= distance;
    float moveDistance = speed * deltaTime;

    if (moveDistance >= distance) {
        arriveAtTarget();
    }
    else {
        sf::Vector2f newPosition = sprite.getPosition();
        newPosition.x += direction.x * moveDistance;
        newPosition.y += direction.y * moveDistance;
        sprite.setPosition(newPosition);
    }
}

void Pooka::checkExitGhostMode() {
    if (status == 1) {
        int tileType = map->getTileAt(targetPosition.x, targetPosition.y);
        if (tileType == 0) {
            status = 0;
        }
    }
}

void Pooka::updateAnimationAndHitbox(float deltaTime) {
    if (!isMoving) return;

    if (status == 1) {
        animation->Update(1, deltaTime * (speed / 20), sprite);
        hitbox.setSize(sf::Vector2f(0, 0));
    }
    else {
        animation->Update(0, deltaTime * (speed / 20), sprite);
        updateNormalHitbox();
    }

    hitbox.setPosition(sprite.getPosition());
}

void Pooka::updateNormalHitbox() {
    hitbox.setSize(sf::Vector2f(10.f, 10.f));
    hitbox.setOrigin(hitbox.getSize() / 2.f);

    sf::Vector2f pookaPos = sprite.getPosition();
    sf::FloatRect bounds = sprite.getGlobalBounds();

    // Center the hitbox horizontally depending on facing
    if (sprite.getScale().x < 0) {
        // Facing left — position at mirrored side
        hitbox.setPosition({ pookaPos.x - (bounds.size.x / 4.0f), pookaPos.y });
    }
    else {
        // Facing right — position normally
        hitbox.setPosition({ pookaPos.x + (bounds.size.x / 4.0f), pookaPos.y });
    }
}

sf::Vector2f Pooka::findClosestTunnelToPooka(sf::Vector2f playerPosition) {
    sf::Vector2f bestTunnel = playerPosition;
    float shortestDistanceToPooka = std::numeric_limits<float>::max();
    sf::Vector2f currentPos = sprite.getPosition();

    const int maxSearchDistance = 8;
    std::vector<sf::Vector2f> playerTunnels;

    for (int searchDistance = 1; searchDistance <= maxSearchDistance; searchDistance++) {
        for (int x = -searchDistance; x <= searchDistance; x++) {
            for (int y = -searchDistance; y <= searchDistance; y++) {
                if (abs(x) != searchDistance && abs(y) != searchDistance) continue;

                sf::Vector2f checkPos = playerPosition + sf::Vector2f(x * TILE_SIZE, y * TILE_SIZE);
                int tileType = map->getTileAt(checkPos.x, checkPos.y);

                if (tileType == 0) {
                    playerTunnels.push_back(checkPos);
                }
            }
        }

        if (!playerTunnels.empty()) break;
    }

    if (playerTunnels.empty()) {
        playerTunnels.push_back(playerPosition);
    }

    for (const sf::Vector2f& tunnel : playerTunnels) {
        float distance = calculateDistance(tunnel, currentPos);

        if (distance > TILE_SIZE && distance < shortestDistanceToPooka) {
            shortestDistanceToPooka = distance;
            bestTunnel = tunnel;
        }
    }

    return bestTunnel;
}

bool Pooka::canMoveTo(sf::Vector2f position) {
    if (map == nullptr) return false;

    sf::Vector2i mapSize = map->getMapSize();
    if (position.x < 0 || position.x >= mapSize.x ||
        position.y < 0 || position.y >= mapSize.y) {
        return false;
    }

    int tileType = map->getTileAt(position.x, position.y);

    if (status == 0)
        return (tileType == 0);
    if (status == 1)
        return (tileType == 0 || tileType == 2 || tileType == 3 || tileType == 4);

    return false;
}

void Pooka::AttachHarpoon() {
    if (!harpoonStuck) {
        harpoonStuck = true;
        std::cout << "Harpoon attached to Pooka" << std::endl;
    }
}

void Pooka::DetachHarpoon() {
    if (harpoonStuck) {
        harpoonStuck = false;
        if (player != nullptr) {
            player->DetachHarpoon();
        }
        std::cout << "Harpoon detached from Pooka" << std::endl;
    }
}

void Pooka::Inflate() {
    if (harpoonStuck) {
        if (pumpState < MAX_PUMP_STATE) {
            pumpState++;
            std::cout << "Pooka inflated to state: " << pumpState << std::endl;
            updateInflationSprite();

            if (pumpState >= MAX_PUMP_STATE) {
                isAlive = false;
                std::cout << "Pooka POPPED!" << std::endl;
                DetachHarpoon();
            }
        }
    }
}

void Pooka::updateInflationSprite() {
    float inflationFactor = 1.0f;
    switch (pumpState) {
    case 0: inflationFactor = 1.0f; break;
    case 1: inflationFactor = 1.2f; break;
    case 2: inflationFactor = 1.4f; break;
    case 3: inflationFactor = 1.6f; break;
    case 4: inflationFactor = 1.8f; break;
    }

    float currentXSign = (sprite.getScale().x >= 0.f) ? 1.f : -1.f;
    sprite.setScale(sf::Vector2f(currentXSign * inflationFactor, inflationFactor));

    hitbox.setSize(sf::Vector2f(size.x - 6, size.y - 6));
    hitbox.setOrigin(hitbox.getSize() / 2.0f);

    std::cout << "Pooka sprite updated for pump state: " << pumpState << std::endl;
}

bool Pooka::isHarpoonAttached() const {
    return harpoonStuck;
}

void Pooka::setPosition(sf::Vector2f pos) {
    pos.x = ((int)pos.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
    pos.y = ((int)pos.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;

    targetPosition = pos;
    sprite.setPosition(pos);
    hitbox.setPosition(pos);
    isMoving = false;
}

void Pooka::multiplySpeed(float multiple) {
}

void Pooka::Draw(sf::RenderWindow& window) {
    if (isAlive && health > 0) {
        window.draw(sprite);
        window.draw(hitbox);
    }
}