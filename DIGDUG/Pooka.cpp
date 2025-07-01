#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include "Pooka.h"
#include "Player.h"

Pooka::Pooka(Map* gameMap, Player* player) : Entity(EntityType::POOKA, true, sf::Vector2i(16, 16)),
health(3), speed(15.0f), status(0), sprite(texture), pumpSound(pumpBuffer), map(gameMap), player(player) {
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
    sprite.setPosition({ 122, 144 });

    hitbox.setSize(sf::Vector2f(size.x * sprite.getScale().x, size.y * sprite.getScale().y));
    hitbox.setOrigin(sf::Vector2f(size.x / 2.0f, size.y / 2.0f));

    if (!pumpBuffer.loadFromFile("Assets/Sounds/SFX/pump.mp3")) {
        std::cout << "failed to load pump sound" << '\n';
    }
    pumpSound.setBuffer(pumpBuffer);

    std::cout << "pooka loaded successfully" << '\n';
    animation = std::make_unique<Animation>(&texture, sf::Vector2u(2, 2), 0.25f, size.x, size.y);
}

void Pooka::Update(float deltaTime, sf::Vector2f playerPosition) {
    if (health <= 0 || !isAlive) return;

    // Handle pump state deflation
    if (harpoonStuck && pumpState >= 0) {
        pumpTimer += deltaTime;
        if (pumpTimer >= PUMP_DURATION) {
            // Deflate one level
            pumpState = std::max(0, pumpState - 1);
            if (health < 3) {
                health += 1;
            }
            pumpTimer = 0.0f;
            std::cout << "Pooka deflated to state: " << pumpState << std::endl;
            std::cout << "Pooka health regen to: " << health << std::endl;

            // If fully deflated, detach harpoon and update sprite (fixes the hitbox)
            if (pumpState == 0) {
                updateInflationSprite();
                DetachHarpoon();
            }
        }
    }


    // Handle movement when not harpooned or pumped
    if (pumpCooldownTimer > 0.0f) {
        pumpCooldownTimer -= deltaTime;
    }

    if (health > 0 && isAlive && pumpState == 0 && !harpoonStuck) {
        sf::Vector2f currentPosition = sprite.getPosition();
        movementTimer += deltaTime;
        stuckTimer += deltaTime;

        if (!isMoving) {
            sf::Vector2f newTarget = targetPosition;

            if (movementTimer >= movementDelay) {
                movementTimer = 0.0f;
                movementDelay = 0.3f + static_cast<float>(rand()) / RAND_MAX * 0.7f;

                sf::Vector2f directionToPlayer = playerPosition - currentPosition;
                bool foundValidMove = false;

                if (status == 1) {
                    // Ghost mode - move directly toward player
                    if (abs(directionToPlayer.y) >= abs(directionToPlayer.x)) {
                        if (directionToPlayer.y < 0) {
                            newTarget.y -= TILE_SIZE;
                            sprite.setScale(sf::Vector2f(1, 1));
                        }
                        else {
                            newTarget.y += TILE_SIZE;
                            sprite.setScale(sf::Vector2f(1, 1));
                        }
                    }
                    else {
                        if (directionToPlayer.x < 0) {
                            newTarget.x -= TILE_SIZE;
                            sprite.setScale(sf::Vector2f(-1, 1));
                        }
                        else {
                            newTarget.x += TILE_SIZE;
                            sprite.setScale(sf::Vector2f(1, 1));
                        }
                    }
                    foundValidMove = true;
                }
                else {
                    // pathfinding logic
                    int currentTileType = map->getTileAt(currentPosition.x, currentPosition.y);
                    int playerTileType = map->getTileAt(playerPosition.x, playerPosition.y);

                    bool canSeePlayer = (currentTileType == 0 && playerTileType == 0);

                    if (canSeePlayer) {
                        // Pathfind toward player
                        if (abs(directionToPlayer.y) > TILE_SIZE / 2) {
                            if (directionToPlayer.y < 0) {
                                newTarget.y -= TILE_SIZE;
                                if (canMoveTo(newTarget)) {
                                    sprite.setScale(sf::Vector2f(1, 1));
                                    foundValidMove = true;
                                }
                            }
                            else {
                                newTarget.y += TILE_SIZE;
                                if (canMoveTo(newTarget)) {
                                    sprite.setScale(sf::Vector2f(1, 1));
                                    foundValidMove = true;
                                }
                            }
                        }

                        if (!foundValidMove && abs(directionToPlayer.x) > TILE_SIZE / 2) {
                            newTarget = targetPosition;
                            if (directionToPlayer.x < 0) {
                                newTarget.x -= TILE_SIZE;
                                if (canMoveTo(newTarget)) {
                                    sprite.setScale(sf::Vector2f(-1, 1));
                                    foundValidMove = true;
                                }
                            }
                            else {
                                newTarget.x += TILE_SIZE;
                                if (canMoveTo(newTarget)) {
                                    sprite.setScale(sf::Vector2f(1, 1));
                                    foundValidMove = true;
                                }
                            }
                        }

                        if (!foundValidMove && stuckTimer >= ghostModeDelay) {
                            sf::Vector2f bestMove = targetPosition;
                            if (abs(directionToPlayer.y) >= abs(directionToPlayer.x)) {
                                bestMove.y += (directionToPlayer.y < 0) ? -TILE_SIZE : TILE_SIZE;
                            }
                            else {
                                bestMove.x += (directionToPlayer.x < 0) ? -TILE_SIZE : TILE_SIZE;
                            }

                            if (!canMoveTo(bestMove)) {
                                status = 1;
                                stuckTimer = 0.0f;
                                foundValidMove = true;
                            }
                        }
                    }

                    // Random movement if no valid pathfinding move
                    if (!foundValidMove) {
                        newTarget = targetPosition;
                        if (rand() % 2 == 0) {
                            newTarget.y -= TILE_SIZE;
                            if (canMoveTo(newTarget)) {
                                sprite.setScale(sf::Vector2f(1, 1));
                                foundValidMove = true;
                            }
                            else {
                                newTarget.y = targetPosition.y + TILE_SIZE;
                                if (canMoveTo(newTarget)) {
                                    sprite.setScale(sf::Vector2f(1, 1));
                                    foundValidMove = true;
                                }
                            }
                        }
                        else {
                            newTarget.y += TILE_SIZE;
                            if (canMoveTo(newTarget)) {
                                sprite.setScale(sf::Vector2f(1, 1));
                                foundValidMove = true;
                            }
                            else {
                                newTarget.y = targetPosition.y - TILE_SIZE;
                                if (canMoveTo(newTarget)) {
                                    sprite.setScale(sf::Vector2f(1, 1));
                                    foundValidMove = true;
                                }
                            }
                        }
                    }
                }

                if (foundValidMove && newTarget != targetPosition) {
                    targetPosition = newTarget;
                    isMoving = true;
                }
            }
        }

        // Movement logic
        if (isMoving) {
            sf::Vector2f direction = targetPosition - currentPosition;
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (distance < 0.1f) {
                sprite.setPosition(targetPosition);
                isMoving = false;

                if (status == 1) {
                    int tileType = map->getTileAt(targetPosition.x, targetPosition.y);
                    if (tileType == 0) {
                        status = 0;
                    }
                }
            }
            else {
                direction.x /= distance;
                direction.y /= distance;
                float moveDistance = speed * deltaTime;
                if (moveDistance >= distance) {
                    sprite.setPosition(targetPosition);
                    isMoving = false;

                    if (status == 1) {
                        int tileType = map->getTileAt(targetPosition.x, targetPosition.y);
                        if (tileType == 0) {
                            status = 0;
                        }
                    }
                }
                else {
                    currentPosition.x += direction.x * moveDistance;
                    currentPosition.y += direction.y * moveDistance;
                    sprite.setPosition(currentPosition);
                }
            }
        }
        if (isMoving) {
            if (status == 1) {
                animation->Update(1, deltaTime, sprite);
                hitbox.setSize(sf::Vector2f(0, 0));
            }
            else {
                animation->Update(0, deltaTime, sprite);
                hitbox.setSize(sf::Vector2f(size.x, size.y));
            }
        }

        hitbox.setPosition(sprite.getPosition());
    }
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
    if (harpoonStuck && pumpCooldownTimer <= 0.0f && pumpState < 3) {
        pumpState++;
        pumpTimer = 0.0f;
        pumpCooldownTimer = PUMP_COOLDOWN;
        pumpSound.play();
        updateInflationSprite();
        std::cout << "Pooka inflated to state: " << pumpState << std::endl;
        if (pumpState >= 3) {
            health = 0;
            isAlive = false;
            DetachHarpoon();
            std::cout << "Pooka destroyed!" << std::endl;
        }
    }
    else if (pumpCooldownTimer > 0.0f) {
        std::cout << "Pump on cooldown: " << pumpCooldownTimer << "s remaining" << std::endl;
    }
}

void Pooka::updateInflationSprite() {
    switch (pumpState) {
    case 0:
        sprite.setScale(sf::Vector2f(1.0f, 1.0f));
        break;
    case 1:
        sprite.setScale(sf::Vector2f(1.2f, 1.2f));
        break;
    case 2:
        sprite.setScale(sf::Vector2f(1.4f, 1.4f));
        break;
    case 3:
        sprite.setScale(sf::Vector2f(1.6f, 1.6f));
        break;
    }
    sf::Vector2f newScale = sprite.getScale();
    hitbox.setSize(sf::Vector2f(size.x * newScale.x, size.y * newScale.y));
    hitbox.setOrigin(sf::Vector2f((size.x * newScale.x) / 2.0f, (size.y * newScale.y) / 2.0f));
    std::cout << "Pooka sprite updated for pump state: " << pumpState << std::endl;
}

bool Pooka::isHarpoonAttached() const {
    return harpoonStuck;
}

void Pooka::setPosition(sf::Vector2f pos) {
    // Snap to grid for tile-based movement
    pos.x = ((int)pos.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
    pos.y = ((int)pos.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;

    targetPosition = pos;
    sprite.setPosition(pos);
    hitbox.setPosition(pos);
    isMoving = false;
}

void Pooka::Draw(sf::RenderWindow& window) {
    if (isAlive && health > 0) {
        window.draw(sprite);
        window.draw(hitbox);
    }
}