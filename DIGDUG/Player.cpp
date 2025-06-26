#include "Player.h"
#include <iostream>
#include <cmath>

Player::Player(Map* gameMap) : Entity(EntityType::PLAYER, true), health(3), lives(2), score(0), speed(43.0f), sprite(texture), size(sf::Vector2f(16, 16)),
isMoving(false), targetPosition(0, 0), map(gameMap) {
}

void Player::Initialise()
{
    hitbox.setSize(sf::Vector2f(size));
    hitbox.setFillColor(sf::Color::Transparent);
    hitbox.setOutlineColor(sf::Color::Red);
    hitbox.setOutlineThickness(1);
}

void Player::Load()
{
    if (!texture.loadFromFile("Assets/Sprites/Player/player.png")) {
        std::cout << "failed to load player sprite" << '\n';
    }
    sprite.setTextureRect(sf::IntRect({ 0 * size.x ,0 * size.y }, { size.x,size.y }));
    // snap pos to grid
    sf::Vector2f initialPos(122, 144);
    initialPos.x = ((int)initialPos.x / TILE_SIZE) * TILE_SIZE;
    initialPos.y = ((int)initialPos.y / TILE_SIZE) * TILE_SIZE;
    sprite.setPosition(initialPos);
    targetPosition = sf::Vector2f(initialPos.x, initialPos.y);
    sprite.scale(sf::Vector2f(1, 1));
    hitbox.setSize(sf::Vector2f(size.x * sprite.getScale().x, size.y * sprite.getScale().y));
    std::cout << "player loaded successfully" << '\n';
}

void Player::Update(float deltaTime) {
    if (health > 0) {
        sf::Vector2f currentPosition = sprite.getPosition();
        if (!isMoving) {
            sf::Vector2f newTarget = targetPosition;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
                newTarget.x -= TILE_SIZE;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
                newTarget.x += TILE_SIZE;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
                newTarget.y -= TILE_SIZE;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
                newTarget.y += TILE_SIZE;
            }

            // new target is within map bounds and not a solid wall
            if (newTarget != targetPosition && canMoveTo(newTarget)) {
                targetPosition = newTarget;
                isMoving = true;
            }
        }
        // for smooth movement to target
        if (isMoving) {
            sf::Vector2f direction = targetPosition - currentPosition;
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
            if (distance < 0.1f) {
                //  snap to position
                sprite.setPosition(targetPosition);
                isMoving = false;

                // create tunnels ----- FUCK YOU
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
        hitbox.setPosition(sprite.getPosition());
    }
}

bool Player::canMoveTo(sf::Vector2f position) {
    if (map == nullptr) return false;

    // if position is within map bounds
    sf::Vector2i mapSize = map->getMapSize();
    if (position.x < 0 || position.x >= mapSize.x ||
        position.y < 0 || position.y >= mapSize.y) {
        return false;
    }

    // Get tile type at target position
    int tileType = map->getTileAt(position.x, position.y);

    // Can move to empty spaces (0) or diggable tiles (2=dirt, 3=grass, 4=water)
    // Cannot move through walls (1) or out of bounds (-1)
    return (tileType == 0 || tileType == 2 || tileType == 3 || tileType == 4);
}

void Player::createTunnel(sf::Vector2f position) {
    if (map != nullptr) {
        int tileType = map->getTileAt(position.x, position.y);

        // specify diggable tile types here:
        if (tileType == 2 || tileType == 3 || tileType == 4) { 
            map->setTileAt(position.x, position.y, 0); //(creates tunnel)

            // score for different tiles
            if (tileType == 2) score += 10;  // Dirt gives 10 points
            if (tileType == 3) score += 20;  // Grass gives 20 points  
            if (tileType == 4) score += 5;   // Water gives 5 points

            std::cout << "Tunnel created! Score: " << score << std::endl;
        }
    }
}

void Player::Draw(sf::RenderWindow& window) {
    window.draw(sprite);
    window.draw(hitbox);
}

//void Player::handleCollision(std::shared_ptr<Entity> other) {
    // collision logic
//}

void Player::shoot() {
    //  will trigger enemy inflation later
}