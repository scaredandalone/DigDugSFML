
#include "Entity.h"
#include <iostream>

Entity::Entity(EntityType t, bool alive, sf::Sprite& spr) : type(t), isAlive(alive), sprite(texture)
{

}

Entity::~Entity()
{
    std::cout << "Entity id: " << static_cast<int>(type) << " has been destroyed..." << '\n';
}

void Entity::Update(float deltaTime, sf::Vector2f playerPosition)
{
    // Base implementation - override in derived classes
}

void Entity::handleCollision(std::shared_ptr<Entity> other)
{
    // Base implementation - override in derived classes
}

void Entity::AttachHarpoon() {
    // Base implementation - override in derived classes
}

void Entity::DetachHarpoon() {
    // Base implementation - override in derived classes
}

void Entity::Inflate() {
    // Base implementation - override in derived classes
}

bool Entity::isHarpoonAttached() const {
    return false; // Base implementation - override in derived classes
}

void Entity::updateInflationSprite(){

}

void Entity::Draw(sf::RenderWindow& window)
{
    // Base implementation - override in derived classes
}


sf::FloatRect Entity::getBounds() const {
    return sprite.getGlobalBounds();
}