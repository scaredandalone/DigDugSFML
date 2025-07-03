#include "Entity.h"
#include "Map.h"
#include "SFX.h"
#include <iostream>
#include <cmath>

EntityID Entity::nextID = 0;

Entity::Entity(EntityType t, bool alive, sf::Vector2i size)
    : type(t), isAlive(alive), size(size), isMoving(false), targetPosition(0, 0) {
    hitbox.setSize(sf::Vector2f(size.x -6, size.y-6));
    hitbox.setFillColor(sf::Color::Transparent);
    hitbox.setOutlineColor(sf::Color::Red);
    hitbox.setOutlineThickness(1);
}

Entity::~Entity() {
    std::cout << "Entity id: " << static_cast<int>(type) << " has been destroyed..." << '\n';
}

void Entity::Initialise() {
    hitbox.setOrigin(sf::Vector2f((size.x-6) / 2.0f, (size.y-6) / 2.0f));
}

bool Entity::canMoveTo(sf::Vector2f position, Map* map) const {
    if (map == nullptr) return false;

    sf::Vector2i mapSize = map->getMapSize();
    if (position.x < 0 || position.x >= mapSize.x ||
        position.y < 0 || position.y >= mapSize.y) {
        return false;
    }
    int tileType = map->getTileAt(position.x, position.y);
    return (tileType == 0 || tileType == 2 || tileType == 3 || tileType == 4);
}

void Entity::move(float deltaTime, float speed, sf::Sprite& sprite) {
    if (!isMoving) return;

    sf::Vector2f currentPosition = sprite.getPosition();
    sf::Vector2f direction = targetPosition - currentPosition;
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (distance < 0.1f) {
        sprite.setPosition(targetPosition);
        hitbox.setPosition(targetPosition);
        isMoving = false;
    }
    else {
        direction.x /= distance;
        direction.y /= distance;
        float moveDistance = speed * deltaTime;
        if (moveDistance >= distance) {
            sprite.setPosition(targetPosition);
            hitbox.setPosition(targetPosition);
            isMoving = false;
        }
        else {
            currentPosition.x += direction.x * moveDistance;
            currentPosition.y += direction.y * moveDistance;
            sprite.setPosition(currentPosition);
            hitbox.setPosition(currentPosition);
        }
    }
}

void Entity::setPosition(sf::Vector2f pos) {
    pos.x = ((int)pos.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
    pos.y = ((int)pos.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;

    targetPosition = pos;
    hitbox.setPosition(pos);
}

void Entity::setTargetPosition(sf::Vector2f target) {
    targetPosition = target;
    isMoving = true;
}

sf::FloatRect Entity::getBounds() const {
    return hitbox.getGlobalBounds();
}

void Entity::handleCollision(std::shared_ptr<Entity> other) {
    // Base implementation
}

void Entity::AttachHarpoon() {
    // Base implementation
}

void Entity::DetachHarpoon() {
    // Base implementation
}

void Entity::Inflate() {
    // Base implementation
}

bool Entity::isHarpoonAttached() const {
    return false;
}

void Entity::updateInflationSprite() {
    // Base implementation
}
bool Entity::getInflationStatus() {
    return false;
}