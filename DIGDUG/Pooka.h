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
    bool harpoonStuck = false;
    sf::SoundBuffer pumpBuffer;
    sf::Sound pumpSound;
    int pumpState = 0;
    float pumpTimer = 0.0f;
    const int MAX_PUMP_STATE = 4;
    const float PUMP_DURATION = 1.0f;
    float pumpCooldownTimer = 0.0f;
    const float PUMP_COOLDOWN = 0.1f;

    // Movement helper methods
    bool canMoveTo(sf::Vector2f position);
    bool canMove() const;
    void updateMovement(float deltaTime, sf::Vector2f playerPosition);
    void updateMovementTimer(float deltaTime, sf::Vector2f playerPosition);
    void performMovement(float deltaTime, sf::Vector2f currentPosition);

    // Target calculation methods
    sf::Vector2f calculateNextTarget(sf::Vector2f playerPosition);
    sf::Vector2f calculateGhostModeTarget();
    sf::Vector2f calculateNormalModeTarget(sf::Vector2f playerPosition);
    sf::Vector2f chooseBestTarget(sf::Vector2f horizontal, sf::Vector2f vertical, sf::Vector2f playerPos);

    // Directional movement methods
    sf::Vector2f moveHorizontallyToward(sf::Vector2f direction);
    sf::Vector2f moveVerticallyToward(sf::Vector2f direction);
    bool tryMoveHorizontally(sf::Vector2f direction, sf::Vector2f& target);
    bool tryMoveVertically(sf::Vector2f direction, sf::Vector2f& target);

    // Stuck state handling
    sf::Vector2f handleStuckState(sf::Vector2f playerPosition);
    sf::Vector2f tryPatrolMove();
    std::vector<sf::Vector2f> findValidTunnelMoves();
    void enterGhostMode(sf::Vector2f playerPosition);

    // Movement execution methods
    void arriveAtTarget();
    void moveTowardTarget(sf::Vector2f direction, float distance, float deltaTime);
    void checkExitGhostMode();

    // Utility methods
    void updateSpriteDirection(sf::Vector2f newTarget);
    float calculateDistance(sf::Vector2f from, sf::Vector2f to);
    sf::Vector2f findClosestTunnelToPooka(sf::Vector2f playerPosition);

    // Animation and hitbox methods
    void updateAnimationAndHitbox(float deltaTime);
    void updateNormalHitbox();

    // Pump state methods
    void updatePumpState(float deltaTime);
    void updatePumpCooldown(float deltaTime);
    void handleDeflation();
    void regenerateHealth();

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