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
    sf::Music startMusic;
    if (!startMusic.openFromFile("Assets/Sounds/Music/start_music.mp3")) {
        std::cerr << "Failed to load start music!" << std::endl;
        return -1;
    }
    startMusic.setLooping(false);
    startMusic.setVolume(35);



    sf::Clock clock;
    GameState gameState;
    gameState.setGameState(States::START);

    float winDelayTimer = 0.0f;
    float startDelayTimer = 0.0f;
    const float START_DELAY = 8.0f;
    const float WIN_DELAY = 3.0f;
    int startSceneStep = 0;
    const int TOTAL_START_STEPS = 10;
    bool startMovementComplete = false;

    Map map;
    Player player(&map);
    player.SetGameState(&gameState);
    EnemyManager enemyManager(&map, &player, 10);
    player.SetEnemyManager(&enemyManager);

    map.loadFromFile("Assets/Map/test.rmap");
    player.Initialise();
    enemyManager.Initialise();
    int const TILE_SIZE = 16;

    // - - - - - - - - - - - - Load - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    player.Load();

    sf::Vector2f startPos(122, -64); // Start above screen
    startPos.x = std::floor(startPos.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
    startPos.y = std::floor(startPos.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
    player.SetPosition(startPos);
    player.SetCreateTunnels(true);
    player.SetTargetPosition(sf::Vector2f(startPos.x, startPos.y + TILE_SIZE)); // First grid step

    enemyManager.SpawnEnemy(EnemyType::POOKA, sf::Vector2f(122, 144));
    map.printInfo();
    startMusic.play(); // Start playing start scene music


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
            startDelayTimer += deltaTime;

            // Update player movement if not yet complete
            if (!startMovementComplete) {
                player.Update(deltaTime);
                sf::Vector2f playerPos = player.getPlayerPosition();

                if (!player.getPlayerMoving()) {
                    startSceneStep++;
                    if (startSceneStep < TOTAL_START_STEPS) {
                        float nextY = -16 + (startSceneStep + 1) * TILE_SIZE;
                        nextY = std::floor(nextY / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                        sf::Vector2f nextTarget(122, nextY);
                        nextTarget.x = std::floor(nextTarget.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                        player.SetTargetPosition(nextTarget);
                        std::cout << "START scene: Moving to next grid position (" << nextTarget.x << ", " << nextTarget.y << ")" << std::endl;
                    }
                    else {
                        // Reached final position
                        sf::Vector2f finalPos(122, 144);
                        finalPos.x = std::floor(finalPos.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                        finalPos.y = std::floor(finalPos.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                        player.SetPosition(finalPos);
                        startMovementComplete = true;
                        player.SetTargetPosition({ finalPos });
                        std::cout << "START scene: Movement complete at (" << finalPos.x << ", " << finalPos.y << ")" << std::endl;


                    }
                }
            }

            if (startDelayTimer >= START_DELAY) {
                gameState.setGameState(States::GAME);
                player.SetCreateTunnels(true); 
                startMusic.stop();
               // player.UpdateMovementMusic();
                std::cout << "Transitioning to GAME state" << std::endl;
            }
            break;
        }
        case States::GAME:
        {
            player.Update(deltaTime);
            enemyManager.Update(deltaTime, player.getPlayerPosition());

            // Check for win condition
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
            if (winDelayTimer >= WIN_DELAY)
            {
                // Restart stage
                sf::Vector2f startPos(122, -16); // Reset to top for start scene
                startPos.x = std::floor(startPos.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                startPos.y = std::floor(startPos.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                player.SetPosition(startPos);
                startSceneStep = 0; // Reset step counter
                startMovementComplete = false; // Reset movement flag
                player.SetTargetPosition(sf::Vector2f(startPos.x, startPos.y + TILE_SIZE)); // First grid step
                player.SetCreateTunnels(true); // Enable tunnel creation
                enemyManager.ClearAllEnemies();
                enemyManager.SpawnEnemy(EnemyType::POOKA, sf::Vector2f(122, 144));
                gameState.setGameState(States::START);
                startDelayTimer = 0.0f; // Reset timer
                startMusic.play(); // Restart start music
                std::cout << "Restarting stage, transitioning to START state" << std::endl;
            }
            break;
        }
        }
        if (gameState.getGameState() != previousState)
        {
            player.UpdateAtStart(); // Call your function here
            previousState = gameState.getGameState(); // Update previous state
        }

        // - - - - - - - - - - - - Draw - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        window.clear(sf::Color::Black);
        map.draw(window);
        player.Draw(window);
        if (gameState.getGameState() == States::GAME || gameState.getGameState() == States::WIN)
        {
            enemyManager.Draw(window);
        }
        // Draw scene-specific text
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