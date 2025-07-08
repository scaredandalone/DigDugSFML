#pragma once
#include "Entity.h"
#include "Map.h"
#include "Player.h"

class EnemyManager;

class Rock : public Entity {
private:
    Map* map;
    EnemyManager* enemyManager;
    Player* player;

    sf::Sprite tileSprite;        // For the underlying tile
    sf::Texture tileTexture;      // Tilesheet texture
    sf::Sprite rockSprite;        // For the rock overlay
    sf::Texture rockTexture;      // Rock-specific texture

    SFX rockFall;
    SFX rockKill;
    SFX rockHitGround;

    float shakeTimer;
    bool isShaking;

    sf::Vector2i initialTileTypeSource; // Grid coordinates of the tile to the right for texture
    int tileTypeTextureIndex; // The texture index based on the tile to its right

    bool isFalling;
    float fallTimer;

    const float FALL_DELAY = 1.0f; // Time before rock starts falling
    const float FALL_SPEED = 75.0f; // Pixels per second
    const float DESTROY_ANIMATION_DURATION = 0.6f; // How long the destruction effect is visible
    const float SHAKE_AMPLITUDE = 1.0f; // Pixels of horizontal shake
    const float SHAKE_SPEED_MULTIPLIER = 15.0f;

    bool hasFallen;

    bool destroyAnimationStarted;
    bool destroyAnimationComplete;
    float destroyTimer; // To track destruction animation time

    bool markedForDeletion; // Flag for deletion

    void updateFalling(float deltaTime);
    void checkAndSquashEntities();
    void startDestroyAnimation(float deltaTime);

public:
    Rock(Map* gameMap, EnemyManager* em, Player* p, sf::Vector2f pos, sf::Vector2i tileTypeSourceGrid);
    void Initialise() override;
    void Load() override;
    void Update(float deltaTime, sf::Vector2f playerPosition) override;
    void Draw(sf::RenderWindow& window) override;
    void setPosition(sf::Vector2f pos) override;

    bool isSolid(float x, float y); // To check if it hits a solid tile

    void setInitialTileTypeSource(sf::Vector2i source) { initialTileTypeSource = source; }
    void setTextureIndex(int index) { tileTypeTextureIndex = index; }
    bool getDestroyAnimationComplete() const { return destroyAnimationComplete; }
    bool isMarkedForDeletion() const { return markedForDeletion; }
    void startDestroyAnimation() const { startDestroyAnimation(); }
};