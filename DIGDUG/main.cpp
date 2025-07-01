#include <SFML/Graphics.hpp>
#include <iostream>
#include "Player.h"
#include "Map.h"
#include "Pooka.h"
#include "EnemyManager.h"
#include "GameState.h"

int main()
{
    // - - - - - - - - - - - - Initialise - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    sf::ContextSettings settings;
    sf::RenderWindow window(sf::VideoMode({ 244, 288 }), "DIG DUG", sf::Style::Resize, sf::State::Windowed, settings);
    // - - - - - - - - - - - - Initialise - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // Load font
    sf::Font font;
    if (!font.openFromFile("Assets/Fonts/arial.ttf")) {
        std::cerr << "Failed to load font!" << std::endl;
        return -1;
    }

    // Create text objects
    sf::Text startText(font, "");
    startText.setString("Stage Start");
    startText.setCharacterSize(10);
    startText.setFillColor(sf::Color::Yellow);
    startText.setPosition(sf::Vector2f({ 122 }, { 50 }));

    sf::Text winText(font, "");
    winText.setString("Stage Clear");
    winText.setCharacterSize(10);
    winText.setFillColor(sf::Color::Yellow);
    winText.setPosition(sf::Vector2f({ 122 }, { 50 }));

    // Load start scene music
    SFX startMusic("Assets/Sounds/Music/start_music.mp3", SFX::Type::MUSIC);
    startMusic.setLoop(false);
    startMusic.setVolume(35);

    sf::Clock clock;
    GameState gameState;
    gameState.setGameState(States::START);

    float winDelayTimer = 0.0f;
    float startDelayTimer = 0.0f;
    const float START_DELAY = 8.0f;
    const float WIN_DELAY = 3.0f;
    int startSceneStep = 0;
    const int TOTAL_START_STEPS = 8; // 1 step to (122, 16) + 8 steps to (122, 144)
    bool startMovementComplete = false;
    bool startSceneInitialized = false;

    Map map;
    Player player(&map);
    player.SetGameState(&gameState);
    EnemyManager enemyManager(&map, &player, 10);
    enemyManager.SetGameState(&gameState);
    player.SetEnemyManager(&enemyManager);

    map.loadFromFile("Assets/Map/test.rmap");
    player.Initialise();
    enemyManager.Initialise();
    int const TILE_SIZE = 16;

    // - - - - - - - - - - - - Load - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // Initialize player at (0, 0)
    sf::Vector2f initialPos(-16, 16);
    player.setPlayerInitialPosition(initialPos);
    player.SetCreateTunnels(true);
    player.Load();

    sf::Vector2f firstTarget(122, 16);
    firstTarget.x = std::floor(firstTarget.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
    firstTarget.y = std::floor(firstTarget.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;

    enemyManager.SpawnEnemy(EnemyType::POOKA, sf::Vector2f(144, 144));
    map.printInfo();
    startMusic.play();

    while (window.isOpen())
    {
        // - - - - - - - - - - - - Update - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        sf::Time deltaTimeTimer = clock.restart();
        float deltaTime = deltaTimeTimer.asSeconds();

        sf::Vector2f mousePosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        static States previousState = gameState.getGameState();
        switch (gameState.getGameState())
        {
        case States::START:
        {
            // Initialize start scene on first entry
            if (!startSceneInitialized) {
                // Fully reset player position
                player.setPosition(initialPos); // Updates both hitbox and sprite
                player.setPlayerInitialPosition(initialPos); // Update initialPos
                player.setTargetPosition(initialPos); // Prevent immediate movement
                player.setIsMoving(false); // Stop any movement
                player.DetachHarpoon(); // Clear any harpoon
                player.SetCreateTunnels(true); // Prevent tunnel creation
                startSceneStep = 0;
                startMovementComplete = false;
                startSceneInitialized = true;
                startDelayTimer = 0.0f;
                std::cout << "START scene initialized: Player reset to (" << initialPos.x << ", " << initialPos.y << ")" << std::endl;
                // Set first target after reset
                player.setTargetPosition(firstTarget);
                std::cout << "START scene: Moving to first target (" << firstTarget.x << ", " << firstTarget.y << ")" << std::endl;
            }

            startDelayTimer += deltaTime;

            // Update player movement if not yet complete
            if (!startMovementComplete) {
                player.Update(deltaTime, player.getPlayerPosition());

                // Check if player has reached the current target position
                if (!player.getIsMoving()) {
                    if (startSceneStep == 0) {
                        startSceneStep++;
                        float nextY = 16 + startSceneStep * TILE_SIZE;
                        sf::Vector2f nextTarget(122, nextY);
                        nextTarget.x = std::floor(nextTarget.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                        nextTarget.y = std::floor(nextTarget.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                        player.setTargetPosition(nextTarget);
                        std::cout << "START scene: Moving to next grid position (" << nextTarget.x << ", " << nextTarget.y << ")" << std::endl;
                    }
                    else if (startSceneStep < TOTAL_START_STEPS) {
                        startSceneStep++;
                        float nextY = 16 + startSceneStep * TILE_SIZE;
                        sf::Vector2f nextTarget(122, nextY);
                        nextTarget.x = std::floor(nextTarget.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                        nextTarget.y = std::floor(nextTarget.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                        player.setTargetPosition(nextTarget);
                        std::cout << "START scene: Moving to next grid position (" << nextTarget.x << ", " << nextTarget.y << ")" << std::endl;
                    }
                    else {
                        sf::Vector2f finalPos(122, 152);
                        finalPos.x = std::floor(finalPos.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                        finalPos.y = std::floor(finalPos.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                        player.setPosition(finalPos); // Ensure sprite and hitbox are set
                        player.setTargetPosition(finalPos);
                        startMovementComplete = true;
                        player.SetCreateTunnels(true); // Re-enable tunnel creation
                        std::cout << "START scene: Movement complete at (" << finalPos.x << ", " << finalPos.y << ")" << std::endl;
                    }
                }
            }

            if (startDelayTimer >= START_DELAY) {
                gameState.setGameState(States::GAME);
                startMusic.stop();
                startSceneInitialized = false;
                std::cout << "Transitioning to GAME state" << std::endl;
            }
            break;
        }
        case States::GAME:
        {
            player.Update(deltaTime, player.getPlayerPosition());
            enemyManager.Update(deltaTime, player.getPlayerPosition());
            if (enemyManager.GetAliveEnemyCount() == 0)
            {
                gameState.setGameState(States::WIN);
                winDelayTimer = 0.0f;
                std::cout << "All enemies defeated! Transitioning to WIN state" << std::endl;
            }
            break;
        }
        case States::WIN:
        {
            winDelayTimer += deltaTime;
            player.Update(deltaTime, player.getPlayerPosition()); // Continue updating for win state
            if (winDelayTimer >= WIN_DELAY)
            {
                // Clean up and prepare for restart
                enemyManager.ClearAllEnemies();
                enemyManager.SpawnEnemy(EnemyType::POOKA, sf::Vector2f(144, 144));

                // Fully reset player state
                player.setPosition(initialPos); // Updates both hitbox and sprite
                player.setPlayerInitialPosition(initialPos);
                player.setTargetPosition(initialPos);
                player.setIsMoving(false);
                player.DetachHarpoon();
                player.SetCreateTunnels(false);

                gameState.setGameState(States::START);
                startDelayTimer = 0.0f;
                startSceneInitialized = false;
                startMusic.play();
                std::cout << "Restarting stage, transitioning to START state" << std::endl;
            }
            break;
        }
        if (gameState.getGameState() != previousState)
        {
            previousState = gameState.getGameState();
        }
        }

        // - - - - - - - - - - - - Draw - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        window.clear(sf::Color::Black);
        map.draw(window);
        player.Draw(window);
        enemyManager.Draw(window);

        if (gameState.getGameState() == States::START)
        {
            window.draw(startText);
        }
        else if (gameState.getGameState() == States::WIN)
        {
            window.draw(winText);
        }
        window.display();
        // - - - - - - - - - - - - Draw - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    }
}