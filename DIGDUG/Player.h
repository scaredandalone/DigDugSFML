#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Entity.h"
#include "Map.h"
#include "Animation.h"
#include "EnemyManager.h"

class GameState;

class EnemyManager;

class Player : public Entity {
private:
    // map and stats
    Map* map;
    int health;
    int lives;
    int score;
    float speed;

    // sprite and hitbox
    sf::RectangleShape hitbox;
    sf::Sprite sprite;
    sf::Texture texture;
    sf::Vector2i size;

    //movement stuff
    sf::Music movementMusic;
    sf::Vector2f targetPosition;
    bool isMoving;
    const int TILE_SIZE = 16;

    bool canMoveTo(sf::Vector2f position);
    void createTunnel(sf::Vector2f position);
    bool createTunnels = true; 


    // animation
    std::unique_ptr<Animation> animation;


    // shooting enemy and harpoonn
    sf::SoundBuffer buffer;
    std::optional<sf::Sound> harpoonSFX;
    EnemyManager* enemyManager = nullptr;
    std::shared_ptr<Entity> harpoonedEnemy = nullptr;
    bool isShooting;
    sf::Vector2f shootDirection;
    sf::Vector2f lastDirection;
    sf::Vector2f harpoonStartPos;
    float harpoonSpeed;
    float maxHarpoonLength;
    float currentHarpoonLength;

    sf::Texture harpoonTexture;
    sf::Sprite harpoonSprite;
    sf::RectangleShape harpoonHitbox;
    bool spaceKeyPressed = false;

    bool isImmobilized = false;
    float immobilizationTimer = 0.0f;
    const float IMMOBILIZATION_DURATION = 0.25f;
    void startShooting();
    void updateShooting(float deltaTime);
    void stopShooting();

    GameState* gameState = nullptr;

public:
    // main
    Player(Map* gameMap);
    void Initialise();
    void Update(float deltaTime);
    void Load();
    void Draw(sf::RenderWindow& window);
    void shoot();
    void DetachHarpoon();

    // helper functions
    void SetEnemyManager(EnemyManager* manager) { enemyManager = manager; }
    sf::Vector2f getPlayerPosition() { return sprite.getPosition(); }
    sf::FloatRect getHarpoonBounds() const;
    bool isCurrentlyShooting() const;
    int getHealth() const { return health; }
    int getScore() const { return score; }
    bool getPlayerMoving() const {return isMoving;}

    // scoreboard (will be added later)
    void addScore(int points) { score += points; }


    // game state
    void SetGameState(GameState* state) { gameState = state; }
    void SetCreateTunnels(bool enable) { createTunnels = enable; }
    void SetPosition(sf::Vector2f pos) { sprite.setPosition(pos); hitbox.setPosition(pos); }
    void SetTargetPosition(sf::Vector2f target) { targetPosition = target; isMoving = true; }
    void UpdateAtStart();
};