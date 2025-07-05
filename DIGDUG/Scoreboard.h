#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <sstream>
#include <vector>

class Player;
class StageManager;

class Scoreboard
{
private:
    sf::Font font;
    sf::Text scoreText;
    sf::Text levelText;
    sf::RectangleShape background;

    sf::Texture playerTexture;
    sf::Sprite playerSprite;
    std::vector<sf::Sprite> lifeSprites;

    sf::Vector2f position;
    sf::Color textColor;
    sf::Color backgroundColor;
    unsigned int fontSize;
    float padding;
    float spriteScale;
    float spriteSpacing;

    void setupTexts();
    void setupBackground();
    void updateLifeSprites(int lives);

public:
    Scoreboard();
    Scoreboard(sf::Vector2f pos, unsigned int fontSz = 12);

    // Main update and render methods
    void update(const Player& player, const StageManager& stageManager);
    void render(sf::RenderWindow& window);

    // Setters for customization
    void setPosition(sf::Vector2f pos);
    void setTextColor(sf::Color color);
    void setBackgroundColor(sf::Color color);
    void setFontSize(unsigned int size);
    void setSpriteScale(float scale);
    void setSpriteSpacing(float spacing);

    // Asset loading methods
    bool loadFont(const std::string& fontPath);
    bool loadPlayerTexture(const std::string& texturePath);
    void setPlayerTexture(const sf::Texture& texture);
};