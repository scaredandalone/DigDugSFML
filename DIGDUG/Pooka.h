#pragma once
#include "Entity.h"
#include "Animation.h"
#include "Map.h"
#include <SFML/Audio.hpp>

class Player;
class Pooka : public Entity
{
private:
    // map + stats
    Player* player;
    Map* map;
    int health;
    float speed;
    int status; // 0 as default, 1 as ghost form.

    // sprite 
    sf::RectangleShape hitbox;
    sf::Sprite sprite;
    sf::Texture texture;
    sf::Vector2i size;

    // movement
    sf::Vector2f targetPosition;
    bool isMoving;
    float movementTimer = 0.0f;
    float movementDelay = 1.0f;
    float stuckTimer = 0.0f;
    float ghostModeDelay = 5.0f + static_cast<float>(rand()) / RAND_MAX * 5.0f;
    const int TILE_SIZE = 16;

    bool canMoveTo(sf::Vector2f position);
    std::unique_ptr<Animation> animation;

    // take damage and inflate
    bool harpoonStuck = false;

    //pump stuff
    sf::SoundBuffer pumpBuffer;
    sf::Sound pumpSound;
    int pumpState = 0; // 0 = normal, 1 = first pump, 2 = second pump, 3 = dead
    float pumpTimer = 0.0f;
    const float PUMP_DURATION = 1.0f; // Time before deflating
    float pumpCooldownTimer = 0.0f;
    const float PUMP_COOLDOWN = 0.1f; // Minimum time between pumps

public:
    Pooka(Map* gameMap, Player* player);
    void Initialise();
    void Update(float deltaTime, sf::Vector2f playerPosition) override;
    void Load();
    void Draw(sf::RenderWindow& window) override;

    // take damage and inflate
    void AttachHarpoon() override;
    void DetachHarpoon() override;
    void Inflate() override;
    bool isHarpoonAttached() const override;
    void updateInflationSprite() override;

    // helper
    int getHealth() const { return health; }
    sf::FloatRect getBounds() const override;

};