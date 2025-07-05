#include "Game.h"
#include <iostream>

Game::Game()
    : window(sf::VideoMode({ 448, 540 }), "DIG DUG", sf::Style::Default, sf::State::Windowed)
    , winDelayTimer(0.0f)
    , startDelayTimer(0.0f)
    , startPauseTimer(0.0f)
    , lossDelayTimer(0.0f)
    , highScoreTimer(0.0f)
    , startSceneStep(0)
    , TOTAL_START_STEPS(0)
    , startMovementComplete(false)
    , startSceneInitialized(false)
    , startPauseComplete(false)
    , movingHorizontally(true)
    , lossSceneInitialized(false)
    , highScoreSceneInitialized(false)
    , horizontalSteps(0)
    , verticalSteps(0)
    , initialPos(-16, 16)
    , previousState(States::START)
    , startMusic("Assets/Sounds/Music/start_music.mp3", SFX::Type::MUSIC)
    , lossMusic("Assets/Sounds/Music/loss.mp3", SFX::Type::MUSIC)
    , noLivesMusic("Assets/Sounds/Music/nolivesleft.mp3", SFX::Type::MUSIC)
    , victory("Assets/Sounds/Music/success.mp3", SFX::Type::MUSIC)
    , highScoreMusic("Assets/Sounds/Music/highscore.mp3", SFX::Type::MUSIC)
    , lossText(font)
    , startText(font)
    , winText(font)
    , highScoreText(font)
    , newHighScoreText(font)
{
}

bool Game::initialize()
{
    if (!loadAssets()) {
        return false;
    }
    sf::View view(sf::FloatRect({ 0, 0 }, { 224, 270 }));
    window.setView(view);

    initializeGameObjects();
    initializeUI();
    initializeAudio();

    if (!loadInitialMap()) {
        return false;
    }

    calculateStartMovement();

    player->Initialise();
    enemyManager->Initialise();
    player->setPlayerInitialPosition(initialPos);
    player->SetCreateTunnels(false);
    player->Load();

    enemyManager->SpawnEnemiesFromMap();
    enemyManager->SpawnRocksFromMap();
    map->printInfo();

    gameState->setGameState(States::START);
    startMusic.play();

    return true;
}

bool Game::loadAssets()
{
    if (!font.openFromFile("Assets/Fonts/digdugfont.otf")) {
        std::cerr << "Failed to load font!" << std::endl;
        return false;
    }

    return true;
}

void Game::initializeGameObjects()
{
    map = std::make_unique<Map>();
    player = std::make_unique<Player>(map.get());
    gameState = std::make_unique<GameState>();
    stageManager = std::make_unique<StageManager>("Assets/Map/");
    enemyManager = std::make_unique<EnemyManager>(map.get(), player.get(), 10);
    scoreboard = std::make_unique<Scoreboard>(sf::Vector2f(0, 240), 8);

    scoreManager = std::make_unique<ScoreManager>(player.get());

    player->SetGameState(gameState.get());
    player->SetScoreManager(scoreManager.get());
    enemyManager->SetGameState(gameState.get());
    player->SetEnemyManager(enemyManager.get());
}

void Game::initializeUI()
{
    startText = sf::Text(font, "Stage Start");
    startText.setCharacterSize(10);
    startText.setFillColor(sf::Color::Yellow);
    startText.setPosition(sf::Vector2f(112, 64));
    sf::FloatRect startTextBounds = startText.getLocalBounds();
    startText.setOrigin(sf::Vector2f(startTextBounds.size.x / 2.0f, 0));

    winText = sf::Text(font, "Stage Clear");
    winText.setCharacterSize(10);
    winText.setFillColor(sf::Color::Yellow);
    winText.setPosition(sf::Vector2f(112, 64));
    sf::FloatRect winTextBounds = winText.getLocalBounds();
    winText.setOrigin(sf::Vector2f(winTextBounds.size.x / 2.0f, 0));

    lossText = sf::Text(font, "Game Over");
    lossText.setCharacterSize(10);
    lossText.setFillColor(sf::Color::Red);
    lossText.setPosition(sf::Vector2f(112, 64));
    sf::FloatRect lossTextBounds = lossText.getLocalBounds();
    lossText.setOrigin(sf::Vector2f(lossTextBounds.size.x / 2.0f, 0));

    highScoreText = sf::Text(font, "HIGH SCORE");
    highScoreText.setCharacterSize(8);
    highScoreText.setFillColor(sf::Color::Yellow);
    highScoreText.setPosition(sf::Vector2f(112, 0));
    sf::FloatRect highScoreBounds = highScoreText.getLocalBounds();
    highScoreText.setOrigin(sf::Vector2f(highScoreBounds.size.x / 2.0f, 0));

    scoreboard->loadFont("Assets/Fonts/digdugfont.otf");
    scoreboard->loadPlayerTexture("Assets/Sprites/Player/spritesheet1.png");
    scoreboard->setTextColor(sf::Color::Red);
    scoreboard->setBackgroundColor(sf::Color(0, 0, 0, 0));
}

void Game::initializeAudio()
{
    victory.setVolume(30);

    startMusic.setLoop(false);
    lossMusic.setLoop(false);
    noLivesMusic.setLoop(false);
    highScoreMusic.setLoop(false);

    startMusic.setVolume(35);
    lossMusic.setVolume(35);
    noLivesMusic.setVolume(35);
    highScoreMusic.setVolume(35);
}

bool Game::loadInitialMap()
{
    std::string mapFile = stageManager->getMapFile(stageManager->getCurrentStage());
    if (!mapFile.empty()) {
        map->loadFromFile(mapFile);
        updateSpawnPosition();
        return true;
    }
    else {
        std::cerr << "No maps available!" << std::endl;
        return false;
    }
}

void Game::calculateStartMovement()
{
    float horizontalDistance = abs(startPos.x - (-16));
    horizontalSteps = static_cast<int>(horizontalDistance / TILE_SIZE);
    float verticalDistance = startPos.y - 16;
    verticalSteps = static_cast<int>(verticalDistance / TILE_SIZE);
    TOTAL_START_STEPS = horizontalSteps + verticalSteps;
}

void Game::updateSpawnPosition()
{
    const auto& spawns = map->getEntitySpawns();
    for (const auto& spawn : spawns) {
        if (spawn.first == '*') {
            startPos = spawn.second;
            break;
        }
    }
}

sf::Vector2f Game::snapToGrid(const sf::Vector2f& position) const
{
    sf::Vector2f snapped;
    snapped.x = std::floor(position.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
    snapped.y = std::floor(position.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
    return snapped;
}

void Game::run()
{
    while (window.isOpen()) {
        sf::Time deltaTimeTimer = clock.restart();
        float deltaTime = deltaTimeTimer.asSeconds();

        handleEvents();
        update(deltaTime);
        render();
    }
}

void Game::handleEvents()
{
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }
    }
}

void Game::update(float deltaTime)
{
    sf::Vector2f mousePosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    switch (gameState->getGameState()) {
    case States::START:
        updateStartState(deltaTime);
        break;
    case States::GAME:
        updateGameState(deltaTime);
        break;
    case States::WIN:
        updateWinState(deltaTime);
        break;
    case States::LOSS:
        updateLossState(deltaTime);
        break;
    case States::HIGHSCORE:
        updateHighScoreState(deltaTime);
        break;
    }

    scoreboard->update(*player, *stageManager);
    scoreManager->UpdateNewHighScoreTimer(deltaTime); // Update new high score timer
    highScoreText.setString("HIGHSCORE " + std::to_string(scoreManager->GetHighScore())); // Update high score text
    sf::FloatRect highScoreBounds = highScoreText.getLocalBounds();
    highScoreText.setOrigin(sf::Vector2f(highScoreBounds.size.x / 2, 0)); // Re-center after text update

    if (gameState->getGameState() != previousState) {
        previousState = gameState->getGameState();
    }
}

void Game::updateStartState(float deltaTime)
{
    if (!startSceneInitialized) {
        initializeStartScene();
    }

    startDelayTimer += deltaTime;
    updateStartMovement(deltaTime);

    if (startDelayTimer >= START_DELAY) {
        gameState->setGameState(States::GAME);
        startMusic.stop();
        startSceneInitialized = false;
        std::cout << "Transitioning to GAME state" << std::endl;
    }
}

void Game::initializeStartScene()
{
    calculateStartMovement();

    player->setPosition(initialPos);
    player->setPlayerInitialPosition(initialPos);
    player->setTargetPosition(initialPos);
    player->setIsMoving(false);
    player->DetachHarpoon();
    player->SetCreateTunnels(false);
    player->setHealth(1);

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

void Game::updateStartMovement(float deltaTime)
{
    if (!startPauseComplete) {
        startPauseTimer += deltaTime;
        if (startPauseTimer >= START_PAUSE_DELAY) {
            startPauseComplete = true;
            setNextStartTarget();
            std::cout << "START scene: Pause complete, starting movement" << std::endl;
        }
    }

    if (startPauseComplete && !startMovementComplete) {
        player->Update(deltaTime, player->getPlayerPosition());

        if (!player->getIsMoving() && startSceneStep < TOTAL_START_STEPS) {
            startSceneStep++;
            setNextStartTarget();
        }
        else if (!player->getIsMoving() && startSceneStep >= TOTAL_START_STEPS) {
            startMovementComplete = true;
            sf::Vector2f finalPos = snapToGrid(startPos);
            player->setPosition(finalPos);
            player->setTargetPosition(finalPos);
            player->SetCreateTunnels(true);
            std::cout << "START scene: Movement complete at (" << finalPos.x << ", " << finalPos.y << ")" << std::endl;
            player->resetTransform();
        }
    }
}

void Game::setNextStartTarget()
{
    if (horizontalSteps > 0) {
        if (movingHorizontally && startSceneStep <= horizontalSteps) {
            float nextX = -16 + startSceneStep * TILE_SIZE;
            sf::Vector2f nextTarget = snapToGrid(sf::Vector2f(nextX, 16));
            player->setTargetPosition(nextTarget);
            player->SetCreateTunnels(false);
            std::cout << "START scene: Moving horizontally to (" << nextTarget.x << ", " << nextTarget.y << ")" << std::endl;
        }
        else if (movingHorizontally && startSceneStep > horizontalSteps) {
            movingHorizontally = false;
            player->SetCreateTunnels(true);
            float nextY = 16 + (startSceneStep - horizontalSteps) * TILE_SIZE;
            sf::Vector2f nextTarget = snapToGrid(sf::Vector2f(startPos.x, nextY));
            player->setTargetPosition(nextTarget);
            std::cout << "START scene: Starting vertical movement to (" << nextTarget.x << ", " << nextTarget.y << ")" << std::endl;
        }
        else if (!movingHorizontally) {
            float nextY = 16 + (startSceneStep - horizontalSteps) * TILE_SIZE;
            sf::Vector2f nextTarget = snapToGrid(sf::Vector2f(startPos.x, nextY));
            player->setTargetPosition(nextTarget);
            std::cout << "START scene: Moving down to (" << nextTarget.x << ", " << nextTarget.y << ")" << std::endl;
        }
    }
    else {
        sf::Vector2f nextTarget = snapToGrid(sf::Vector2f(startPos.x, 32));
        player->setTargetPosition(nextTarget);
        player->SetCreateTunnels(true);
        movingHorizontally = false;
        startSceneStep = 1;
    }
}

void Game::updateGameState(float deltaTime)
{
    player->Update(deltaTime, player->getPlayerPosition());
    enemyManager->Update(deltaTime, player->getPlayerPosition());

    if (enemyManager->GetEnemyCount() == 0) {
        gameState->setGameState(States::WIN);
        victory.play();
        winDelayTimer = 0.0f;
        std::cout << "All enemies defeated! Transitioning to WIN state" << std::endl;
    }

    if (player->getHealth() <= 0) {
        gameState->setGameState(States::LOSS);
        lossDelayTimer = 0.0f;
        lossSceneInitialized = false;
        std::cout << "Player died! Transitioning to LOSS state" << std::endl;
    }
}

void Game::updateWinState(float deltaTime)
{
    winDelayTimer += deltaTime;
    player->Update(deltaTime, player->getPlayerPosition());
    player->resetTransform();

    if (winDelayTimer >= WIN_DELAY) {
        loadNextStage();
        gameState->setGameState(States::START);
        std::cout << "Stage " << stageManager->getCurrentStage() << " started" << std::endl;
    }
}

void Game::updateLossState(float deltaTime)
{
    if (!lossSceneInitialized) {
        player->setLives(player->getLives() - 1);
        if (player->getLives() > 0) {
            lossMusic.play();
            std::cout << "Player died! Lives remaining: " << player->getLives() << std::endl;
        }
        else {
            noLivesMusic.play();
            std::cout << "Player died! No lives remaining. Game Over!" << std::endl;
        }
        lossSceneInitialized = true;
        lossDelayTimer = 0.0f;
    }

    lossDelayTimer += deltaTime;
    player->Update(deltaTime, player->getPlayerPosition());

    // Use different delay based on whether player has lives left
    float delayToUse = (player->getLives() <= 0) ? NOLIVES_DELAY : LOSS_DELAY;

    if (lossDelayTimer >= delayToUse) {
        if (player->getLives() <= 0) {
            // Update the high score and check if it's new
            int oldHighScore = scoreManager->GetHighScore();
            scoreManager->UpdateHighScore();
            bool newHighScore = (scoreManager->GetHighScore() > oldHighScore);
            if (newHighScore) {
                // Transition to high score scene
                gameState->setGameState(States::HIGHSCORE);
                initializeHighScoreScene();
                std::cout << "New high score achieved! Transitioning to HIGHSCORE state" << std::endl;
            }
            else {
                // Normal game over flow
                restartFromFirstStage();
                scoreManager->ResetScore();
                gameState->setGameState(States::START);
            }
        }
        else {
            restartCurrentStage();
            gameState->setGameState(States::START);
        }
        lossMusic.stop();
        noLivesMusic.stop();
        lossSceneInitialized = false;
    }
}

void Game::updateHighScoreState(float deltaTime)
{
    if (!highScoreSceneInitialized) {
        initializeHighScoreScene();
    }

    highScoreTimer += deltaTime;

    if (highScoreTimer >= HIGHSCORE_DELAY) {
        highScoreMusic.stop();
        highScoreSceneInitialized = false;

        // After high score scene, restart from first stage
        restartFromFirstStage();
        scoreManager->ResetScore();
        gameState->setGameState(States::START);

        std::cout << "High score scene complete! Restarting game" << std::endl;
    }
}

void Game::initializeHighScoreScene()
{
    highScoreTimer = 0.0f;
    highScoreSceneInitialized = true;

    // Stop any other music and play high score music
    lossMusic.stop();
    noLivesMusic.stop();
    highScoreMusic.play();

    // Ensure the score manager is showing the new high score
    scoreManager->ShouldShowNewHighScore();

    std::cout << "High score scene initialized" << std::endl;
    std::cout << "New high score text: " << scoreManager->GetNewHighScoreText() << std::endl;
    drawHighscoreScene();
}

void Game::loadNextStage()
{
    stageManager->incrementStage();
    map->setCurrentLevel(stageManager->getCurrentStage());

    std::string nextMapFile = stageManager->getMapFile(stageManager->getCurrentStage());
    if (!nextMapFile.empty()) {
        map->loadFromFile(nextMapFile);
        std::cout << "Loaded stage " << stageManager->getCurrentStage() << ": " << nextMapFile << std::endl;
    }
    else {
        std::cerr << "Failed to load stage " << stageManager->getCurrentStage() << std::endl;
        map->setCurrentLevel(stageManager->getCurrentStage());
        map->loadFromFile(stageManager->getMapFile(stageManager->getCurrentStage()));
    }

    enemyManager->ClearAllEnemies();
    enemyManager->ClearAllRocks();
    enemyManager->SpawnEnemiesFromMap();
    enemyManager->SpawnRocksFromMap();

    updateSpawnPosition();
}

void Game::restartCurrentStage()
{
    std::cout << "Restarting current stage with " << player->getLives() << " lives remaining" << std::endl;
}

void Game::restartFromFirstStage()
{
    stageManager->setCurrentStage(0);
    map->setCurrentLevel(stageManager->getCurrentStage());

    std::string firstMapFile = stageManager->getMapFile(stageManager->getCurrentStage());
    if (!firstMapFile.empty()) {
        map->loadFromFile(firstMapFile);
    }

    enemyManager->ClearAllEnemies();
    enemyManager->ClearAllRocks();
    enemyManager->SpawnEnemiesFromMap();
    enemyManager->SpawnRocksFromMap();

    updateSpawnPosition();
    player->setLives(3);
    std::cout << "Game Over - Restarting from Stage 0 with 3 lives" << std::endl;
}


void Game::drawHighscoreScene() {
  sf::Text newHighScoreText(font, scoreManager->GetNewHighScoreText());
  newHighScoreText.setCharacterSize(12);


  static sf::Clock flashClock;
  float flashInterval = 0.25f;
  bool isYellow = fmod(flashClock.getElapsedTime().asSeconds(), flashInterval * 2) < flashInterval;

  // Alternate between red and yellow
  newHighScoreText.setFillColor(isYellow ? sf::Color::Yellow : sf::Color::Red);

  newHighScoreText.setPosition(sf::Vector2f(112, 135)); // Center of screen
  sf::FloatRect newHighScoreBounds = newHighScoreText.getLocalBounds();
  newHighScoreText.setOrigin(sf::Vector2f(newHighScoreBounds.size.x / 2, newHighScoreBounds.size.y / 2));
  window.draw(newHighScoreText);
  window.draw(highScoreText);
}

void Game::render()
{
    window.clear(sf::Color::Black);
    // Render normal game elements for all other states
    map->draw(window);
    player->Draw(window);
    enemyManager->Draw(window);

    scoreboard->render(window);
    window.draw(highScoreText); // Always draw high score

    switch (gameState->getGameState()) {
    case States::START:
        window.draw(startText);
        break;
    case States::WIN:
        window.draw(winText);
        break;
    case States::LOSS:
        window.draw(lossText);
        break;
    case States::GAME:
        break;
    case States::HIGHSCORE:
        drawHighscoreScene();
        if(highScoreTimer > HIGHSCORE_DELAY)
        break;
    }
    window.display();
}

void Game::cleanup()
{
    startMusic.stop();
    lossMusic.stop();
    noLivesMusic.stop();
    highScoreMusic.stop();
}