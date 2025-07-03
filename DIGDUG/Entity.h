#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "Animation.h"

using EntityID = uint32_t;
constexpr EntityID INVALID_ENTITY = 0;

enum class EntityType { PLAYER, POOKA, ROCK };

class Map;

class Entity {
protected:
    EntityType type;
    bool isAlive;
    sf::RectangleShape hitbox;
    sf::Vector2i size;
    bool isMoving;
    sf::Vector2f targetPosition;
    const int TILE_SIZE = 16;
    std::unique_ptr<Animation> animation;

    virtual bool canMoveTo(sf::Vector2f position, Map* map) const;
    void move(float deltaTime, float speed, sf::Sprite& sprite);

public:
    Entity(EntityType t, bool alive, sf::Vector2i size);
    virtual ~Entity();

    virtual void Initialise();
    virtual void Load() = 0;
    virtual void Update(float deltaTime, sf::Vector2f playerPosition) = 0;
    virtual void Draw(sf::RenderWindow& window) = 0;
    virtual void handleCollision(std::shared_ptr<Entity> other);
    virtual void AttachHarpoon();
    virtual void DetachHarpoon();
    virtual void Inflate();
    virtual bool isHarpoonAttached() const;
    virtual void updateInflationSprite();
    virtual void setPosition(sf::Vector2f pos);
    virtual void setTargetPosition(sf::Vector2f target);
    virtual bool getInflationStatus();

    sf::FloatRect getBounds() const;
    sf::Vector2f getPosition() const { return hitbox.getPosition(); }
    bool getIsMoving() const { return isMoving; }
    bool isActive() const { return isAlive; }
    void setActive(bool y) {isAlive = y; }

    static EntityID nextID;
    static EntityID CreateEntity() { return ++nextID; }
    static bool IsValid(EntityID entity) { return entity != INVALID_ENTITY; }
};