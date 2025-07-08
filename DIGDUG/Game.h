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
#include "Scoreboard.h"
#include "ScoreManager.h"


class Game
{
private:
    // Window and graphics

    enum class TimerPhase { NORMAL, THIRTY_SECONDS, FIFTEEN_SECONDS };
    TimerPhase currentTimerPhase = TimerPhase::NORMAL;
    sf::RenderWindow window;
    sf::Font font;
    sf::Clock clock;

    // Game objects
    std::unique_ptr<Map> map;
    std::unique_ptr<Player> player;
    std::unique_ptr<EnemyManager> enemyManager;
    std::unique_ptr<GameState> gameState;
    std::unique_ptr<StageManager> stageManager;
    std::unique_ptr<Scoreboard> scoreboard;
    std::unique_ptr<ScoreManager> scoreManager;

    // Audio
    SFX victory;
    SFX lossMusic;
    SFX noLivesMusic;
    SFX startMusic;
    SFX highScoreMusic;
    SFX lastEnemySound;
    SFX lowTimeSound;

    // UI Text
    sf::Text startText;
    sf::Text winText;
    sf::Text lossText;
    sf::Text highScoreText;
    sf::Text newHighScoreText;

    // Game timing variables
    float winDelayTimer;
    float startDelayTimer;
    float startPauseTimer;
    float lossDelayTimer;
    float highScoreTimer;
    float lowTimeDelayTimer;
    
    bool lastEnemySoundPlayed = false;
    bool lastEnemySoundPlaying = false;
    float lastEnemySoundTimer = 0.0f;

    bool lowTimeSoundPlayed;
    bool lowTimeSoundPlaying;



    // Constants
    static constexpr float START_DELAY = 8.0f;
    static constexpr float WIN_DELAY = 3.0f;
    static constexpr float START_PAUSE_DELAY = 1.0f;
    static constexpr float LASTENEMY_DURATION = 2.2f;

    static constexpr float LOSS_DELAY = 3.0f;
    static constexpr float NOLIVES_DELAY = 6.0f;
    static constexpr float HIGHSCORE_DELAY = 11.0f;
    static constexpr float LOWTIME_DELAY = 2.5f;
    static constexpr int TILE_SIZE = 16;

    // Start scene variables
    int startSceneStep;
    int TOTAL_START_STEPS;
    bool startMovementComplete;
    bool startSceneInitialised;
    bool startPauseComplete;
    bool movingHorizontally;
    bool lossSceneInitialised;
    bool highScoreSceneInitialised;
    int horizontalSteps;
    int verticalSteps;

    // Positions
    sf::Vector2f initialPos;
    sf::Vector2f startPos;

    // Previous state tracking
    States previousState;

    // timer
    float levelTimeLimit;
    bool timerEnabled;

public:
    // Constructor and destructor
    Game();
    ~Game() = default;

    // Main game functions
    bool initialise();
    void run();
    void cleanup();

private:
    // Initialization helpers
    bool loadAssets();
    void initialiseGameObjects();
    void initialiseUI();
    void initialiseAudio();
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
    void updateHighScoreState(float deltaTime);


    void drawHighscoreScene();
    // Start scene helpers
    void initialiseStartScene();    
    void updateStartMovement(float deltaTime);
    void setNextStartTarget();

    // High score scene helpers
    void initialiseHighScoreScene();

    // Stage management
    void loadNextStage();
    void restartCurrentStage();
    void restartFromFirstStage();

    // Utility functions
    sf::Vector2f snapToGrid(const sf::Vector2f& position) const;
    void updateSpawnPosition();
};