#pragma once
#include "Entity.h"
#include "Animation.h"
#include "Map.h"
#include "Player.h"

class Pooka : public Entity
{
    // map + stats
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
    bool regenInProgress = false;
    float regenDelayTimer = 0.0f;
    float regenTimer = 0.0f;


public:
    Pooka(Map* gameMap);
    void Initialise();
    void Update(float deltaTime, sf::Vector2f playerPosition) override;
    void Load();
    void Draw(sf::RenderWindow& window);

    // take damage and inflate
    void AttachHarpoon() override;
    void DetachHarpoon() override;
    void Inflate() override;





    int getHealth() const { return health; }
};

