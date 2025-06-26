#pragma once
#include <SFML/Graphics.hpp>
#include "Entity.h"
#include "Map.h"

class Player : public Entity {
private:
    Map* map;
    int health;
    int lives;
    int score;
    float speed; 


    sf::RectangleShape hitbox;
    sf::Sprite sprite;
    sf::Texture texture;
    sf::Vector2i size;

    sf::Vector2f targetPosition;
    bool isMoving;
    const int TILE_SIZE = 16;

    bool canMoveTo(sf::Vector2f position);
    void createTunnel(sf::Vector2f position);

public:
    Player(Map* gameMap);
    void Initialise();
    void Update(float deltaTime) override; 
    void Load();
    void Draw(sf::RenderWindow& window);
    // void handleCollision(std::shared_ptr<Entity> other) override;
    void shoot(); 
    int getHealth() const { return health; }
    int getScore() const { return score; }
    void addScore(int points) { score += points; }
};