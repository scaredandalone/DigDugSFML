
#include <iostream>
#include <cmath>

#include "Player.h"

Player::Player(Map* gameMap) : Entity(EntityType::PLAYER, true), health(3), lives(2), score(0), speed(35.0f), sprite(texture), size(sf::Vector2f(16, 16)),
isMoving(false), targetPosition(0, 0), map(gameMap) {
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
    if (!texture.loadFromFile("Assets/Sprites/Player/spritesheet.png")) {
        std::cout << "failed to load player sprite" << '\n';
    }

    if (!movementMusic.openFromFile("Assets/Sounds/Music/moving1.mp3")) {
        std::cout << "failed to load movement music" << '\n';
    }
    movementMusic.setLooping(true); 
    movementMusic.setVolume(50); 

    sprite.setTextureRect(sf::IntRect({ 0 * size.x ,0 * size.y }, { size.x,size.y }));
    // snap pos to grid
    sf::Vector2f initialPos(122, 144);

    initialPos.x = ((int)initialPos.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
    initialPos.y = ((int)initialPos.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
    targetPosition = sf::Vector2f(initialPos.x, initialPos.y);

    sprite.setPosition(initialPos);
    sprite.setOrigin(sf::Vector2f(size.x / 2.0f, size.y / 2.0f));
    sprite.scale(sf::Vector2f(1, 1));

    hitbox.setSize(sf::Vector2f(size.x * sprite.getScale().x, size.y * sprite.getScale().y));
    hitbox.setOrigin(sf::Vector2f(size.x / 2.0f, size.y / 2.0f));



    std::cout << "player loaded successfully" << '\n';
    animation = std::make_unique<Animation>(&texture, sf::Vector2u(4,1), 0.25f, size.x, size.y);
}

void Player::Update(float deltaTime) {
    if (health > 0) {
        sf::Vector2f currentPosition = sprite.getPosition();
        bool wasMoving = isMoving;
        int animRow = 0;


        if (!isMoving) {
            sf::Vector2f newTarget = targetPosition;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
                newTarget.x -= TILE_SIZE;
                sprite.setScale(sf::Vector2f(1, 1));
                sprite.setRotation(sf::degrees(0));

            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
                newTarget.x += TILE_SIZE;
                sprite.setScale(sf::Vector2f(-1, 1));
                sprite.setRotation(sf::degrees(0));

            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
                newTarget.y -= TILE_SIZE;
                sprite.setScale(sf::Vector2f(1, -1));
                sprite.setRotation(sf::degrees(90));
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
                newTarget.y += TILE_SIZE;
                sprite.setRotation(sf::degrees(90));
                sprite.setScale(sf::Vector2f(-1, -1));
            }

            // new target is within map bounds and not a solid wall
            if (newTarget != targetPosition && canMoveTo(newTarget)) {
                targetPosition = newTarget;
                isMoving = true;
            }
        }
        // for smooth movement to target
        if (isMoving) {
            sf::Vector2f direction = targetPosition - currentPosition;
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            animation->Update(animRow, deltaTime, sprite);
            if (distance < 0.1f) {
                //  snap to position
                sprite.setPosition(targetPosition);
                isMoving = false;

                // create tunnels -----> FUCK YOU
                createTunnel(targetPosition);
            }
            else {
       
                direction.x /= distance;
                direction.y /= distance;
                float moveDistance = speed * deltaTime;
                if (moveDistance >= distance) {
                    sprite.setPosition(targetPosition);
                    isMoving = false;
                    createTunnel(targetPosition);
                }
                else {
                    currentPosition.x += direction.x * moveDistance;
                    currentPosition.y += direction.y * moveDistance;
                    sprite.setPosition(currentPosition);
                   
                }
            }
        }
        if (isMoving && !wasMoving) {
            std::cout << "Started moving - playing music" << std::endl;
            if (movementMusic.getStatus() != sf::SoundSource::Status::Playing) {
                movementMusic.play();
            }
        }
        else if (!isMoving && wasMoving) {
            std::cout << "Stopped moving - pausing music" << std::endl;
            if (movementMusic.getStatus() == sf::SoundSource::Status::Playing) {
                movementMusic.pause();
            }
        }
        hitbox.setPosition(sprite.getPosition());
    }
}

bool Player::canMoveTo(sf::Vector2f position) {
    if (map == nullptr) return false;

    // if position is within map bounds
    sf::Vector2i mapSize = map->getMapSize();
    if (position.x < 0 || position.x >= mapSize.x ||
        position.y < 0 || position.y >= mapSize.y) {
        return false;
    }
    int tileType = map->getTileAt(position.x, position.y);

   // define tiles that player can walk through
    return (tileType == 0 || tileType == 2 || tileType == 3 || tileType == 4);
}

void Player::createTunnel(sf::Vector2f position) {
    if (map != nullptr) {
        int tileType = map->getTileAt(position.x, position.y);

        // specify diggable tile types here:
        if (tileType == 2 || tileType == 3 || tileType == 4) { 
            map->setTileAt(position.x, position.y, 0); //(creates tunnel)

            if (tileType == 2) score += 10;  // this is score for each tiletype
            if (tileType == 3) score += 20;  // this is score for each tiletype  
            if (tileType == 4) score += 5;   // this is score for each tiletype

            std::cout << "Tunnel created! Score: " << score << std::endl;
        }
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