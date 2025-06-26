#pragma once
#include <SFML/Graphics.hpp>
#include "Entity.h"

class Player : public Entity {
private:
    int health;
    int score;
    float speed; 
    sf::RectangleShape hitbox;
    sf::Sprite sprite;
    sf::Texture texture;

    sf::Vector2i size;

public:
    Player();
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