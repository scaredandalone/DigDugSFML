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
    sf::Text timerText;  // New timer text

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

    // Timer variables
    sf::Clock levelClock;
    sf::Time pausedTime;
    float levelTimeLimit;      // Time limit for the level (in seconds)
    bool timerRunning;
    bool timerExpired;
    bool timerPaused;

    void setupTexts();
    void setupBackground();
    void updateLifeSprites(int lives);
    void updateTimer();        // New timer update method

public:
    Scoreboard();
    Scoreboard(sf::Vector2f pos, unsigned int fontSz = 12);

    // Main update and render methods
    void update(const Player& player, const StageManager& stageManager);
    void render(sf::RenderWindow& window);

    // Timer methods
    void startTimer(float timeLimitSeconds = 60.0f);
    void stopTimer();
    void resetTimer();
    void pauseTimer();
    void resumeTimer();
    float getRemainingTime() const;
    float getElapsedTime() const;
    bool isTimerExpired() const;
    bool isTimerRunning() const;

    // Setters for customization
    void setPosition(sf::Vector2f pos);
    void setTextColor(sf::Color color);
    void setBackgroundColor(sf::Color color);
    void setFontSize(unsigned int size);
    void setSpriteScale(float scale);
    void setSpriteSpacing(float spacing);
    void setTimerLimit(float seconds);

    // Asset loading methods
    bool loadFont(const std::string& fontPath);
    bool loadPlayerTexture(const std::string& texturePath);
    void setPlayerTexture(const sf::Texture& texture);
};