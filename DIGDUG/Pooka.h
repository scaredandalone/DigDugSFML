#pragma once
#include "Entity.h"
#include <SFML/Audio.hpp>

class Player;
class Map;

class Pooka : public Entity {
private:
    Player* player;
    Map* map;
    int health;
    float speed;
    int status; // 0 = default, 1 = ghost form

    sf::Sprite sprite;
    sf::Texture texture;

    sf::Vector2f initialPos;
    float movementTimer = 0.0f;
    float movementDelay = 0.5f;
    float stuckTimer = 0.0f;
    float ghostModeDelay = 2.0f + static_cast<float>(rand()) / RAND_MAX * 5.0f;
    sf::Vector2f ghostTarget;
    sf::Vector2f findNearestTunnelToPlayer(sf::Vector2f playerPosition);

    bool harpoonStuck = false;
    sf::SoundBuffer pumpBuffer;
    sf::Sound pumpSound;
    int pumpState = 0; // 0 = normal, 1 = first pump, 2 = second pump, 3 = third pump, 4 = DEAD 
    float pumpTimer = 0.0f;
    const int MAX_PUMP_STATE = 4;
    const float PUMP_DURATION = 1.0f;
    float pumpCooldownTimer = 0.0f;
    const float PUMP_COOLDOWN = 0.1f;

private:
    bool canMoveTo(sf::Vector2f position);

public:
    Pooka(Map* gameMap, Player* player);
    void Initialise() override;
    void Load() override;
    void Update(float deltaTime, sf::Vector2f playerPosition) override;
    void Draw(sf::RenderWindow& window) override;

    void AttachHarpoon() override;
    void DetachHarpoon() override;
    void Inflate() override;
    bool isHarpoonAttached() const override;
    void updateInflationSprite() override;
    void setPosition(sf::Vector2f pos) override;
    void multiplySpeed(float multiple) override;

    bool getInflationStatus() override {
        return pumpState > 0;
    }

    sf::FloatRect getBounds() const {
        return hitbox.getGlobalBounds();
    }

    int getHealth() const { return health; }
    int getPumpState() const { return pumpState; }
};