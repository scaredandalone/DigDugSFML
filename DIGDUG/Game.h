#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "Player.h"
#include "Map.h"
#include "Pooka.h"
#include "Rock.h"
#include "EnemyManager.h"
#include "StageManager.h"
#include "GameState.h"
#include "SFX.h"


class Game
{
private:
    // Window and graphics
    sf::RenderWindow window;
    sf::Font font;
    sf::Clock clock;

    // Game objects
    std::unique_ptr<Map> map;
    std::unique_ptr<Player> player;
    std::unique_ptr<EnemyManager> enemyManager;
    std::unique_ptr<GameState> gameState;
    std::unique_ptr<StageManager> stageManager;

    // Audio
    SFX victory;
    SFX lossMusic;
    SFX noLivesMusic;
    SFX startMusic;

    // UI Text
    sf::Text startText;
    sf::Text winText;
    sf::Text lossText;
    sf::Text livesText;

    // Game timing variables
    float winDelayTimer;
    float startDelayTimer;
    float startPauseTimer;
    float lossDelayTimer;

    // Constants
    static constexpr float START_DELAY = 8.0f;
    static constexpr float WIN_DELAY = 3.0f;
    static constexpr float START_PAUSE_DELAY = 1.0f;
    static constexpr float LOSS_DELAY = 6.0f;
    static constexpr int TILE_SIZE = 16;

    // Start scene variables
    int startSceneStep;
    int TOTAL_START_STEPS;
    bool startMovementComplete;
    bool startSceneInitialized;
    bool startPauseComplete;
    bool movingHorizontally;
    bool lossSceneInitialized;
    int horizontalSteps;
    int verticalSteps;

    // Positions
    sf::Vector2f initialPos;
    sf::Vector2f startPos;

    // Previous state tracking
    States previousState;

public:
    // Constructor and destructor
    Game();
    ~Game() = default;

    // Main game functions
    bool initialize();
    void run();
    void cleanup();

private:
    // Initialization helpers
    bool loadAssets();
    void initializeGameObjects();
    void initializeUI();
    void initializeAudio();
    bool loadInitialMap();
    void calculateStartMovement();

    // Game loop functions
    void handleEvents();
    void update(float deltaTime);
    void render();

    // State-specific updates
    void updateStartState(float deltaTime);
    void updateGameState(float deltaTime);
    void updateWinState(float deltaTime);
    void updateLossState(float deltaTime);

    // Start scene helpers
    void initializeStartScene();
    void updateStartMovement(float deltaTime);
    void setNextStartTarget();

    // Stage management
    void loadNextStage();
    void restartCurrentStage();
    void restartFromFirstStage();

    // Utility functions
    sf::Vector2f snapToGrid(const sf::Vector2f& position) const;
    void updateSpawnPosition();
};