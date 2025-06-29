#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include "Pooka.h"
#include "Player.h"

Pooka::Pooka(Map* gameMap, Player* player) : Entity(EntityType::POOKA, true, sprite),
health(3), speed(15.0f), size(16, 16),
isMoving(false), targetPosition(0, 0), map(gameMap), player(player), status(0), sprite(texture), pumpSound(pumpBuffer) {
    // Initialize sprite and texture properly
    sprite.setTexture(texture);
}

void Pooka::Initialise()
{
    hitbox.setSize(sf::Vector2f(size.x, size.y));
    hitbox.setFillColor(sf::Color::Transparent);
    hitbox.setOutlineColor(sf::Color::Red);
    hitbox.setOutlineThickness(1);
}

void Pooka::Load()
{
    if (!texture.loadFromFile("Assets/Sprites/Pooka/spritesheet.png")) {
        std::cout << "failed to load pooka sprite" << '\n';
    }

    sprite.setTexture(texture); // Make sure texture is set

    sprite.setTextureRect(sf::IntRect({ 0 * size.x ,0 * size.y }, { size.x,size.y }));
    // snap pos to grid
    sf::Vector2f initialPos(144, 144);
    initialPos.x = ((int)initialPos.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
    initialPos.y = ((int)initialPos.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
    targetPosition = sf::Vector2f(initialPos.x, initialPos.y);

    sprite.setPosition(initialPos);
    sprite.setOrigin(sf::Vector2f(size.x / 2.0f, size.y / 2.0f));
    sprite.setScale(sf::Vector2f(1, 1));

    hitbox.setSize(sf::Vector2f(size.x * sprite.getScale().x, size.y * sprite.getScale().y));
    hitbox.setOrigin(sf::Vector2f(size.x / 2.0f, size.y / 2.0f));

    std::cout << "pooka loaded successfully" << '\n';
    animation = std::make_unique<Animation>(&texture, sf::Vector2u(2, 2), 0.25f, size.x, size.y);
}

void Pooka::Update(float deltaTime, sf::Vector2f playerPosition) {
    // pumpState deflation
    if (harpoonStuck && pumpState >= 0) {
        pumpTimer += deltaTime;
        if (pumpTimer >= PUMP_DURATION) {
            // Deflate one level
            pumpState = std::max(0, pumpState - 1);
            int healthMax = 3;
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

    // pump cooldown - prevent rapid pumping
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


        // Notify player to detach harpoon
        if (player != nullptr) {
            player->DetachHarpoon();
        }

        std::cout << "Harpoon detached from Pooka" << std::endl;
    }
}

void Pooka::Inflate() {
    if (harpoonStuck && pumpCooldownTimer <= 0.0f && pumpState < 3) {
        pumpState++;
        pumpTimer = 0.0f; // Reset deflation timer
        pumpCooldownTimer = PUMP_COOLDOWN; // Set pump cooldown

        // Update sprite based on new pump state
        pumpSound.play();
        updateInflationSprite();

        std::cout << "Pooka inflated to state: " << pumpState << std::endl;


        if (pumpState >= 3) {
            health = 0;
            isAlive = false;
            // FIXED: Detach harpoon when enemy dies
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
       // sprite.setTextureRect(sf::IntRect(0 * size.x, 0 * size.y, size.x, size.y));
        sprite.setScale(sf::Vector2f(1.0f, 1.0f));
        break;
    case 1: 
        //sprite.setTextureRect(sf::IntRect(1 * size.x, 0 * size.y, size.x, size.y));
        sprite.setScale(sf::Vector2f(1.2f, 1.2f)); // 20% bigger
        break;
    case 2: 
       // sprite.setTextureRect(sf::IntRect(2 * size.x, 0 * size.y, size.x, size.y));
        sprite.setScale(sf::Vector2f(1.4f, 1.4f)); // 40% bigger
        break;
    case 3: 
        //sprite.setTextureRect(sf::IntRect(3 * size.x, 0 * size.y, size.x, size.y));
        sprite.setScale(sf::Vector2f(1.6f, 1.6f)); // 60% bigger
        break;
    }

    // Update hitbox to match new sprite size
    sf::Vector2f newScale = sprite.getScale();
    hitbox.setSize(sf::Vector2f(size.x * newScale.x, size.y * newScale.y));
    hitbox.setOrigin(sf::Vector2f((size.x * newScale.x) / 2.0f, (size.y * newScale.y) / 2.0f));

    std::cout << "Pooka sprite updated for pump state: " << pumpState << std::endl;
}

bool Pooka::isHarpoonAttached() const {
    return harpoonStuck;
}

// Override getBounds to return proper sprite bounds
sf::FloatRect Pooka::getBounds() const {
    return hitbox.getGlobalBounds();
}

void Pooka::Draw(sf::RenderWindow& window) {
    // Only draw if the Pooka is alive
    if (isAlive && health > 0) {
        window.draw(sprite);
        window.draw(hitbox);
    }
}