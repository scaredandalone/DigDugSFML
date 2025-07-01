#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Entity.h"
#include "Map.h"
#include "Animation.h"
#include "EnemyManager.h"
#include "SFX.h"

class GameState;
class EnemyManager;

class Player : public Entity {
private:
    Map* map;
    int health;
    int lives;
    int score;
    float speed;

    sf::Sprite sprite;
    sf::Texture texture;
    // sound + start
    sf::Vector2f initialPos;
    SFX MovementMusic;
    SFX harpoonSound;

    // enemy manager
    EnemyManager* enemyManager = nullptr;
    std::shared_ptr<Entity> harpoonedEnemy = nullptr;

    //shooting
    bool isShooting;
    sf::Vector2f shootDirection;
    sf::Vector2f lastDirection;
    sf::Vector2f harpoonStartPos;

    // harpoon stuff
    float harpoonSpeed;
    float maxHarpoonLength;
    float currentHarpoonLength;
    sf::Texture harpoonTexture;
    sf::Sprite harpoonSprite;
    sf::RectangleShape harpoonHitbox;
    bool spaceKeyPressed = false;
    // immobolisation
    bool isImmobilized = false;
    float immobilizationTimer = 0.0f;
    const float IMMOBILIZATION_DURATION = 0.25f;
    bool createTunnels = true;
    //gameplay
    void startShooting();
    void updateShooting(float deltaTime);
    void stopShooting();
    void createTunnel(sf::Vector2f position);
    // gamestate
    GameState* gameState = nullptr;
    void updateStartState(float deltaTime, sf::Vector2f playerPosition);
    void updateGameState(float deltaTime, sf::Vector2f playerPosition);
    void updateWinState(float deltaTime, sf::Vector2f playerPosition);

public:
    Player(Map* gameMap);
    void Initialise() override;
    void Load() override;
    void Update(float deltaTime, sf::Vector2f playerPosition) override;
    void Draw(sf::RenderWindow& window) override;
    void shoot();
    void DetachHarpoon() override;
    void setPosition(sf::Vector2f pos) override; // Add override for setPosition

    void SetEnemyManager(EnemyManager* manager) { enemyManager = manager; }
    sf::Vector2f getPlayerPosition() { return sprite.getPosition(); }
    void setPlayerInitialPosition(sf::Vector2f initialpos) {
        initialPos.x = ((int)initialpos.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
        initialPos.y = ((int)initialpos.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
    }
    sf::FloatRect getHarpoonBounds() const;
    bool isCurrentlyShooting() const;
    int getHealth() const { return health; }
    int getScore() const { return score; }

    void addScore(int points) { score += points; }
    void SetGameState(GameState* state) { gameState = state; }
    void SetCreateTunnels(bool enable) { createTunnels = enable; }
    void setIsMoving(bool state) { isMoving = state; }



};