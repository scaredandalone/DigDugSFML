#pragma once
#include "Entity.h"
#include <SFML/Audio.hpp>

class Player;
class Map;

class Fygar : public Entity {
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

    // Fire breath mechanics
    bool preparingShot = false;
    float prepareTimer = 0.0f;
    const float PREPARE_DURATION = 0.5f; // Time to prepare before shooting
    float shootCooldown = 0.0f;
    const float SHOOT_COOLDOWN = 3.0f; // Cooldown between shots
    bool fireBreathActive = false;
    float fireBreathTimer = 0.0f;
    const float FIRE_BREATH_GROW_TIME = 0.4f; // Time to grow to full length
    const float FIRE_BREATH_HOLD_TIME = 0.5f; // Time to hold at full length
    const float FIRE_BREATH_TOTAL_TIME = FIRE_BREATH_GROW_TIME + FIRE_BREATH_HOLD_TIME;
    float fireBreathDirection = 1.0f; // 1 = right, -1 = left
    sf::Sprite fireBreathSprite;
    sf::Texture fireBreathTexture;
    const float FIRE_BREATH_MAX_LENGTH = TILE_SIZE * 3; // 3 tiles long
    sf::RectangleShape fireHitbox;
    SFX fireBreathSound;


    // Shooting helper methods
    bool shouldPrepareShot(sf::Vector2f playerPosition);
    bool hasHorizontalTunnelPath(sf::Vector2f from, sf::Vector2f to);
    void breatheFire(sf::Vector2f playerPosition);
    void updateFireBreathState(float deltaTime);

    // Movement helper methods
    bool canMoveTo(sf::Vector2f position);
    bool canMove() const;
    bool isGridAligned() const;
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
    sf::Vector2f findClosestTunnelToFygar(sf::Vector2f playerPosition);

    // Animation and hitbox methods
    void updateAnimationAndHitbox(float deltaTime);
    void updateNormalHitbox();

    // Pump state methods
    void updatePumpState(float deltaTime);
    void updatePumpCooldown(float deltaTime);
    void handleDeflation();
    void regenerateHealth();

public:
    Fygar(Map* gameMap, Player* player);
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

    sf::FloatRect getFireBreathBounds() const {
        if (fireBreathActive) {
            // Calculate bounds based on current fire breath state
            sf::Vector2f fygarPos = sprite.getPosition();
            float currentLength = FIRE_BREATH_MAX_LENGTH;

            if (fireBreathDirection > 0) {
                return sf::FloatRect({ fygarPos.x, fygarPos.y - 8 }, { currentLength, 16 });
            }
            else {
                return sf::FloatRect({ fygarPos.x - currentLength, fygarPos.y - 8 }, { currentLength, 16 });
            }
        }
        return sf::FloatRect({ 0, 0 }, { 0, 0 });
    }

    bool isFireBreathActive() const { return fireBreathActive; }
    void deactivateFireBreath() { fireBreathActive = false; }

    int getHealth() const { return health; }
    int getPumpState() const { return pumpState; }
};