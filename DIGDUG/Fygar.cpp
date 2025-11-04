#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include "Fygar.h"
#include "Player.h"

Fygar::Fygar(Map* gameMap, Player* player) : Entity(EntityType::FYGAR, true, sf::Vector2i(16, 16), 300),
health(4), speed(25.0f), status(0), sprite(texture), fireBreathSprite(fireBreathTexture),
pumpSound(pumpBuffer), map(gameMap), player(player), fireBreathSound("Assets/Sounds/SFX/Enemies/Fygar/fygarFire.mp3", SFX::Type::SOUND) {
}

void Fygar::Initialise() {
    Entity::Initialise();
}

void Fygar::Load() {
    if (!texture.loadFromFile("Assets/Sprites/Fygar/fygarspritesheet.png")) {
        std::cout << "failed to load Fygar sprite" << '\n';
    }
    sprite.setTexture(texture);
    sprite.setTextureRect(sf::IntRect({ 0, 0 }, { size.x, size.y }));
    sprite.setOrigin(sf::Vector2f(size.x / 2.0f, size.y / 2.0f));
    sprite.setScale(sf::Vector2f(1, 1));
    sprite.setPosition({ 0, 0 });

    // Load fire breath sprite from separate texture
    if (!fireBreathTexture.loadFromFile("Assets/Sprites/Fygar/fygarfire.png")) {
        std::cout << "failed to load fire breath sprite" << '\n';
    }
    fireBreathSprite.setTexture(fireBreathTexture);
    // Will set texture rect dynamically based on length
    fireBreathSprite.setOrigin(sf::Vector2f(0.0f, 8.0f)); // Origin at left-center
    fireBreathSprite.setScale(sf::Vector2f(1, 1));

    if (!pumpBuffer.loadFromFile("Assets/Sounds/SFX/pump.mp3")) {
        std::cout << "failed to load pump sound" << '\n';
    }
    pumpSound.setBuffer(pumpBuffer);
    fireBreathSound.setVolume(50);  
    fireBreathSound.setLoop(false);

    std::cout << "fygar loaded successfully" << '\n';
    animation = std::make_unique<Animation>(&texture, sf::Vector2u(2, 2), 0.25f, size.x, size.y, true);
}

void Fygar::Update(float deltaTime, sf::Vector2f playerPosition) {
    if (health <= 0 || !isAlive) return;

    updatePumpState(deltaTime);
    updatePumpCooldown(deltaTime);
    updateFireBreathState(deltaTime);

    if (preparingShot) {
        // Safety check - if fire breath already started, stop preparing
        if (fireBreathActive) {
            preparingShot = false;
            prepareTimer = 0.0f;
            sprite.setColor(sf::Color::White);
            return;
        }

        prepareTimer += deltaTime;
        isMoving = false;

        // Check if we should fire
        if (prepareTimer >= PREPARE_DURATION) {
            // Double safety check before firing
            if (!fireBreathActive) {
                breatheFire(playerPosition);
            }
            preparingShot = false;
            prepareTimer = 0.0f;
            sprite.setColor(sf::Color::White);
            shootCooldown = SHOOT_COOLDOWN;
            return;
        }

        // Visual feedback - make sprite flash/highlight
        if (static_cast<int>(prepareTimer * 4) % 2 == 0) {
            sprite.setColor(sf::Color(255, 200, 200)); // Reddish tint
        }
        else {
            sprite.setColor(sf::Color::White);
        }

        return; // Don't move while preparing
    }

    if (fireBreathActive) {
        // While breathing fire, freeze direction & position
        // Make sure the sprite stays facing the locked fire direction
        sprite.setScale(sf::Vector2f(fireBreathDirection, 1.0f));
        return;
    }

    if (canMove()) {
        if (!isMoving && isGridAligned() && shouldPrepareShot(playerPosition)) {
            preparingShot = true;
            prepareTimer = 0.0f;
            isMoving = false;

            // Lock the direction right here (this becomes our "locked" direction)
            sf::Vector2f fygarPos = sprite.getPosition();
            if (playerPosition.x > fygarPos.x) {
                fireBreathDirection = 1.0f; // Right
            }
            else if (playerPosition.x < fygarPos.x) {
                fireBreathDirection = -1.0f; // Left
            }

            sprite.setScale(sf::Vector2f(fireBreathDirection, 1.0f));
        }
        else {
            sprite.setColor(sf::Color::White);
            updateMovement(deltaTime, playerPosition);
        }
    }
}


bool Fygar::shouldPrepareShot(sf::Vector2f playerPosition) {
    if (status == 1) return false; // cant shoot while in ghost mode
    if (preparingShot) return true; // Continue preparing if already started
    if (shootCooldown > 0.0f) return false; // Still on cooldown
    if (fireBreathActive) return false; // Already breathing fire

    sf::Vector2f currentPos = sprite.getPosition();
    sf::Vector2f directionToPlayer = playerPosition - currentPos;

    // Check if player is on same horizontal line (within half a tile tolerance)
    if (abs(directionToPlayer.y) > TILE_SIZE / 2) return false;

    // Check if player is within 2 tiles horizontally
    float horizontalDistance = abs(directionToPlayer.x);
    if (horizontalDistance > TILE_SIZE * 2 || horizontalDistance < TILE_SIZE * 0.5f) return false;

    // Check if there's a clear tunnel path to the player
    return hasHorizontalTunnelPath(currentPos, playerPosition);
}

bool Fygar::hasHorizontalTunnelPath(sf::Vector2f from, sf::Vector2f to) {
    float direction = (to.x > from.x) ? 1.0f : -1.0f;
    float currentX = from.x;
    float targetX = to.x;

    // Check each tile between Fygar and player
    while (abs(currentX - from.x) < abs(targetX - from.x)) {
        currentX += direction * TILE_SIZE;
        int tileType = map->getTileAt(currentX, from.y);

        // If we hit a wall (non-tunnel tile), path is blocked
        if (tileType != 0) {
            return false;
        }
    }

    return true;
}


void Fygar::breatheFire(sf::Vector2f playerPosition) {
    if (fireBreathActive || status == 1) {
        std::cout << "breatheFire() blocked - already active" << std::endl;
        return; // Don't start a new fire breath if one is active
    }

    std::cout << "breatheFire() CALLED" << std::endl;

    // Initialize fire breath
    fireBreathActive = true;
    fireBreathTimer = 0.0f;

    // Play sound once (restart it to be safe)
    if (fireBreathSound.isPlaying()) {
        fireBreathSound.stop();   
    }
    fireBreathSound.play();

    std::cout << "Fygar at (" << sprite.getPosition().x << ", " << sprite.getPosition().y
        << ") breathes fire in direction: " << fireBreathDirection << std::endl;
}


void Fygar::updateFireBreathState(float deltaTime) {
    if (shootCooldown > 0.0f) {
        shootCooldown -= deltaTime;
    }

    if (!fireBreathActive) return;
    if (status == 1) return;

    fireBreathTimer += deltaTime;

    // --- Determine current fire length ---
    float currentLength = 0.0f;
    if (fireBreathTimer < FIRE_BREATH_GROW_TIME) {
        // Growing phase
        float growthProgress = fireBreathTimer / FIRE_BREATH_GROW_TIME;
        currentLength = FIRE_BREATH_MAX_LENGTH * growthProgress;
    }
    else if (fireBreathTimer < FIRE_BREATH_TOTAL_TIME) {
        // Holding phase
        currentLength = FIRE_BREATH_MAX_LENGTH;
    }
    else {
        // Fire breath finished
        fireBreathActive = false;
        hasHitPlayerWithFire = false; // Reset for next fire breath

        // Ensure sound is stopped when the effect ends
        if (fireBreathSound.isPlaying()) {
            fireBreathSound.stop();
        }

        std::cout << "Fire breath dissipated" << std::endl;
        return;
    }

    // --- Choose which sprite frames to show based on length ---
    float lengthInTiles = currentLength / TILE_SIZE;
    int spriteStartIndex = 0;
    int spriteCount = 0;

    if (lengthInTiles <= 1.0f) {
        spriteStartIndex = 0;
        spriteCount = 1;
    }
    else if (lengthInTiles <= 2.0f) {
        spriteStartIndex = 1;
        spriteCount = 2;
    }
    else {
        spriteStartIndex = 3;
        spriteCount = 3;
    }

    int totalSpriteWidth = spriteCount * 16;
    fireBreathSprite.setTextureRect(sf::IntRect({ spriteStartIndex * 16, 0 }, { totalSpriteWidth, 16 }));

    // --- Position fire sprite ---
    sf::Vector2f fygarPos = sprite.getPosition();
    sf::Vector2f fireBreathPos = fygarPos;

    sprite.setScale(sf::Vector2f(fireBreathDirection, 1.0f));

    if (fireBreathDirection < 0) {
        // Facing left — flip the fire horizontally
        fireBreathSprite.setOrigin(sf::Vector2f(0.f, 8.f));
        fireBreathSprite.setScale(sf::Vector2f(-1.f, 1.f));
        fireBreathPos.x -= 10.0f;
    }
    else {
        // Facing right — normal orientation
        fireBreathSprite.setOrigin(sf::Vector2f(0.f, 8.f));
        fireBreathSprite.setScale(sf::Vector2f(1.f, 1.f));
        fireBreathPos.x += 10.0f;
    }

    fireBreathSprite.setPosition(fireBreathPos);

    // --- Fire hitbox setup (visual + collision) ---
    float visibleWidth = static_cast<float>(totalSpriteWidth);
    fireHitbox.setSize(sf::Vector2f(visibleWidth, 16.f));
    fireHitbox.setFillColor(sf::Color::Transparent);
    fireHitbox.setOutlineColor(sf::Color::Red);
    fireHitbox.setOutlineThickness(1.f);

    if (fireBreathDirection < 0) {
        fireHitbox.setOrigin({ fireHitbox.getSize().x, 8.f });
        fireHitbox.setPosition({ fireBreathPos.x, fireBreathPos.y });
    }
    else {
        fireHitbox.setOrigin({ 0.f, 8.f });
        fireHitbox.setPosition({ fireBreathPos.x, fireBreathPos.y });
    }

    // --- Collision check with player (ONLY ONCE) ---
    if (player != nullptr && !hasHitPlayerWithFire) {
        sf::Vector2f playerPos = player->getPosition();
        float verticalDiff = abs(playerPos.y - fygarPos.y);

        if (verticalDiff < TILE_SIZE / 2) {
            float horizontalDiff = playerPos.x - fygarPos.x;
            float distanceToPlayer = abs(horizontalDiff);
            bool inDirection = (fireBreathDirection > 0 && horizontalDiff > 0) ||
                (fireBreathDirection < 0 && horizontalDiff < 0);

            if (inDirection && distanceToPlayer <= currentLength) {
                player->setHealth(0);
                hasHitPlayerWithFire = true; // Mark as hit so sound only plays once
                std::cout << "Player hit by fire breath!" << std::endl;
            }
        }
    }
}


void Fygar::updatePumpState(float deltaTime) {
    if (!harpoonStuck) return;

    pumpTimer += deltaTime;
    if (pumpTimer >= PUMP_DURATION) {
        handleDeflation();
        pumpTimer = 0.0f;
        updateInflationSprite();
    }
}

void Fygar::handleDeflation() {
    if (pumpState > 0) {
        pumpState = std::max(0, pumpState - 1);
        std::cout << "Fygar deflated to state: " << pumpState << std::endl;

        if (pumpState == 0) {
            regenerateHealth();
            DetachHarpoon();
        }
    }
    else {
        DetachHarpoon();
    }
}

void Fygar::regenerateHealth() {
    const int MAX_HEALTH = 4;
    if (health < MAX_HEALTH) {
        health += 1;
        std::cout << "Fygar health regen to: " << health << std::endl;
    }
}

void Fygar::updatePumpCooldown(float deltaTime) {
    if (pumpCooldownTimer > 0.0f) {
        pumpCooldownTimer -= deltaTime;
    }
}

bool Fygar::canMove() const {
    return health > 0 && isAlive && pumpState == 0 && !harpoonStuck;
}

void Fygar::updateMovement(float deltaTime, sf::Vector2f playerPosition) {
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

void Fygar::updateMovementTimer(float deltaTime, sf::Vector2f playerPosition) {
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

sf::Vector2f Fygar::calculateNextTarget(sf::Vector2f playerPosition) {
    if (status == 1) {
        return calculateGhostModeTarget();
    }
    else {
        return calculateNormalModeTarget(playerPosition);
    }
}

sf::Vector2f Fygar::calculateGhostModeTarget() {
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

sf::Vector2f Fygar::moveHorizontallyToward(sf::Vector2f direction) {
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

sf::Vector2f Fygar::moveVerticallyToward(sf::Vector2f direction) {
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

sf::Vector2f Fygar::calculateNormalModeTarget(sf::Vector2f playerPosition) {
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

bool Fygar::tryMoveHorizontally(sf::Vector2f direction, sf::Vector2f& target) {
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

bool Fygar::tryMoveVertically(sf::Vector2f direction, sf::Vector2f& target) {
    if (abs(direction.y) <= 1.0f) return false;

    if (direction.y < 0) {
        target.y -= TILE_SIZE;
    }
    else {
        target.y += TILE_SIZE;
    }

    return canMoveTo(target);
}

sf::Vector2f Fygar::chooseBestTarget(sf::Vector2f horizontal, sf::Vector2f vertical, sf::Vector2f playerPos) {
    // fygar will ALWAYS prefer to move to a position on the same row as the player (so it can use its fire breath)
    sf::Vector2f currentPos = sprite.getPosition();
    sf::Vector2f playerPosition = playerPos;

    float verticalDiff = abs(currentPos.y - playerPosition.y);

    if (verticalDiff > TILE_SIZE / 2) {
        return vertical;
    }

    return horizontal;
}

float Fygar::calculateDistance(sf::Vector2f from, sf::Vector2f to) {
    float dx = to.x - from.x;
    float dy = to.y - from.y;
    return std::sqrt(dx * dx + dy * dy);
}

sf::Vector2f Fygar::handleStuckState(sf::Vector2f playerPosition) {
    if (stuckTimer < ghostModeDelay) {
        return tryPatrolMove();
    }
    else {
        enterGhostMode(playerPosition);
        return targetPosition;
    }
}

sf::Vector2f Fygar::tryPatrolMove() {
    std::vector<sf::Vector2f> possibleMoves = findValidTunnelMoves();

    if (!possibleMoves.empty()) {
        int randomIndex = rand() % possibleMoves.size();
        sf::Vector2f newTarget = possibleMoves[randomIndex];
        updateSpriteDirection(newTarget);
        return newTarget;
    }

    return targetPosition;
}

std::vector<sf::Vector2f> Fygar::findValidTunnelMoves() {
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

void Fygar::updateSpriteDirection(sf::Vector2f newTarget) {
    if (newTarget.x < targetPosition.x) {
        sprite.setScale(sf::Vector2f(-1, 1));
    }
    else if (newTarget.x > targetPosition.x) {
        sprite.setScale(sf::Vector2f(1, 1));
    }
}

void Fygar::enterGhostMode(sf::Vector2f playerPosition) {
    ghostTarget = findClosestTunnelToFygar(playerPosition);
    status = 1;
    stuckTimer = 0.0f;
}

void Fygar::performMovement(float deltaTime, sf::Vector2f currentPosition) {
    sf::Vector2f direction = targetPosition - currentPosition;
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (distance < 0.1f) {
        arriveAtTarget();
    }
    else {
        moveTowardTarget(direction, distance, deltaTime);
    }
}

void Fygar::arriveAtTarget() {
    sprite.setPosition(targetPosition);
    isMoving = false;
    checkExitGhostMode();
}

void Fygar::moveTowardTarget(sf::Vector2f direction, float distance, float deltaTime) {
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

void Fygar::checkExitGhostMode() {
    if (status == 1) {
        int tileType = map->getTileAt(targetPosition.x, targetPosition.y);
        if (tileType == 0) {
            status = 0;
        }
    }
}

void Fygar::updateAnimationAndHitbox(float deltaTime) {
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

void Fygar::updateNormalHitbox() {
    hitbox.setSize(sf::Vector2f(10.f, 10.f));
    hitbox.setOrigin(hitbox.getSize() / 2.f);

    sf::Vector2f fygarPos = sprite.getPosition();
    sf::FloatRect bounds = sprite.getGlobalBounds();

    // Center the hitbox horizontally depending on facing
    if (sprite.getScale().x < 0) {
        // Facing left — position at mirrored side
        hitbox.setPosition({ fygarPos.x - (bounds.size.x / 4.0f), fygarPos.y });
    }
    else {
        // Facing right — position normally
        hitbox.setPosition({ fygarPos.x + (bounds.size.x / 4.0f), fygarPos.y });
    }
}


sf::Vector2f Fygar::findClosestTunnelToFygar(sf::Vector2f playerPosition) {
    sf::Vector2f bestTunnel = playerPosition;
    float shortestDistanceToFygar = std::numeric_limits<float>::max();
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

        if (distance > TILE_SIZE && distance < shortestDistanceToFygar) {
            shortestDistanceToFygar = distance;
            bestTunnel = tunnel;
        }
    }

    return bestTunnel;
}

bool Fygar::canMoveTo(sf::Vector2f position) {
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

bool Fygar::isGridAligned() const {
    sf::Vector2f pos = sprite.getPosition();

    // Check if position is aligned to grid (within small tolerance for floating point errors)
    float xRemainder = fmod(pos.x - (TILE_SIZE / 2.0f), TILE_SIZE);
    float yRemainder = fmod(pos.y - (TILE_SIZE / 2.0f), TILE_SIZE);

    const float TOLERANCE = 0.5f;
    return (abs(xRemainder) < TOLERANCE && abs(yRemainder) < TOLERANCE);
}

void Fygar::AttachHarpoon() {
    if (!harpoonStuck) {
        harpoonStuck = true;
        std::cout << "Harpoon attached to Fygar" << std::endl;
    }
}

void Fygar::DetachHarpoon() {
    if (harpoonStuck) {
        harpoonStuck = false;
        if (player != nullptr) {
            player->DetachHarpoon();
        }
        std::cout << "Harpoon detached from Fygar" << std::endl;
    }
}

void Fygar::Inflate() {
    if (harpoonStuck) {
        if (pumpState < MAX_PUMP_STATE) {
            pumpState++;
            std::cout << "Fygar inflated to state: " << pumpState << std::endl;
            updateInflationSprite();

            if (pumpState >= MAX_PUMP_STATE) {
                isAlive = false;
                std::cout << "Fygar POPPED!" << std::endl;
                DetachHarpoon();
            }
        }
    }
}

void Fygar::updateInflationSprite() {
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

    std::cout << "Fygar sprite updated for pump state: " << pumpState << std::endl;
}

bool Fygar::isHarpoonAttached() const {
    return harpoonStuck;
}

void Fygar::setPosition(sf::Vector2f pos) {
    pos.x = ((int)pos.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
    pos.y = ((int)pos.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;

    targetPosition = pos;
    sprite.setPosition(pos);
    hitbox.setPosition(pos);
    isMoving = false;
}

void Fygar::multiplySpeed(float multiple) {
}

void Fygar::Draw(sf::RenderWindow& window) {
    if (isAlive && health > 0) {
        window.draw(sprite);
        window.draw(hitbox); // body hitbox

        if (fireBreathActive) {
            window.draw(fireBreathSprite);
            window.draw(fireHitbox); // hitbox for the firebreath
        }
    }
}
