#include "Scoreboard.h"
#include "Player.h"        
#include "StageManager.h"  
#include <sstream>
#include <iostream>
#include <algorithm>
#include <iomanip>

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
    timerText(font),
    playerSprite(playerTexture),
    levelTimeLimit(60.0f),
    timerRunning(false),
    timerExpired(false),
    timerPaused(false)
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
    timerText(font),
    playerSprite(playerTexture),
    levelTimeLimit(60.0f),
    timerRunning(false),
    timerExpired(false)
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

    // Configure timer text
    timerText.setFont(font);
    timerText.setCharacterSize(fontSize);
    timerText.setFillColor(textColor);
    timerText.setPosition(sf::Vector2f(position.x + padding + 150, position.y + padding + 15));
}

void Scoreboard::setupBackground() {
    background.setFillColor(backgroundColor);
    background.setPosition(position);
    background.setSize(sf::Vector2f(500.f, 50.f)); // Increased width to accommodate timer
}

void Scoreboard::update(const Player& player, const StageManager& stageManager) {
    std::stringstream scoreStream;
    scoreStream << "SCORE " << player.getScore();
    scoreText.setString(scoreStream.str());

    std::stringstream levelStream;
    levelStream << "LEVEL " << stageManager.getCurrentStage();
    levelText.setString(levelStream.str());

    // Update timer
    updateTimer();

    // Update lifes
    updateLifeSprites(player.getLives());
}

void Scoreboard::updateTimer() {
    float remaining = levelTimeLimit;

    if (timerRunning) {
        if (timerPaused) {
            // When paused, use the saved pausedTime
            remaining = levelTimeLimit - pausedTime.asSeconds();
        }
        else {
            // When not paused, get current elapsed time
            float elapsed = levelClock.getElapsedTime().asSeconds();
            remaining = levelTimeLimit - elapsed;
        }

        if (remaining <= 0.0f) {
            remaining = 0.0f;
            timerExpired = true;
            timerRunning = false;
        }
    }
    else {
        remaining = 0.0f; // Display 0 when timer is not running
    }

    int seconds = static_cast<int>(remaining);

    std::stringstream timerStream;
    timerStream << "TIME " << std::setw(2) << std::setfill('0') << seconds;

    // Change color when time is running low (last 10 seconds)
    if (timerRunning && remaining <= 10.0f && remaining > 0.0f) {
        timerText.setFillColor(sf::Color::Red);
    }
    else {
        timerText.setFillColor(textColor);
    }

    timerText.setString(timerStream.str());
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
    window.draw(timerText);

    // Draw life sprites
    for (const auto& lifeSprite : lifeSprites) {
        window.draw(lifeSprite);
    }
}

// Timer methods
void Scoreboard::startTimer(float timeLimitSeconds) {
    levelTimeLimit = timeLimitSeconds;
    levelClock.restart();
    timerRunning = true;
    timerExpired = false;
    timerPaused = false;
}

void Scoreboard::stopTimer() {
    timerRunning = false;
    timerPaused = false;
}

void Scoreboard::resetTimer() {
    levelClock.restart();
    timerRunning = false;
    timerExpired = false;
    timerPaused = false;
}

void Scoreboard::pauseTimer() {
    if (timerRunning && !timerPaused) {
        pausedTime = levelClock.getElapsedTime();
        timerPaused = true;
    }
}

void Scoreboard::resumeTimer() {
    if (timerRunning && timerPaused) {
        levelClock.restart();
        timerPaused = false;

        // Adjust the time limit to account for time already elapsed
        levelTimeLimit = levelTimeLimit - pausedTime.asSeconds();
    }
}

float Scoreboard::getRemainingTime() const {
    if (!timerRunning || timerPaused) return timerPaused ? (levelTimeLimit - pausedTime.asSeconds()) : 0.0f;

    float elapsed = levelClock.getElapsedTime().asSeconds();
    float remaining = levelTimeLimit - elapsed;
    return remaining > 0.0f ? remaining : 0.0f;
}

float Scoreboard::getElapsedTime() const {
    if (timerPaused) return pausedTime.asSeconds();
    return levelClock.getElapsedTime().asSeconds();
}

bool Scoreboard::isTimerExpired() const {
    return timerExpired;
}

bool Scoreboard::isTimerRunning() const {
    return timerRunning && !timerPaused;
}

void Scoreboard::setTimerLimit(float seconds) {
    levelTimeLimit = seconds;
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
    timerText.setFillColor(color);
}

void Scoreboard::setBackgroundColor(sf::Color color) {
    backgroundColor = color;
    background.setFillColor(color);
}

void Scoreboard::setFontSize(unsigned int size) {
    fontSize = size;
    scoreText.setCharacterSize(size);
    levelText.setCharacterSize(size);
    timerText.setCharacterSize(size);
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
        timerText.setFont(font);
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