#include <iostream>
#include <cmath>
#include "Player.h"
#include "Math.h"
#include "GameState.h"

Player::Player(Map* gameMap) : Entity(EntityType::PLAYER, true, sf::Vector2i(16, 16)),
health(3), lives(2), score(0), speed(40.0f), sprite(texture),
isShooting(false), shootDirection(0, 0), harpoonSpeed(150.0f), maxHarpoonLength(32.0f),
currentHarpoonLength(0.0f), harpoonSprite(harpoonTexture), map(gameMap), createTunnels(true),
harpoonSound("Assets/Sounds/SFX/pump.mp3", SFX::Type::SOUND), MovementMusic("Assets/Sounds/Music/moving1.mp3", SFX::Type::MUSIC) {
}

void Player::Initialise() {
    Entity::Initialise();
    harpoonHitbox.setFillColor(sf::Color::Transparent);
    harpoonHitbox.setOutlineColor(sf::Color::Yellow);
    harpoonHitbox.setOutlineThickness(1);
}

void Player::Load() {
    if (!texture.loadFromFile("Assets/Sprites/Player/spritesheet1.png")) {
        std::cout << "failed to load player sprite" << '\n';
    }
    sprite.setTexture(texture);
    sprite.setTextureRect(sf::IntRect({ 0, 0 }, { size.x, size.y }));

    if (!harpoonTexture.loadFromFile("Assets/Sprites/Player/harpoon.png")) {
        std::cout << "failed to load harpoon sprite, using fallback" << '\n';
    }
    harpoonSprite.setTexture(harpoonTexture);
    harpoonSprite.setOrigin(sf::Vector2f(2, 2));

    setPosition(initialPos);
    sprite.setPosition(initialPos);

    sprite.setOrigin(sf::Vector2f(size.x / 2.0f, size.y / 2.0f));
    sprite.setScale(sf::Vector2f(1, 1));

    std::cout << "player loaded successfully" << '\n';
    animation = std::make_unique<Animation>(&texture, sf::Vector2u(4, 3), 0.25f, size.x, size.y);

    MovementMusic.setVolume(30);
    MovementMusic.setLoop(true);
}

void Player::setPosition(sf::Vector2f pos) {
    // Align position to grid
    pos.x = ((int)pos.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
    pos.y = ((int)pos.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;

    // Update base class members
    Entity::setPosition(pos);

    // Update sprite position to match
    sprite.setPosition(pos);
    std::cout << "Player position set to (" << pos.x << ", " << pos.y << ")" << std::endl;
}

void Player::Update(float deltaTime, sf::Vector2f playerPosition) {
    if (health <= 0) return;

    // Handle immobilization timer regardless of state
    if (isImmobilized) {
        immobilizationTimer += deltaTime;
        if (immobilizationTimer >= IMMOBILIZATION_DURATION) {
            isImmobilized = false;
            immobilizationTimer = 0.0f;
            std::cout << "Player can move again!" << std::endl;
        }
    }

    // Call appropriate state-specific update function
    if (gameState) {
        States currentState = gameState->getGameState();
        switch (currentState) {
        case States::START:
            updateStartState(deltaTime, playerPosition);
            break;
        case States::GAME:
            updateGameState(deltaTime, playerPosition);
            break;
        case States::WIN:
            updateWinState(deltaTime, playerPosition);
            break;
        }
    }
}

void Player::updateStartState(float deltaTime, sf::Vector2f playerPosition) {
    MovementMusic.stop();
    playerPosition = initialPos;
    if (isMoving) {
        // Calculate movement direction based on current position and target
        sf::Vector2f currentPos = sprite.getPosition();
        sf::Vector2f direction = targetPosition - currentPos;

        // Normalize direction to get unit vector
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 0.1f) { // Avoid division by zero
            direction.x /= length;
            direction.y /= length;

            // Update sprite orientation based on movement direction
            if (abs(direction.x) > abs(direction.y)) {
                // Moving horizontally
                if (direction.x > 0) {
                    // Moving right
                    sprite.setScale(sf::Vector2f(-1, 1));
                    sprite.setRotation(sf::degrees(0));
                    lastDirection = sf::Vector2f(1, 0);
                }
                else {
                    // Moving left
                    sprite.setScale(sf::Vector2f(1, 1));
                    sprite.setRotation(sf::degrees(0));
                    lastDirection = sf::Vector2f(-1, 0);
                }
            }
            else {
                // Moving vertically
                if (direction.y > 0) {
                    // Moving down
                    sprite.setRotation(sf::degrees(90));
                    sprite.setScale(sf::Vector2f(-1, -1));
                    lastDirection = sf::Vector2f(0, 1);
                }
                else {
                    // Moving up
                    sprite.setScale(sf::Vector2f(1, -1));
                    sprite.setRotation(sf::degrees(90));
                    lastDirection = sf::Vector2f(0, -1);
                }
            }
        }

        move(deltaTime, speed, sprite);
        animation->Update(0, deltaTime, sprite);

        if (!isMoving) {
            createTunnel(targetPosition);
        }
    }
}

void Player::updateGameState(float deltaTime, sf::Vector2f playerPosition) {
    bool wasMoving = isMoving;

    // Handle space key for shooting/pumping
    bool spaceCurrentlyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
    if (spaceCurrentlyPressed && !spaceKeyPressed) {
        if (harpoonedEnemy) {
            std::cout << "Pumping harpooned enemy!" << std::endl;
            harpoonedEnemy->Inflate();
            harpoonSound.play();
            animation->currentImage.x++;
            if (animation->currentImage.x >= animation->imageCount.x) {
                animation->currentImage.x = 0;
            }
            animation->currentImage.y = 1;
            animation->uvRect.position = sf::Vector2i(animation->currentImage.x * animation->uvRect.size.x,
                animation->currentImage.y * animation->uvRect.size.y);
            sprite.setTextureRect(animation->uvRect);
        }
        else if (!isShooting && !isMoving && !isImmobilized) {
            harpoonSound.play();
            startShooting();
        }
    }
    spaceKeyPressed = spaceCurrentlyPressed;

    // Handle shooting updates
    if (isShooting) {
        updateShooting(deltaTime);
    }
    // Handle movement input
    else if (!isMoving && !isImmobilized && !harpoonedEnemy) {
        sf::Vector2f newTarget = targetPosition;
        bool movementAttempted = false;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
            newTarget.x -= TILE_SIZE;
            movementAttempted = true;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
            newTarget.x += TILE_SIZE;
            movementAttempted = true;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
            newTarget.y -= TILE_SIZE;
            movementAttempted = true;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
            newTarget.y += TILE_SIZE;
            movementAttempted = true;
        }

        if (movementAttempted && harpoonedEnemy) {
            std::cout << "Movement key pressed - detaching harpoon!" << std::endl;
            DetachHarpoon();
            return;
        }

        if (movementAttempted && newTarget != targetPosition && canMoveTo(newTarget, map)) {
            setTargetPosition(newTarget);
        }
    }

    // Stop movement if immobilized or harpooned
    if (isMoving && (isImmobilized || harpoonedEnemy)) {
        isMoving = false;
        std::cout << "Movement stopped due to immobilization or harpooned enemy" << std::endl;
    }

    // Handle movement and sprite orientation
    if (isMoving) {
        // Calculate movement direction based on current position and target
        sf::Vector2f currentPos = sprite.getPosition();
        sf::Vector2f direction = targetPosition - currentPos;

        // Normalize direction to get unit vector
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 0.1f) { // Avoid division by zero
            direction.x /= length;
            direction.y /= length;

            // Update sprite orientation based on movement direction
            if (abs(direction.x) > abs(direction.y)) {
                // Moving horizontally
                if (direction.x > 0) {
                    // Moving right
                    sprite.setScale(sf::Vector2f(-1, 1));
                    sprite.setRotation(sf::degrees(0));
                    lastDirection = sf::Vector2f(1, 0);
                }
                else {
                    // Moving left
                    sprite.setScale(sf::Vector2f(1, 1));
                    sprite.setRotation(sf::degrees(0));
                    lastDirection = sf::Vector2f(-1, 0);
                }
            }
            else {
                // Moving vertically
                if (direction.y > 0) {
                    // Moving down
                    sprite.setRotation(sf::degrees(90));
                    sprite.setScale(sf::Vector2f(-1, -1));
                    lastDirection = sf::Vector2f(0, 1);
                }
                else {
                    // Moving up
                    sprite.setScale(sf::Vector2f(1, -1));
                    sprite.setRotation(sf::degrees(90));
                    lastDirection = sf::Vector2f(0, -1);
                }
            }
        }

        move(deltaTime, speed, sprite);
        animation->Update(0, deltaTime, sprite);
        if (!isMoving) {
            createTunnel(targetPosition);
        }
    }

    // Handle movement music
    if (isMoving && !wasMoving) {
        if (MovementMusic.isPlaying() == false) {
            MovementMusic.play();
            std::cout << "Started moving - playing music" << std::endl;
        }
    }
    else if (!isMoving && wasMoving) {
        if (MovementMusic.isPlaying() != false) {
            MovementMusic.pause();
            std::cout << "Stopped moving - pausing music" << std::endl;
        }
    }
}

void Player::updateWinState(float deltaTime, sf::Vector2f playerPosition) {
    // During WIN state, stop all movement and music
    MovementMusic.stop();

    // Stop any ongoing shooting
    if (isShooting) {
        stopShooting();
    }

    // Detach any harpooned enemies
    if (harpoonedEnemy) {
        DetachHarpoon();
    }

    // Stop any movement
    isMoving = false;

    // Could add win celebration animations or effects here
    // For now, just keep the player stationary
}

void Player::startShooting() {
    harpoonSound.play();
    isShooting = true;
    currentHarpoonLength = 0.0f;
    shootDirection = lastDirection;
    if (shootDirection.x == 0 && shootDirection.y == 0) {
        shootDirection = sf::Vector2f(1, 0);
    }
    harpoonStartPos = sprite.getPosition();
    std::cout << "Started shooting in direction: " << shootDirection.x << ", " << shootDirection.y << std::endl;
}

void Player::updateShooting(float deltaTime) {
    if (!isShooting) return;

    bool hitWall = false;
    currentHarpoonLength += harpoonSpeed * deltaTime;
    sf::Vector2f harpoonEndPos = harpoonStartPos + (shootDirection * currentHarpoonLength);
    harpoonSprite.setPosition(harpoonEndPos);

    sf::Vector2f harpoonSize;
    sf::Vector2f harpoonHitboxPos;
    if (abs(shootDirection.x) > abs(shootDirection.y)) {
        harpoonSize = sf::Vector2f(currentHarpoonLength, 4);
        harpoonHitboxPos = sf::Vector2f(harpoonStartPos.x + (shootDirection.x * currentHarpoonLength / 2.0f), harpoonStartPos.y);
    }
    else {
        harpoonSize = sf::Vector2f(4, currentHarpoonLength);
        harpoonHitboxPos = sf::Vector2f(harpoonStartPos.x, harpoonStartPos.y + (shootDirection.y * currentHarpoonLength / 2.0f));
    }

    harpoonHitbox.setSize(harpoonSize);
    harpoonHitbox.setPosition(harpoonHitboxPos);
    harpoonHitbox.setOrigin(sf::Vector2f(harpoonSize.x / 2.0f, harpoonSize.y / 2.0f));

    if (map != nullptr) {
        int tileType = map->getTileAt(harpoonEndPos.x, harpoonEndPos.y);
        if (tileType == 1 || tileType == 2 || tileType == 3 || tileType == 4) {
            hitWall = true;
        }
    }

    if (enemyManager != nullptr && !harpoonedEnemy) {
        for (auto& enemy : enemyManager->GetEnemies()) {
            if (enemy && enemy->isActive()) {
                sf::FloatRect enemyBounds = enemy->getBounds();
                sf::FloatRect harpoonBounds = harpoonHitbox.getGlobalBounds();
                if (harpoonBounds.findIntersection(enemyBounds)) {
                    std::cout << "Enemy harpooned at (" << enemyBounds.position.x << ", " << enemyBounds.position.y << ")" << std::endl;
                    harpoonedEnemy = enemy;
                    animation->Update(1, deltaTime, sprite);
                    enemy->AttachHarpoon();
                    isImmobilized = true;
                    immobilizationTimer = 0.0f;
                    std::cout << "Player immobilized for " << IMMOBILIZATION_DURATION << " seconds" << std::endl;
                    return;
                }
            }
        }
    }

    if (currentHarpoonLength >= maxHarpoonLength || hitWall) {
        std::cout << "Stopping harpoon - Length: " << currentHarpoonLength << ", Max: " << maxHarpoonLength << ", Hit wall: " << hitWall << std::endl;
        stopShooting();
    }
}

void Player::stopShooting() {
    std::cout << "stopShooting() called" << std::endl;
    isShooting = false;
    currentHarpoonLength = 0.0f;
    if (!harpoonedEnemy) {
        std::cout << "Harpoon stopped without hitting enemy" << std::endl;
    }
    else {
        std::cout << "Harpoon attached to enemy, keeping connection" << std::endl;
    }
    std::cout << "Shooting stopped, isShooting = " << isShooting << std::endl;
}

void Player::createTunnel(sf::Vector2f position) {
    if (map != nullptr && createTunnels) {
        int tileType = map->getTileAt(position.x, position.y);
        if (tileType == 2 || tileType == 3 || tileType == 4) {
            map->setTileAt(position.x, position.y, 0);
            if (gameState && gameState->getGameState() != States::START) {
                if (tileType == 2) score += 10;
                if (tileType == 3) score += 20;
                if (tileType == 4) score += 5;
            }
            std::cout << "Tunnel created at (" << position.x << ", " << position.y << ")! Score: " << score << std::endl;
        }
    }
}

void Player::Draw(sf::RenderWindow& window) {
    window.draw(sprite);
    window.draw(hitbox);
    if (isShooting || harpoonedEnemy) {
        sf::RectangleShape harpoonLine;
        if (harpoonedEnemy) {
            sf::Vector2f enemyPos = harpoonedEnemy->getBounds().position;
            enemyPos.x += harpoonedEnemy->getBounds().size.x / 2.0f;
            enemyPos.y += harpoonedEnemy->getBounds().size.y / 2.0f;
            sf::Vector2f playerPos = sprite.getPosition();
            sf::Vector2f direction = enemyPos - playerPos;
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (abs(direction.x) > abs(direction.y)) {
                harpoonLine.setSize(sf::Vector2f(distance, 2));
                harpoonLine.setPosition(sf::Vector2f(playerPos.x, playerPos.y - 1));
                if (direction.x < 0) {
                    harpoonLine.setPosition(sf::Vector2f(playerPos.x - distance, playerPos.y - 1));
                }
            }
            else {
                harpoonLine.setSize(sf::Vector2f(2, distance));
                harpoonLine.setPosition(sf::Vector2f(playerPos.x - 1, playerPos.y));
                if (direction.y < 0) {
                    harpoonLine.setPosition(sf::Vector2f(playerPos.x - 1, playerPos.y - distance));
                }
            }
        }
        else {
            if (abs(shootDirection.x) > abs(shootDirection.y)) {
                harpoonLine.setSize(sf::Vector2f(currentHarpoonLength, 2));
                harpoonLine.setPosition(sf::Vector2f(harpoonStartPos.x, harpoonStartPos.y - 1));
                if (shootDirection.x < 0) {
                    harpoonLine.setPosition(sf::Vector2f(harpoonStartPos.x - currentHarpoonLength, harpoonStartPos.y - 1));
                }
            }
            else {
                harpoonLine.setSize(sf::Vector2f(2, currentHarpoonLength));
                harpoonLine.setPosition(sf::Vector2f(harpoonStartPos.x - 1, harpoonStartPos.y));
                if (shootDirection.y < 0) {
                    harpoonLine.setPosition(sf::Vector2f(harpoonStartPos.x - 1, harpoonStartPos.y - currentHarpoonLength));
                }
            }
        }
        harpoonLine.setFillColor(sf::Color::White);
        window.draw(harpoonLine);
        if (isShooting) {
            window.draw(harpoonHitbox);
        }
    }
}

sf::FloatRect Player::getHarpoonBounds() const {
    if (isShooting) {
        return harpoonHitbox.getGlobalBounds();
    }
    return sf::FloatRect();
}

bool Player::isCurrentlyShooting() const {
    return isShooting;
}

void Player::shoot() {
    if (!isShooting && !isMoving && !isImmobilized && !harpoonedEnemy) {
        startShooting();
    }
}

void Player::DetachHarpoon() {
    if (harpoonedEnemy) {
        std::cout << "Player harpoon detached from enemy" << std::endl;
        harpoonedEnemy = nullptr;
        isImmobilized = false;
        immobilizationTimer = 0.0f;
        isShooting = false;
        currentHarpoonLength = 0.0f;
    }
}