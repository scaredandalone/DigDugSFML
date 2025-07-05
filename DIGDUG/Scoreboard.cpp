#include "Scoreboard.h"
#include "Player.h"        
#include "StageManager.h"  
#include <sstream>
#include <iostream>
#include <algorithm>

Scoreboard::Scoreboard()
    : position(10.f, 10.f),
    textColor(sf::Color::White),
    backgroundColor(sf::Color(0, 0, 0, 128)),
    fontSize(12),
    padding(10.f),
    spriteScale(0.5f),
    spriteSpacing(5.f),
    scoreText(font),
    levelText(font),
    playerSprite(playerTexture)
{
    if (!font.openFromFile("Assets/Fonts/digdugfont.otf")) {
        std::cerr << "Warning: Could not load font from digdugfont" << std::endl;
    }

    setupTexts();
    setupBackground();
}

// Constructor with custom position
Scoreboard::Scoreboard(sf::Vector2f pos, unsigned int fontSz)
    : position(pos),
    textColor(sf::Color::White),
    backgroundColor(sf::Color(0, 0, 0, 0)),
    fontSize(fontSz),
    padding(5.0f),
    spriteScale(0.75f),
    spriteSpacing(5.f),
    scoreText(font),
    levelText(font),
    playerSprite(playerTexture)
{
    if (!font.openFromFile("Assets/Fonts/digdugfont.otf")) {
        std::cerr << "Warning: Could not load font digdugfont" << std::endl;
    }

    setupTexts();
    setupBackground();
}

void Scoreboard::setupTexts() {
    // Configure score text
    scoreText.setFont(font);
    scoreText.setCharacterSize(fontSize);
    scoreText.setFillColor(textColor);
    scoreText.setPosition(sf::Vector2f(position.x + padding, position.y + padding));

    // Configure level text
    levelText.setFont(font);
    levelText.setCharacterSize(fontSize);
    levelText.setFillColor(textColor);
    levelText.setPosition(sf::Vector2f(position.x + padding + 150, position.y + padding));
}

void Scoreboard::setupBackground() {
    background.setFillColor(backgroundColor);
    background.setPosition(position);

    background.setSize(sf::Vector2f(400.f, 50.f));
}

void Scoreboard::update(const Player& player, const StageManager& stageManager) {
    std::stringstream scoreStream;
    scoreStream << "SCORE " << player.getScore();
    scoreText.setString(scoreStream.str());

    std::stringstream levelStream;
    levelStream << "LEVEL " << stageManager.getCurrentStage();
    levelText.setString(levelStream.str());

    // update lifes
    updateLifeSprites(player.getLives());
}

void Scoreboard::updateLifeSprites(int lives) {
    lifeSprites.clear();

    if (playerTexture.getSize().x == 0) {
        // No texture loaded, skip sprite creation
        return;
    }
    float spriteWidth = 16 * spriteScale; 
    float totalSpritesWidth = lives * spriteWidth + (lives - 1) * spriteSpacing;

    float spriteX = (224 - totalSpritesWidth) / 2; // Center horizontally
    float spriteY = 270 - 15.f; 

    // Create sprites based on number of lives
    for (int i = 0; i < lives; ++i) {
        sf::Sprite lifeSprite(playerTexture);

        // first sprite in spritesheet
        lifeSprite.setTextureRect(sf::IntRect({ 0, 0 }, { 16, 16 }));

        lifeSprite.setScale(sf::Vector2f(spriteScale, spriteScale));

        // Offset each sprite horizontally
        float currentX = spriteX + i * (spriteWidth + spriteSpacing);

        lifeSprite.setPosition(sf::Vector2f(currentX, spriteY));
        lifeSprites.push_back(lifeSprite);
    }
}

void Scoreboard::render(sf::RenderWindow& window) {
    window.draw(background);
    window.draw(scoreText);
    window.draw(levelText);

    // Draw life sprites
    for (const auto& lifeSprite : lifeSprites) {
        window.draw(lifeSprite);
    }
}

// Setters for customization
void Scoreboard::setPosition(sf::Vector2f pos) {
    position = pos;
    setupTexts();
    setupBackground();
}

void Scoreboard::setTextColor(sf::Color color) {
    textColor = color;
    scoreText.setFillColor(color);
    levelText.setFillColor(color);
}

void Scoreboard::setBackgroundColor(sf::Color color) {
    backgroundColor = color;
    background.setFillColor(color);
}

void Scoreboard::setFontSize(unsigned int size) {
    fontSize = size;
    scoreText.setCharacterSize(size);
    levelText.setCharacterSize(size);
    setupTexts(); 
}

void Scoreboard::setSpriteScale(float scale) {
    spriteScale = scale;
}

void Scoreboard::setSpriteSpacing(float spacing) {
    spriteSpacing = spacing;
}


bool Scoreboard::loadFont(const std::string& fontPath) {
    if (font.openFromFile(fontPath)) {
        scoreText.setFont(font);
        levelText.setFont(font);
        return true;
    }
    std::cerr << "Error: Could not load font from " << fontPath << std::endl;
    return false;
}

bool Scoreboard::loadPlayerTexture(const std::string& texturePath) {
    if (playerTexture.loadFromFile(texturePath)) {
        playerSprite.setTexture(playerTexture);
        return true;
    }
    std::cerr << "Error: Could not load player texture from " << texturePath << std::endl;
    return false;
}

void Scoreboard::setPlayerTexture(const sf::Texture& texture) {
    playerTexture = texture;
    playerSprite.setTexture(playerTexture);

}