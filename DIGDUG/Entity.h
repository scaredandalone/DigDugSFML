#pragma once
#include <SFML/Graphics.hpp>
#include <memory>

enum class EntityType { PLAYER, POOKA, FYGAR };

class Entity
{
protected:
    // drawing stuff for sprite
    sf::Sprite sprite;
    sf::Texture texture;
    sf::Vector2f position;
    sf::RectangleShape hitbox;
    sf::Vector2f size; 
    EntityType type;

    bool isAlive = true;

public:
    // main
    Entity(EntityType t, bool alive, sf::Sprite& spr);
    virtual ~Entity();
    virtual void Update(float deltaTime, sf::Vector2f playerPosition);
    virtual void Draw(sf::RenderWindow& window);
    virtual void handleCollision(std::shared_ptr<Entity> other);

    // helper
    virtual sf::FloatRect getBounds() const;
    EntityType getType() const { return type; }
    bool isActive() const { return isAlive; }
    sf::RectangleShape getHitbox() const { return hitbox; }
    sf::Sprite getSprite() const { return sprite; }

    // entity damage 
    virtual void AttachHarpoon();
    virtual void DetachHarpoon();
    virtual void Inflate();
    virtual bool isHarpoonAttached() const;
    virtual void updateInflationSprite();
};