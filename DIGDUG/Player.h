#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Entity.h"
#include "Map.h"
#include "Animation.h"
#include "EnemyManager.h"

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

    // animation
    std::unique_ptr<Animation> animation;

    // shooting enemy and harpoonn
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

    void startShooting();
    void updateShooting(float deltaTime);
    void stopShooting();


public:
    Player(Map* gameMap);
    void Initialise();
    void Update(float deltaTime); 
    void Load();
    void Draw(sf::RenderWindow& window);
    void shoot(); 
    int getHealth() const { return health; }
    int getScore() const { return score; }
    void addScore(int points) { score += points; }


    void SetEnemyManager(EnemyManager *manager) { enemyManager = manager; }

    sf::Vector2f getPlayerPosition() { return sprite.getPosition(); }
    sf::FloatRect getHarpoonBounds() const;
    bool isCurrentlyShooting() const;
};