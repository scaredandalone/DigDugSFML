#include <iostream>
#include <cmath>

#include "Player.h"
#include "Math.h"

Player::Player(Map* gameMap) : Entity(EntityType::PLAYER, true, sprite), health(3), lives(2), score(0), speed(30.0f), sprite(texture), size(16,16),
isMoving(false), targetPosition(0, 0), map(gameMap), isShooting(false), shootDirection(0, 0), harpoonSpeed(150.0f), maxHarpoonLength(32.0f), currentHarpoonLength(0.0f), harpoonSprite(harpoonTexture)
{
}

void Player::Initialise()
{
    hitbox.setSize(sf::Vector2f(size));
    hitbox.setFillColor(sf::Color::Transparent);
    hitbox.setOutlineColor(sf::Color::Red);
    hitbox.setOutlineThickness(1);

    // Initialize harpoon
    harpoonHitbox.setFillColor(sf::Color::Transparent);
    harpoonHitbox.setOutlineColor(sf::Color::Yellow);
    harpoonHitbox.setOutlineThickness(1);
}

void Player::Load()
{
    if (!texture.loadFromFile("Assets/Sprites/Player/spritesheet1.png")) {
        std::cout << "failed to load player sprite" << '\n';
    }

    // Load harpoon texture
    if (!harpoonTexture.loadFromFile("Assets/Sprites/Player/harpoon.png")) {
        std::cout << "failed to load harpoon sprite, using fallback" << '\n';
    }

    harpoonSprite.setTexture(harpoonTexture);
    harpoonSprite.setOrigin(sf::Vector2f(2, 2));

    if (!movementMusic.openFromFile("Assets/Sounds/Music/moving1.mp3")) {
        std::cout << "failed to load movement music" << '\n';
    }
    movementMusic.setLooping(true);
    movementMusic.setVolume(35);
    if (!buffer.loadFromFile("Assets/Sounds/SFX/pump.mp3")) {
        std::cout << "failed to load pump sound" << '\n';
    }
    harpoonSFX = sf::Sound(buffer);
    harpoonSFX->setVolume(75);


    sprite.setTextureRect(sf::IntRect({ 0 * size.x ,0 * size.y }, { size.x,size.y }));
    // snap pos to grid
    sf::Vector2f initialPos(122, 144);

    initialPos.x = ((int)initialPos.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
    initialPos.y = ((int)initialPos.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
    targetPosition = sf::Vector2f(initialPos.x, initialPos.y);

    sprite.setPosition(initialPos);
    sprite.setOrigin(sf::Vector2f(size.x / 2.0f, size.y / 2.0f));
    sprite.scale(sf::Vector2f(1, 1));

    hitbox.setSize(sf::Vector2f(size.x * sprite.getScale().x, size.y * sprite.getScale().y));
    hitbox.setOrigin(sf::Vector2f(size.x / 2.0f, size.y / 2.0f));

    std::cout << "player loaded successfully" << '\n';
    animation = std::make_unique<Animation>(&texture, sf::Vector2u(4, 3), 0.25f, size.x, size.y);
}

void Player::Update(float deltaTime) {
    if (health > 0) {
        sf::Vector2f currentPosition = sprite.getPosition();
        bool wasMoving = isMoving;

        if (isImmobilized) {
            immobilizationTimer += deltaTime;
            if (immobilizationTimer >= IMMOBILIZATION_DURATION) {
                isImmobilized = false;
                immobilizationTimer = 0.0f;
                std::cout << "Player can move again!" << std::endl;
            }
        }
        bool spaceCurrentlyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

        if (spaceCurrentlyPressed && !spaceKeyPressed) {
            if (harpoonedEnemy) {
                // Pump enemy and advance to next animation frame
                std::cout << "Pumping harpooned enemy!" << std::endl;
                harpoonedEnemy->Inflate();
                harpoonSFX->play();

                // next animation frame when spacebar is pressed
                animation->currentImage.x++;
                if (animation->currentImage.x >= animation->imageCount.x) {
                    animation->currentImage.x = 0; // loop back when reaches the end of animation 
                }

                // set animation row
                animation->currentImage.y = 1; // row 1 for pump
                animation->uvRect.position = sf::Vector2i(animation->currentImage.x * animation->uvRect.size.x,
                    animation->currentImage.y * animation->uvRect.size.y);
                sprite.setTextureRect(animation->uvRect);
            }
            else if (!isShooting && !isMoving && !isImmobilized) {
                harpoonSFX->play();
                startShooting();
            }
        }
        spaceKeyPressed = spaceCurrentlyPressed;

        if (isShooting) {
            updateShooting(deltaTime);
        }
        // allow movement if not shooting, not immobilized, and no harpooned enemy
        else if (!isMoving && !isImmobilized) {
            sf::Vector2f newTarget = targetPosition;
            bool movementAttempted = false;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
                newTarget.x -= TILE_SIZE;
                sprite.setScale(sf::Vector2f(1, 1));
                sprite.setRotation(sf::degrees(0));
                lastDirection = sf::Vector2f(-1, 0);
                movementAttempted = true;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
                newTarget.x += TILE_SIZE;
                sprite.setScale(sf::Vector2f(-1, 1));
                sprite.setRotation(sf::degrees(0));
                lastDirection = sf::Vector2f(1, 0);
                movementAttempted = true;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
                newTarget.y -= TILE_SIZE;
                sprite.setScale(sf::Vector2f(1, -1));
                sprite.setRotation(sf::degrees(90));
                lastDirection = sf::Vector2f(0, -1);
                movementAttempted = true;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
                newTarget.y += TILE_SIZE;
                sprite.setRotation(sf::degrees(90));
                sprite.setScale(sf::Vector2f(-1, -1));
                lastDirection = sf::Vector2f(0, 1);
                movementAttempted = true;
            }

            // detach harpoon on movement
            if (movementAttempted && harpoonedEnemy) {
                std::cout << "Movement key pressed - detaching harpoon!" << std::endl;
                DetachHarpoon();
                return; 
            }

            // movement logic (only executes if no harpooned enemy)
            if (movementAttempted && newTarget != targetPosition && canMoveTo(newTarget)) {
                targetPosition = newTarget;
                isMoving = true;
            }
        }

        if (isMoving && !isImmobilized) {
            sf::Vector2f direction = targetPosition - currentPosition;
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            animation->Update(0, deltaTime, sprite);
            if (distance < 0.1f) {
                //  snap to position
                sprite.setPosition(targetPosition);
                isMoving = false;

                // create tunnels -----> FUCK YOU
                createTunnel(targetPosition);
            }
            else {
                direction.x /= distance;
                direction.y /= distance;
                float moveDistance = speed * deltaTime;
                if (moveDistance >= distance) {
                    sprite.setPosition(targetPosition);
                    isMoving = false;
                    createTunnel(targetPosition);
                }
                else {
                    currentPosition.x += direction.x * moveDistance;
                    currentPosition.y += direction.y * moveDistance;
                    sprite.setPosition(currentPosition);
                }
            }
        }
        else if (isMoving && (isImmobilized || harpoonedEnemy)) {
            // Stop movement if we become immobilized or harpoon an enemy
            isMoving = false;
            std::cout << "Movement stopped due to immobilization or harpooned enemy" << std::endl;
        }

        // Handle movement music
        if (isMoving && !wasMoving) {
            std::cout << "Started moving - playing music" << std::endl;
            if (movementMusic.getStatus() != sf::SoundSource::Status::Playing) {
                movementMusic.play();
            }
        }
        else if (!isMoving && wasMoving) {
            std::cout << "Stopped moving - pausing music" << std::endl;
            if (movementMusic.getStatus() == sf::SoundSource::Status::Playing) {
                movementMusic.pause();
            }
        }
        hitbox.setPosition(sprite.getPosition());
    }
}

void Player::startShooting() {
    harpoonSFX->play();
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
        // Horizontal harpoon
        harpoonSize = sf::Vector2f(currentHarpoonLength, 4);
        harpoonHitboxPos = sf::Vector2f(harpoonStartPos.x + (shootDirection.x * currentHarpoonLength / 2.0f), harpoonStartPos.y);
    }
    else {
        // Vertical harpoon
        harpoonSize = sf::Vector2f(4, currentHarpoonLength);
        harpoonHitboxPos = sf::Vector2f(harpoonStartPos.x, harpoonStartPos.y + (shootDirection.y * currentHarpoonLength / 2.0f));
    }

    harpoonHitbox.setSize(harpoonSize);
    harpoonHitbox.setPosition(harpoonHitboxPos);
    harpoonHitbox.setOrigin(sf::Vector2f(harpoonSize.x / 2.0f, harpoonSize.y / 2.0f));

    // Check for collision with walls
    if (map != nullptr) {
        int tileType = map->getTileAt(harpoonEndPos.x, harpoonEndPos.y);
        if (tileType == 1 || tileType == 2 || tileType == 3 || tileType == 4) {
            hitWall = true;
        }
    }

    // Check for enemy collision - FIXED VERSION
    if (enemyManager != nullptr && !harpoonedEnemy) {
        for (auto& enemy : enemyManager->GetEnemies()) {
            if (enemy && enemy->isActive()) {                sf::FloatRect enemyBounds = enemy->getBounds();
                sf::FloatRect harpoonBounds = harpoonHitbox.getGlobalBounds();

                std::cout << "Checking collision - Enemy at (" << enemyBounds.position.x << ", " << enemyBounds.position.y
                    << ") size (" << enemyBounds.size.x << ", " << enemyBounds.size.y << ")" << std::endl;
                std::cout << "Harpoon at (" << harpoonBounds.position.x << ", " << harpoonBounds.position.y
                    << ") size (" << harpoonBounds.size.x << ", " << harpoonBounds.size.y << ")" << std::endl;

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

    // Check if harpoon reached max length or hit a wall
    if (currentHarpoonLength >= maxHarpoonLength || hitWall) {
        std::cout << "Stopping harpoon - Length: " << currentHarpoonLength << ", Max: " << maxHarpoonLength << ", Hit wall: " << hitWall << std::endl;
        stopShooting();
    }
}

void Player::stopShooting() {
    std::cout << "stopShooting() called" << std::endl;
    // Reset shooting state
    isShooting = false;
    currentHarpoonLength = 0.0f;

    // Handle harpooned enemy - but DON'T detach it here if we hit an enemy
    // Only detach if we didn't hit an enemy (harpoonedEnemy would be nullptr)
    if (!harpoonedEnemy) {
        // Harpoon hit a wall or reached max length without hitting enemy
        std::cout << "Harpoon stopped without hitting enemy" << std::endl;
    }
    else {
        // Keep the harpoon attached to the enemy
        std::cout << "Harpoon attached to enemy, keeping connection" << std::endl;
    }

    std::cout << "Shooting stopped, isShooting = " << isShooting << std::endl;
}

bool Player::canMoveTo(sf::Vector2f position) {
    if (map == nullptr) return false;

    // if position is within map bounds
    sf::Vector2i mapSize = map->getMapSize();
    if (position.x < 0 || position.x >= mapSize.x ||
        position.y < 0 || position.y >= mapSize.y) {
        return false;
    }
    int tileType = map->getTileAt(position.x, position.y);

    // define tiles that player can walk through
    return (tileType == 0 || tileType == 2 || tileType == 3 || tileType == 4);
}

void Player::createTunnel(sf::Vector2f position) {
    if (map != nullptr) {
        int tileType = map->getTileAt(position.x, position.y);

        // specify diggable tile types here:
        if (tileType == 2 || tileType == 3 || tileType == 4) {
            map->setTileAt(position.x, position.y, 0); //(creates tunnel)

            if (tileType == 2) score += 10;  // this is score for each tiletype
            if (tileType == 3) score += 20;  // this is score for each tiletype  
            if (tileType == 4) score += 5;   // this is score for each tiletype

            std::cout << "Tunnel created! Score: " << score << std::endl;
        }
    }
}

void Player::Draw(sf::RenderWindow& window) {
    window.draw(sprite);
    window.draw(hitbox);

    // Draw harpoon if shooting OR if attached to enemy
    if (isShooting || harpoonedEnemy) {
        // Draw harpoon line
        sf::RectangleShape harpoonLine;

        // If attached to enemy, draw line from player to enemy
        if (harpoonedEnemy) {
            sf::Vector2f enemyPos = harpoonedEnemy->getBounds().position;
            enemyPos.x += harpoonedEnemy->getBounds().size.x / 2.0f;
            enemyPos.y += harpoonedEnemy->getBounds().size.y / 2.0f;

            sf::Vector2f playerPos = sprite.getPosition();
            sf::Vector2f direction = enemyPos - playerPos;
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (abs(direction.x) > abs(direction.y)) {
                // Horizontal
                harpoonLine.setSize(sf::Vector2f(distance, 2));
                harpoonLine.setPosition(sf::Vector2f(playerPos.x, playerPos.y - 1));
                if (direction.x < 0) {
                    harpoonLine.setPosition(sf::Vector2f(playerPos.x - distance, playerPos.y - 1));
                }
            }
            else {
                // Vertical
                harpoonLine.setSize(sf::Vector2f(2, distance));
                harpoonLine.setPosition(sf::Vector2f(playerPos.x - 1, playerPos.y));
                if (direction.y < 0) {
                    harpoonLine.setPosition(sf::Vector2f(playerPos.x - 1, playerPos.y - distance));
                }
            }
        }
        else {
            // Normal shooting harpoon
            if (abs(shootDirection.x) > abs(shootDirection.y)) {
                // Horizontal
                harpoonLine.setSize(sf::Vector2f(currentHarpoonLength, 2));
                harpoonLine.setPosition(sf::Vector2f(harpoonStartPos.x, harpoonStartPos.y - 1));
                if (shootDirection.x < 0) {
                    harpoonLine.setPosition(sf::Vector2f(harpoonStartPos.x - currentHarpoonLength, harpoonStartPos.y - 1));
                }
            }
            else {
                // Vertical
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
    // function is now handled by the spacebar input in Update()
    // call startShooting() directly if needed from other classes
    if (!isShooting && !isMoving && !isImmobilized && !harpoonedEnemy) {
        startShooting();
    }
}


void Player::DetachHarpoon() {
    if (harpoonedEnemy) {
        std::cout << "Player harpoon detached from enemy" << std::endl;
        harpoonedEnemy = nullptr;

        // Reset immobilization state
        isImmobilized = false;
        immobilizationTimer = 0.0f;

        // Stop shooting state
        isShooting = false;
        currentHarpoonLength = 0.0f;
    }
}
