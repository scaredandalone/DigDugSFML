#pragma once
#include <SFML/Graphics.hpp>
#include <memory>

enum class EntityType {PLAYER, POOKA};
class Entity
{
protected:
	// drawing stuff for sprite
	sf::Vector2f position;
	sf::RectangleShape hitbox;
	sf::Vector2i size;



	EntityType type;

	bool isAlive = true;

public:
	Entity(EntityType t, bool isAlive);
	virtual ~Entity();
	virtual void Update(float deltaTime, sf::Vector2f playerPosition);
	virtual void Draw(sf::RenderWindow& window);
	virtual void handleCollision(std::shared_ptr<Entity> other);

	EntityType getType() const { return type; }
	bool isActive() const { return isAlive; }
	sf::RectangleShape getHitbox() const { return hitbox; }

	// entity damage 
	virtual void AttachHarpoon();
	virtual void DetachHarpoon();
	virtual void Inflate();
	virtual bool isHarpoonAttached() const;

};

