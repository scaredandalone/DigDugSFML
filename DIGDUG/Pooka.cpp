
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include "Pooka.h"
#include "Player.h"

Pooka::Pooka(Map* gameMap, Player* player) : Entity(EntityType::POOKA, true, sf::Vector2i(16, 16), 300),
health(4), speed(25.0f), status(0), sprite(texture), pumpSound(pumpBuffer), map(gameMap), player(player) {

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
    sprite.setPosition({ 0, 0 }); // default pos


    if (!pumpBuffer.loadFromFile("Assets/Sounds/SFX/pump.mp3")) {
        std::cout << "failed to load pump sound" << '\n';
    }
    pumpSound.setBuffer(pumpBuffer);

    std::cout << "pooka loaded successfully" << '\n';
    animation = std::make_unique<Animation>(&texture, sf::Vector2u(2, 2), 0.25f, size.x, size.y, true);
}

void Pooka::Update(float deltaTime, sf::Vector2f playerPosition) {
    if (health <= 0 || !isAlive) return;

    // Handle pump state deflation
    if (harpoonStuck) {
        pumpTimer += deltaTime;
        if (pumpTimer >= PUMP_DURATION) {
            if (pumpState > 0) { // Only deflate if pumpState is greater than 0
                pumpState = std::max(0, pumpState - 1); // Deflate one level
                std::cout << "Pooka deflated to state: " << pumpState << std::endl;

                if (pumpState == 0) {
                    if (health < 4) { // Assuming 4 is max health
                        health += 1;
                        std::cout << "Pooka health regen to: " << health << std::endl;
                    }
                    DetachHarpoon(); // Detach when fully deflated
                }
            }
            else {
                DetachHarpoon();
            }
            pumpTimer = 0.0f;
            updateInflationSprite();
        }
    }

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
                movementDelay = 0.0f;

                sf::Vector2f directionToPlayer = playerPosition - currentPosition;
                bool foundValidMove = false;

                if (status == 1) {
                    // Ghost mode - move directly toward ghostTarget (set when entering ghost mode)
                    sf::Vector2f directionToGhost = ghostTarget - currentPosition;

                    // Choose the axis with the larger distance to move along
                    if (abs(directionToGhost.x) >= abs(directionToGhost.y)) {
                        // Move horizontally first
                        if (directionToGhost.x < 0) {
                            newTarget.x -= TILE_SIZE;
                            sprite.setScale(sf::Vector2f(-1, 1));
                        }
                        else if (directionToGhost.x > 0) {
                            newTarget.x += TILE_SIZE;
                            sprite.setScale(sf::Vector2f(1, 1));
                        }
                    }
                    else {
                        // Move vertically
                        if (directionToGhost.y < 0) {
                            newTarget.y -= TILE_SIZE;
                            sprite.setScale(sf::Vector2f(1, 1));
                        }
                        else if (directionToGhost.y > 0) {
                            newTarget.y += TILE_SIZE;
                            sprite.setScale(sf::Vector2f(1, 1));
                        }
                    }
                    foundValidMove = true;
                }
                else {
                    // Normal mode - pathfinding logic
                    // Pooka ALWAYS sees the player and tries to move toward them

                    // Simple greedy pathfinding - always move closer to player
                    sf::Vector2f horizontalTarget = targetPosition;
                    sf::Vector2f verticalTarget = targetPosition;

                    bool canMoveHorizontal = false;
                    bool canMoveVertical = false;

                    // Check horizontal movement
                    if (abs(directionToPlayer.x) > 1.0f) { // Small threshold to avoid jittering
                        if (directionToPlayer.x < 0) {
                            horizontalTarget.x -= TILE_SIZE;
                            sprite.setScale(sf::Vector2f(-1, 1)); // Face left
                        }
                        else {
                            horizontalTarget.x += TILE_SIZE;
                            sprite.setScale(sf::Vector2f(1, 1)); // Face right
                        }
                        canMoveHorizontal = canMoveTo(horizontalTarget);
                    }

                    // Check vertical movement
                    if (abs(directionToPlayer.y) > 1.0f) { // Small threshold to avoid jittering
                        if (directionToPlayer.y < 0) {
                            verticalTarget.y -= TILE_SIZE;
                        }
                        else {
                            verticalTarget.y += TILE_SIZE;
                        }
                        canMoveVertical = canMoveTo(verticalTarget);
                    }

                    // Choose the move that gets us closer to the player
                    if (canMoveHorizontal && canMoveVertical) {
                        // Calculate distances for both options
                        float horizontalDist = std::sqrt(
                            (horizontalTarget.x - playerPosition.x) * (horizontalTarget.x - playerPosition.x) +
                            (horizontalTarget.y - playerPosition.y) * (horizontalTarget.y - playerPosition.y)
                        );
                        float verticalDist = std::sqrt(
                            (verticalTarget.x - playerPosition.x) * (verticalTarget.x - playerPosition.x) +
                            (verticalTarget.y - playerPosition.y) * (verticalTarget.y - playerPosition.y)
                        );

                        if (horizontalDist < verticalDist) {
                            newTarget = horizontalTarget;
                        }
                        else {
                            newTarget = verticalTarget;
                        }
                        foundValidMove = true;
                    }
                    else if (canMoveHorizontal) {
                        newTarget = horizontalTarget;
                        foundValidMove = true;
                    }
                    else if (canMoveVertical) {
                        newTarget = verticalTarget;
                        foundValidMove = true;
                    }

                    // If no valid move toward player, patrol the tunnel
                    if (!foundValidMove && stuckTimer < ghostModeDelay) {
                        // Check all four directions for valid tunnel tiles
                        std::vector<sf::Vector2f> possibleMoves;
                        sf::Vector2f moves[4] = {
                            sf::Vector2f(targetPosition.x + TILE_SIZE, targetPosition.y), // Right
                            sf::Vector2f(targetPosition.x - TILE_SIZE, targetPosition.y), // Left
                            sf::Vector2f(targetPosition.x, targetPosition.y + TILE_SIZE), // Down
                            sf::Vector2f(targetPosition.x, targetPosition.y - TILE_SIZE)  // Up
                        };

                        for (const auto& move : moves) {
                            if (canMoveTo(move) && map->getTileAt(move.x, move.y) == 0) {
                                possibleMoves.push_back(move);
                            }
                        }

                        // Choose a random valid tunnel move
                        if (!possibleMoves.empty()) {
                            int randomIndex = rand() % possibleMoves.size();
                            newTarget = possibleMoves[randomIndex];
                            foundValidMove = true;

                            // Update sprite facing based on movement direction
                            if (newTarget.x < targetPosition.x) {
                                sprite.setScale(sf::Vector2f(-1, 1)); // Face left
                            }
                            else if (newTarget.x > targetPosition.x) {
                                sprite.setScale(sf::Vector2f(1, 1)); // Face right
                            }
                        }
                    }

                    // If still stuck after trying both directions and patrol, enter ghost mode
                    if (!foundValidMove && stuckTimer >= ghostModeDelay) {
                        // Set ghost target to the closest tunnel to Pooka that connects to player
                        ghostTarget = findClosestTunnelToPooka(playerPosition);
                        status = 1; // Enter ghost mode
                        stuckTimer = 0.0f;
                        foundValidMove = true;
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

                // Check if we should exit ghost mode
                if (status == 1) {
                    int tileType = map->getTileAt(targetPosition.x, targetPosition.y);
                    if (tileType == 0) { // Back in normal tunnel
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

                    // Check if we should exit ghost mode
                    if (status == 1) {
                        int tileType = map->getTileAt(targetPosition.x, targetPosition.y);
                        if (tileType == 0) { // Back in normal tunnel
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

        // Update animation and hitbox
        if (isMoving) {
            if (status == 1) {
                animation->Update(1, deltaTime * (speed / 20), sprite);
                hitbox.setSize(sf::Vector2f(0, 0));
            }
            else {
                animation->Update(0, deltaTime * (speed / 20), sprite);
                hitbox.setSize(sf::Vector2f(10.f, 10.f));
                sf::Vector2f spriteCenter = sprite.getPosition() + sf::Vector2f(size.x / 2.f, size.y / 2.f);
                hitbox.setOrigin(hitbox.getSize() / 2.f);
                hitbox.setPosition(spriteCenter);
            }
        }

        hitbox.setPosition(sprite.getPosition());
    }
}

sf::Vector2f Pooka::findClosestTunnelToPooka(sf::Vector2f playerPosition) {
    sf::Vector2f bestTunnel = playerPosition;
    float shortestDistanceToPooka = std::numeric_limits<float>::max();
    sf::Vector2f currentPos = sprite.getPosition();

    // First, find tunnels near the player (expanding search around player)
    const int maxSearchDistance = 8;
    std::vector<sf::Vector2f> playerTunnels;

    for (int searchDistance = 1; searchDistance <= maxSearchDistance; searchDistance++) {
        // Check all tiles at this distance from player
        for (int x = -searchDistance; x <= searchDistance; x++) {
            for (int y = -searchDistance; y <= searchDistance; y++) {
                // Only check tiles on the edge of the current ring
                if (abs(x) != searchDistance && abs(y) != searchDistance) continue;

                sf::Vector2f checkPos = playerPosition + sf::Vector2f(x * TILE_SIZE, y * TILE_SIZE);

                // Check if this position is a valid tunnel
                int tileType = map->getTileAt(checkPos.x, checkPos.y);
                if (tileType == 0) {
                    playerTunnels.push_back(checkPos);
                }
            }
        }

        // If we found some tunnels near the player, that's enough
        if (!playerTunnels.empty()) {
            break;
        }
    }

    // If no tunnels found near player, just use player position
    if (playerTunnels.empty()) {
        playerTunnels.push_back(playerPosition);
    }

    for (const sf::Vector2f& tunnel : playerTunnels) {
        float distance = std::sqrt(
            (tunnel.x - currentPos.x) * (tunnel.x - currentPos.x) +
            (tunnel.y - currentPos.y) * (tunnel.y - currentPos.y)
        );

        if (distance > TILE_SIZE && distance < shortestDistanceToPooka) { // Make sure it's not the same tunnel Pooka is in
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

            // Only kill the Pooka when fully inflated
            if (pumpState >= MAX_PUMP_STATE) {
                isAlive = false; // This will trigger scoring in EnemyManager
                std::cout << "Pooka POPPED!" << std::endl;
                DetachHarpoon();
            }
        }
    }
}

void Pooka::updateInflationSprite() {
    switch (pumpState) {
    case 0:
        sprite.setScale(sf::Vector2f(1.0f, 1.0f));
        // Restore normal hitbox size
        break;
    case 1:
        sprite.setScale(sf::Vector2f(1.2f, 1.2f));
        // Disable hitbox

        break;
    case 2:
        sprite.setScale(sf::Vector2f(1.4f, 1.4f));
        // Disable hitbox
     
        break;
    case 3:
        sprite.setScale(sf::Vector2f(1.6f, 1.6f));
        // Disable hitbox
   
        break;
    case 4:
        sprite.setScale(sf::Vector2f(1.8f, 1.6f));
        // Disable hitbox
        break;
}
    sf::Vector2f newScale = sprite.getScale();

    // Update hitbox origin to match sprite position
    hitbox.setSize(sf::Vector2f(size.x - 6, size.y - 6));
    hitbox.setOrigin(sf::Vector2f(hitbox.getSize().x / 2.0f, hitbox.getSize().y / 2.0f));

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

void Pooka::multiplySpeed(float multiple)
{

}

void Pooka::Draw(sf::RenderWindow& window) {
    if (isAlive && health > 0) {
        window.draw(sprite);
        window.draw(hitbox);
    }
}

