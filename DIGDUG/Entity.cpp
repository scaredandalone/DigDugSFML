#include "Entity.h"
#include <iostream>

Entity::Entity(EntityType t, bool isAlive)
{
	isAlive = true;
	type = t;
}

Entity::~Entity()
{
	std::cout << "Entity id: " << static_cast<int>(type) << " has been destroyed..." << '\n';
}

void Entity::Update(float deltaTime)
{
}

void Entity::handleCollision(std::shared_ptr<Entity> other)
{
}

void Entity::Draw(sf::RenderWindow& window)
{
}
