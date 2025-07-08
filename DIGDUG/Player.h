#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Entity.h"
#include "Map.h"
#include "Animation.h"
#include "SFX.h"

class GameState;
class EnemyManager;
class ScoreManager;


enum class Reason {
    DEFAULT,
    LAST_ENEMY,
    THIRTY_SECONDS,
    FIFTEEN_SECONDS
};

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

    SFX RareMovementMusic;
    SFX fastMovementMusic;
    SFX fasterMovementMusic;
    SFX popSound;
    SFX inflatingSound;

    bool isPlayingRareMusic;
    bool playFastMusic = false;
    bool playFasterMusic = false;
    static constexpr float RARE_MUSIC_CHANCE = 0.1f; // 10% chance of rare movement music after normal music ends.

    // enemy manager
    EnemyManager* enemyManager = nullptr;
    std::shared_ptr<Entity> harpoonedEnemy = nullptr;

    // Score manager - Add this
    ScoreManager* scoreManager= nullptr;

    //shooting
    bool isShooting;
    sf::Vector2f shootDirection;
    sf::Vector2f lastDirection;
    sf::Vector2f harpoonStartPos;

    // harpoon stuff
    float harpoonSpeed;
    float maxHarpoonLength;
    float currentHarpoonLength;
    float harpoonTimer;
    const float HARPOON_DURATION = 3.0f;
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

    sf::Vector2f facingDirection;
    void startShooting();
    void updateShooting(float deltaTime);
    void stopShooting();
    void createTunnel(sf::Vector2f position);

    void PopEnemyAndScore(std::shared_ptr<Entity> enemy);

    // gamestate
    GameState* gameState = nullptr;
    // death
    bool deathAnimationComplete = false;
    bool deathAnimationStarted = false;

    void startMovementMusic();
    void stopMovementMusic();
    void updateMovementMusic(float deltatime);

    bool shouldPlayMovementMusic;



public:
    Player(Map* gameMap);
    void Initialise() override;
    void Load() override;
    void Update(float deltaTime, sf::Vector2f playerPosition) override;
    void Draw(sf::RenderWindow& window) override;
    void shoot();
    void DetachHarpoon() override;
    void setPosition(sf::Vector2f pos) override;
    bool getInflationStatus() override { return 0; }

    // set managers
    void SetEnemyManager(EnemyManager* manager) { enemyManager = manager; }
    void SetScoreManager(ScoreManager* manager) { scoreManager = manager; }

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
    void clearScore() { score = 0; }
    void SetGameState(GameState* state) { gameState = state; }
    void SetCreateTunnels(bool enable) { createTunnels = enable; }
    void setIsMoving(bool state) { isMoving = state; }

    ScoreManager* getScoreManager() const { return scoreManager; }

    void updateStartState(float deltaTime, sf::Vector2f playerPosition);
    void updateGameState(float deltaTime, sf::Vector2f playerPosition);
    void updateWinState(float deltaTime, sf::Vector2f playerPosition);
    void updateLossState(float deltaTime, sf::Vector2f playerPosition);

    void resetTransform() {
        sprite.setRotation(sf::degrees(0));
        sprite.setScale({ 1, 1 });
    }
    void setHealth(int hp) { health = hp; };
    int getLives() const { return lives; }
    void setLives(int life) { lives = life; }
    void resetDeathAnimation();

    void setMovementMusicStatus(bool enabled) {
        shouldPlayMovementMusic = enabled;
    }
    bool isPlayingFastMusic() const { return playFastMusic; }
    bool isPlayingFasterMusic() const { return playFasterMusic; }
    void resetMusic(Reason reason);
    void resetMusicForNewLevel();

};