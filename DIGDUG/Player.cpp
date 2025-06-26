#include "Player.h"
#include <iostream>

Player::Player() : Entity(EntityType::PLAYER, true), health(3), score(0), speed(9.6f), sprite(texture), size(sf::Vector2f(16,16)) {
}

void Player::Initialise()
{
    hitbox.setSize(sf::Vector2f(size));
    hitbox.setFillColor(sf::Color::Transparent);
    hitbox.setOutlineColor(sf::Color::Red);
    hitbox.setOutlineThickness(1);
}
void Player::Load()
{
    if (!texture.loadFromFile("Assets/Sprites/Player/player.png")) {
        std::cout << "failed to load player sprite" << '\n';
    }

    sprite.setTextureRect(sf::IntRect({ 0 * size.x ,0 * size.y}, { size.x,size.y}));
    sprite.setOrigin(sf::Vector2f(size.x / 2.f, size.y / 2.f));
    sprite.setPosition(sf::Vector2f(122, 144));

    sprite.scale(sf::Vector2f(1, 1));
    hitbox.setSize(sf::Vector2f(size.x * sprite.getScale().x, size.y * sprite.getScale().y));
    hitbox.setOrigin(sf::Vector2f(hitbox.getSize().x / 2.f, hitbox.getSize().y / 2.f));
    std::cout << "player loaded successfully" << '\n';
}


void Player::Update(float deltaTime) {
    if (health > 0) {

        sf::Vector2f prev_position = sprite.getPosition();
        sf::Vector2f movement(0.f, 0.f);
        // simple movement
        const float tileSize = 16.0f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
            movement.x = -speed;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
            movement.x = speed;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
            movement.y = -speed;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
            movement.y = speed;
        }
        hitbox.setPosition(sprite.getPosition());   
        sprite.setPosition(prev_position + movement * speed * deltaTime);

    }
}

void Player::Draw(sf::RenderWindow& window) {
    window.draw(sprite);
    window.draw(hitbox);
}

//void Player::handleCollision(std::shared_ptr<Entity> other) {
    // collision logic
//}

void Player::shoot() {
    //  will trigger enemy inflation later
}