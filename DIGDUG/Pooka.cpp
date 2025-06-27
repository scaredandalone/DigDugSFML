
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>

#include "Pooka.h"

Pooka::Pooka (Map* gameMap) : Entity(EntityType::POOKA, true), health(3), speed(35.0f), sprite(texture), size(sf::Vector2f(16, 16)),
isMoving(false), targetPosition(0, 0), map(gameMap), status(0)  {

}

void Pooka::Initialise()
{
    hitbox.setSize(sf::Vector2f(size));
    hitbox.setFillColor(sf::Color::Transparent);
    hitbox.setOutlineColor(sf::Color::Red);
    hitbox.setOutlineThickness(1);
}

void Pooka::Load()
{
    if (!texture.loadFromFile("Assets/Sprites/Pooka/spritesheet.png")) {
        std::cout << "failed to load player sprite" << '\n';
    }

    sprite.setTextureRect(sf::IntRect({ 0 * size.x ,0 * size.y }, { size.x,size.y }));
    // snap pos to grid
    sf::Vector2f initialPos(144, 144);

    initialPos.x = ((int)initialPos.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
    initialPos.y = ((int)initialPos.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
    targetPosition = sf::Vector2f(initialPos.x, initialPos.y);

    sprite.setPosition(initialPos);
    sprite.setOrigin(sf::Vector2f(size.x / 2.0f, size.y / 2.0f));
    sprite.scale(sf::Vector2f(1, 1));

    hitbox.setSize(sf::Vector2f(size.x * sprite.getScale().x, size.y * sprite.getScale().y));
    hitbox.setOrigin(sf::Vector2f(size.x / 2.0f, size.y / 2.0f));



    std::cout << "pooka loaded successfully" << '\n';
    animation = std::make_unique<Animation>(&texture, sf::Vector2u(2,1), 0.25f, size.x, size.y);
}

void Pooka::Update(float deltaTime, sf::Vector2f playerPosition) {
    if (harpoonStuck) {
        isMoving = false;
    }
    else if (regenInProgress) {
        regenDelayTimer += deltaTime;
        if (regenDelayTimer >= 3.0f) {
            regenTimer += deltaTime;
            if (regenTimer >= 1.0f) {
                regenTimer = 0.0f;
                if (health < 3) health++;
            }
        }
    }
    if (health == 3) {
        sf::Vector2f currentPosition = sprite.getPosition();
        movementTimer += deltaTime;
        stuckTimer += deltaTime; // Always increment stuck timer
        int animRow = 0;
        

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
                    // Normal mode
                    int currentTileType = map->getTileAt(currentPosition.x, currentPosition.y);
                    int playerTileType = map->getTileAt(playerPosition.x, playerPosition.y);

                    // Check if both Pooka and player are in tunnels (can "see" each other)
                    bool canSeePlayer = (currentTileType == 0 && playerTileType == 0);

                    if (canSeePlayer) {
                        // In tunnel and can see player - pathfind toward player
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

                        // If stuck timer is over and best move toward player is blocked, go ghost
                        if (!foundValidMove && stuckTimer >= ghostModeDelay) {
                            // Check if direct path to player is blocked
                            sf::Vector2f bestMove = targetPosition;
                            if (abs(directionToPlayer.y) >= abs(directionToPlayer.x)) {
                                bestMove.y += (directionToPlayer.y < 0) ? -TILE_SIZE : TILE_SIZE;
                            }
                            else {
                                bestMove.x += (directionToPlayer.x < 0) ? -TILE_SIZE : TILE_SIZE;
                            }

                            if (!canMoveTo(bestMove)) {
                                status = 1; // Enter ghost mode
                                stuckTimer = 0.0f;
                                foundValidMove = true; // Will handle movement next frame
                            }
                        }
                    }

                    // If can't see player or no valid move found, do random up/down movement
                    if (!foundValidMove) {
                        newTarget = targetPosition;

                        // Random up/down movement
                        if (rand() % 2 == 0) {
                            // Try up
                            newTarget.y -= TILE_SIZE;
                            if (canMoveTo(newTarget)) {
                                sprite.setScale(sf::Vector2f(1, 1));
                                foundValidMove = true;
                            }
                            else {
                                // Try down if up fails
                                newTarget.y = targetPosition.y + TILE_SIZE;
                                if (canMoveTo(newTarget)) {
                                    sprite.setScale(sf::Vector2f(1, 1));
                                    foundValidMove = true;
                                }
                            }
                        }
                        else {
                            // Try down
                            newTarget.y += TILE_SIZE;
                            if (canMoveTo(newTarget)) {
                                sprite.setScale(sf::Vector2f(1,1));
                                foundValidMove = true;
                            }
                            else {
                                // Try up if down fails
                                newTarget.y = targetPosition.y - TILE_SIZE;
                                if (canMoveTo(newTarget)) {
                                    sprite.setScale(sf::Vector2f(1, 1));
                                    foundValidMove = true;
                                }
                            }
                        }
                    }
                }

                // Start moving if we found a valid target
                if (foundValidMove && newTarget != targetPosition) {
                    targetPosition = newTarget;
                    isMoving = true;
                }
            }
        }

        if (isMoving) {
            sf::Vector2f direction = targetPosition - currentPosition;
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            animation->Update(animRow, deltaTime, sprite);
            if (distance < 0.1f) {
                sprite.setPosition(targetPosition);
                isMoving = false;

                // Exit ghost mode when reaching a tunnel
                if (status == 1) {
                    int tileType = map->getTileAt(targetPosition.x, targetPosition.y);
                    if (tileType == 0) {
                        status = 0; // Exit ghost mode
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

                    // Exit ghost mode when reaching a tunnel
                    if (status == 1) {
                        int tileType = map->getTileAt(targetPosition.x, targetPosition.y);
                        if (tileType == 0) {
                            status = 0; // Exit ghost mode
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

        hitbox.setPosition(sprite.getPosition());
    }
}

bool Pooka::canMoveTo(sf::Vector2f position) {
    if (map == nullptr) return false;

    // if position is within map bounds
    sf::Vector2i mapSize = map->getMapSize();
    if (position.x < 0 || position.x >= mapSize.x ||
        position.y < 0 || position.y >= mapSize.y) {
        return false;
    }
    int tileType = map->getTileAt(position.x, position.y);

    if(status == 0)
        return (tileType == 0);
    if(status == 1)
        return (tileType == 0 || tileType == 2 || tileType == 3 || tileType == 4); // tiles that pooka as ghost can move through
}



void Pooka::AttachHarpoon() {
    harpoonStuck = true;
    regenDelayTimer = 0.0f;
    regenInProgress = false;
}

void Pooka::DetachHarpoon() {
    harpoonStuck = false;
    regenDelayTimer = 0.0f;
    regenInProgress = true;
    regenTimer = 0.0f;
}

void Pooka::Inflate() {
    if (health > 0) {
        health--;
        if (health == 0) {
            isAlive = false; // kill....
        }
    }
}

bool Entity::isHarpoonAttached() const
{
    return false;
}

void Pooka::Draw(sf::RenderWindow& window) {
    window.draw(sprite);
    window.draw(hitbox);
}
