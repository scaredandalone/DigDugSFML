#include <SFML/Graphics.hpp>
#include <iostream>
#include "Player.h"
#include "Map.h"
#include "Pooka.h"
#include "Rock.h"
#include "EnemyManager.h"
#include "GameState.h"
#include "StageManager.h"

int main()
{
    // - - - - - - - - - - - - Initialise - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    sf::ContextSettings settings;
    sf::RenderWindow window(sf::VideoMode({ 224, 270 }), "DIG DUG", sf::Style::Default, sf::State::Windowed, settings);

    sf::Font font;
    if (!font.openFromFile("Assets/Fonts/arial.ttf")) {
        std::cerr << "Failed to load font!" << std::endl;
        return -1;
    }

    SFX victory("Assets/Sounds/Music/success.mp3");
    victory.setVolume(30);

    // Initialize StageManager
    StageManager stageManager("Assets/Map/");

    // Create text objects
    sf::Text startText(font, "");
    startText.setString("Stage Start");
    startText.setCharacterSize(10);
    startText.setFillColor(sf::Color::Yellow);
    startText.setPosition(sf::Vector2f(112, 50));

    sf::Text winText(font, "");
    winText.setString("Stage Clear");
    winText.setCharacterSize(10);
    winText.setFillColor(sf::Color::Yellow);
    winText.setPosition(sf::Vector2f(112, 50));

    sf::Text lossText(font, "");
    lossText.setString("Game Over");
    lossText.setCharacterSize(10);
    lossText.setFillColor(sf::Color::Red);
    lossText.setPosition(sf::Vector2f(112, 50));

  

    // Load start scene music
    SFX lossMusic("Assets/Sounds/Music/loss.mp3", SFX::Type::MUSIC);
    SFX noLivesMusic("Assets/Sounds/Music/nolivesleft.mp3", SFX::Type::MUSIC);
    SFX startMusic("Assets/Sounds/Music/start_music.mp3", SFX::Type::MUSIC);
    startMusic.setLoop(false); lossMusic.setLoop(false); noLivesMusic.setLoop(false);
    startMusic.setVolume(35); lossMusic.setVolume(35); noLivesMusic.setVolume(35);

    sf::Clock clock;
    GameState gameState;
    gameState.setGameState(States::START);

    float winDelayTimer = 0.0f;
    float startDelayTimer = 0.0f;
    float startPauseTimer = 0.0f;
    float lossDelayTimer = 0.0f;
    const float START_DELAY = 8.0f;
    const float WIN_DELAY = 3.0f;
    const float START_PAUSE_DELAY = 1.0f;
    const float LOSS_DELAY = 6.0f;
    int startSceneStep = 0;
    int TOTAL_START_STEPS = 0;
    bool startMovementComplete = false;
    bool startSceneInitialized = false;
    bool startPauseComplete = false;
    bool movingHorizontally = true;
    bool lossSceneInitialized = false;
    int horizontalSteps = 0;
    int verticalSteps = 0;

    Map map;
    Player player(&map);
    player.SetGameState(&gameState);
    EnemyManager enemyManager(&map, &player, 10);
    enemyManager.SetGameState(&gameState);
    player.SetEnemyManager(&enemyManager);

    sf::Text livesText(font, "");
    livesText.setCharacterSize(10);
    livesText.setFillColor(sf::Color::Red);
    livesText.setPosition(sf::Vector2f(112,16));

    // Load initial map using StageManager
    std::string mapFile = stageManager.getMapFile(stageManager.getCurrentStage());
    if (!mapFile.empty()) {
        map.loadFromFile(mapFile);
    }
    else {
        std::cerr << "No maps available!" << std::endl;
        return -1;
    }

    player.Initialise();
    enemyManager.Initialise();
    int const TILE_SIZE = 16;

    // - - - - - - - - - - - - Load - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // Initialize player at (-16, 16)
    sf::Vector2f initialPos(-16, 16);
    sf::Vector2f startPos;
    const auto& spawns = map.getEntitySpawns();
    for (const auto& spawn : spawns) {
        if (spawn.first == '*') {
            startPos = spawn.second;
            break;
        }
    }

    // Calculate total steps needed
    float horizontalDistance = abs(startPos.x - (-16));
    horizontalSteps = static_cast<int>(horizontalDistance / TILE_SIZE);
    float verticalDistance = startPos.y - 16;
    verticalSteps = static_cast<int>(verticalDistance / TILE_SIZE);
    TOTAL_START_STEPS = horizontalSteps + verticalSteps;

    player.setPlayerInitialPosition(initialPos);
    player.SetCreateTunnels(false);
    player.Load();

    enemyManager.SpawnEnemiesFromMap();
    enemyManager.SpawnRocksFromMap();
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
                // Recalculate steps in case map changed
                float horizontalDistance = abs(startPos.x - (-16));
                horizontalSteps = static_cast<int>(horizontalDistance / TILE_SIZE);
                float verticalDistance = startPos.y - 16;
                verticalSteps = static_cast<int>(verticalDistance / TILE_SIZE);
                TOTAL_START_STEPS = horizontalSteps + verticalSteps;

                // Fully reset player position
                player.setPosition(initialPos);
                player.setPlayerInitialPosition(initialPos);
                player.setTargetPosition(initialPos);
                player.setIsMoving(false);
                player.DetachHarpoon();
                player.SetCreateTunnels(false);
                player.setHealth(1); // Reset health
                startSceneStep = 0;
                startMovementComplete = false;
                startSceneInitialized = true;
                startDelayTimer = 0.0f;
                startPauseTimer = 0.0f;
                startPauseComplete = false;
                movingHorizontally = true;
                startMusic.play();
                std::cout << "START scene initialized: Player reset to (" << initialPos.x << ", " << initialPos.y << ")" << std::endl;
            }

            startDelayTimer += deltaTime;

            // Handle initial pause
            if (!startPauseComplete) {
                startPauseTimer += deltaTime;
                if (startPauseTimer >= START_PAUSE_DELAY) {
                    startPauseComplete = true;
                    // Set first target based on whether we need horizontal movement
                    if (horizontalSteps > 0) {
                        // Move horizontally first
                        sf::Vector2f nextTarget(-16 + TILE_SIZE, 16);
                        nextTarget.x = std::floor(nextTarget.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                        nextTarget.y = std::floor(nextTarget.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                        player.setTargetPosition(nextTarget);
                        player.SetCreateTunnels(false); // No tunnels on surface
                        movingHorizontally = true;
                        startSceneStep = 1;
                    }
                    else {
                        // Go straight to vertical movement
                        sf::Vector2f nextTarget(startPos.x, 32);
                        nextTarget.x = std::floor(nextTarget.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                        nextTarget.y = std::floor(nextTarget.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                        player.setTargetPosition(nextTarget);
                        player.SetCreateTunnels(true); // Create tunnels when digging
                        movingHorizontally = false;
                        startSceneStep = 1;
                    }
                    std::cout << "START scene: Pause complete, starting movement" << std::endl;
                }
            }

            // Update player movement if pause is complete and movement not yet complete
            if (startPauseComplete && !startMovementComplete) {
                player.Update(deltaTime, player.getPlayerPosition());

                // Check if player has reached the current target position
                if (!player.getIsMoving() && startSceneStep < TOTAL_START_STEPS) {
                    startSceneStep++;

                    if (movingHorizontally && startSceneStep <= horizontalSteps) {
                        // Continue horizontal movement
                        float nextX = -16 + startSceneStep * TILE_SIZE;
                        sf::Vector2f nextTarget(nextX, 16);
                        nextTarget.x = std::floor(nextTarget.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                        nextTarget.y = std::floor(nextTarget.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                        player.setTargetPosition(nextTarget);
                        std::cout << "START scene: Moving horizontally to (" << nextTarget.x << ", " << nextTarget.y << ")" << std::endl;
                    }
                    else if (movingHorizontally && startSceneStep > horizontalSteps) {
                        // Switch to vertical movement
                        movingHorizontally = false;
                        player.SetCreateTunnels(true); // Enable tunnels for digging
                        float nextY = 16 + (startSceneStep - horizontalSteps) * TILE_SIZE;
                        sf::Vector2f nextTarget(startPos.x, nextY);
                        nextTarget.x = std::floor(nextTarget.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                        nextTarget.y = std::floor(nextTarget.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                        player.setTargetPosition(nextTarget);
                        std::cout << "START scene: Starting vertical movement to (" << nextTarget.x << ", " << nextTarget.y << ")" << std::endl;
                    }
                    else if (!movingHorizontally) {
                        // Continue vertical movement
                        float nextY = 16 + (startSceneStep - horizontalSteps) * TILE_SIZE;
                        sf::Vector2f nextTarget(startPos.x, nextY);
                        nextTarget.x = std::floor(nextTarget.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                        nextTarget.y = std::floor(nextTarget.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                        player.setTargetPosition(nextTarget);
                        std::cout << "START scene: Moving down to (" << nextTarget.x << ", " << nextTarget.y << ")" << std::endl;
                    }
                }
                else if (!player.getIsMoving() && startSceneStep >= TOTAL_START_STEPS) {
                    // Movement complete
                    startMovementComplete = true;
                    sf::Vector2f finalPos(startPos.x, startPos.y);
                    finalPos.x = std::floor(finalPos.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                    finalPos.y = std::floor(finalPos.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
                    player.setPosition(finalPos);
                    player.setTargetPosition(finalPos);
                    player.SetCreateTunnels(true);
                    std::cout << "START scene: Movement complete at (" << finalPos.x << ", " << finalPos.y << ")" << std::endl;
                    player.resetTransform();
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
            if (enemyManager.GetEnemyCount() == 0)
            {
                gameState.setGameState(States::WIN);
                victory.play();
                winDelayTimer = 0.0f;
                std::cout << "All enemies defeated! Transitioning to WIN state" << std::endl;
            }
            if (player.getHealth() <= 0)
            {
                gameState.setGameState(States::LOSS);
                lossDelayTimer = 0.0f;
                lossSceneInitialized = false;
                std::cout << "Player died! Transitioning to LOSS state" << std::endl;
            }
            break;
        }
        case States::WIN:
        {
            winDelayTimer += deltaTime;
            player.Update(deltaTime, player.getPlayerPosition());
            player.resetTransform();
            if (winDelayTimer >= WIN_DELAY)
            {
                // Advance to next stage
                stageManager.incrementStage();
                map.setCurrentLevel(stageManager.getCurrentStage());

                std::string nextMapFile = stageManager.getMapFile(stageManager.getCurrentStage());
                if (!nextMapFile.empty()) {
                    map.loadFromFile(nextMapFile);
                    std::cout << "Loaded stage " << stageManager.getCurrentStage() << ": " << nextMapFile << std::endl;
                }
                else {
                    std::cerr << "Failed to load stage " << stageManager.getCurrentStage() << std::endl;
                    map.setCurrentLevel(stageManager.getCurrentStage()); 
                    map.loadFromFile(stageManager.getMapFile(stageManager.getCurrentStage()));
                }

                enemyManager.ClearAllEnemies();
                enemyManager.ClearAllRocks();
                enemyManager.SpawnEnemiesFromMap();
                enemyManager.SpawnRocksFromMap();

                // Update spawn position for new map
                const auto& spawns = map.getEntitySpawns();
                for (const auto& spawn : spawns) {
                    if (spawn.first == '*') {
                        startPos = spawn.second;
                        break;
                    }
                }

                gameState.setGameState(States::START);
                std::cout << "Stage " << stageManager.getCurrentStage() << " started" << std::endl;
            }
            break;
        }
        case States::LOSS:
        {
            // Initialize loss scene on first entry
            if (!lossSceneInitialized) {
                // Decrement lives ONCE when entering LOSS state
                player.setLives(player.getLives() - 1);

                // Play appropriate music based on lives remaining AFTER decrementing
                if (player.getLives() > 0) {
                    lossMusic.play();
                    std::cout << "Player died! Lives remaining: " << player.getLives() << std::endl;
                }
                else {
                    noLivesMusic.play();
                    std::cout << "Player died! No lives remaining. Game Over!" << std::endl;
                }

                lossSceneInitialized = true;
                lossDelayTimer = 0.0f;
            }

            lossDelayTimer += deltaTime;

            // Continue updating player to show death animation
            player.Update(deltaTime, player.getPlayerPosition());

            if (lossDelayTimer >= LOSS_DELAY) {
                if (player.getLives() <= 0) {
                    // Game over - restart from stage 0
                    noLivesMusic.play();
                    stageManager.setCurrentStage(0);
                    map.setCurrentLevel(stageManager.getCurrentStage());

                    std::string firstMapFile = stageManager.getMapFile(stageManager.getCurrentStage());
                    if (!firstMapFile.empty()) {
                        map.loadFromFile(firstMapFile);
                    }

                    enemyManager.ClearAllEnemies();
                    enemyManager.ClearAllRocks();
                    enemyManager.SpawnEnemiesFromMap();
                    enemyManager.SpawnRocksFromMap();

                    // Update spawn position for first map
                    const auto& spawns = map.getEntitySpawns();
                    for (const auto& spawn : spawns) {
                        if (spawn.first == '*') {
                            startPos = spawn.second;
                            break;
                        }
                    }

                    // Reset lives for new game
                    player.setLives(3);
                    std::cout << "Game Over - Restarting from Stage 0 with 3 lives" << std::endl;
                }
                else {
                    // Still have lives - restart current stage
                    std::cout << "Restarting current stage with " << player.getLives() << " lives remaining" << std::endl;
                }

                // Stop music and transition to start state
                lossMusic.stop();
                noLivesMusic.stop();

                // Reset the loss scene flag for next time
                lossSceneInitialized = false;

                gameState.setGameState(States::START);
            }
            break;
        }
        }

        if (gameState.getGameState() != previousState)
        {
            previousState = gameState.getGameState();
        }

        // - - - - - - - - - - - - Draw - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        window.clear(sf::Color::Black);
        map.draw(window);
        player.Draw(window);
        enemyManager.Draw(window);
        sf::String lives = std::to_string(player.getLives());
        livesText.setString(lives);
        window.draw(livesText);

        if (gameState.getGameState() == States::START)
        {
            window.draw(startText);
        }
        else if (gameState.getGameState() == States::WIN)
        {
            window.draw(winText);
        }
        else if (gameState.getGameState() == States::LOSS)
        {
            window.draw(lossText);
        }
        window.display();
        // - - - - - - - - - - - - Draw - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    }
}