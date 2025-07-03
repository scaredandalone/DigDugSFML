#include "Game.h"
#include <iostream>

Game::Game()
    : window(sf::VideoMode({ 224, 270 }), "DIG DUG", sf::Style::Default, sf::State::Windowed)
    , winDelayTimer(0.0f)
    , startDelayTimer(0.0f)
    , startPauseTimer(0.0f)
    , lossDelayTimer(0.0f)
    , startSceneStep(0)
    , TOTAL_START_STEPS(0)
    , startMovementComplete(false)
    , startSceneInitialized(false)
    , startPauseComplete(false)
    , movingHorizontally(true)
    , lossSceneInitialized(false)
    , horizontalSteps(0)
    , verticalSteps(0)
    , initialPos(-16, 16)
    , previousState(States::START)
    , startMusic("Assets/Sounds/Music/start_music.mp3", SFX::Type::MUSIC)
    , lossMusic("Assets/Sounds/Music/loss.mp3", SFX::Type::MUSIC)
    , noLivesMusic("Assets/Sounds/Music/nolivesleft.mp3", SFX::Type::MUSIC)
    , victory("Assets/Sounds/Music/success.mp3", SFX::Type::MUSIC)
    , livesText(font)
    , lossText(font)
    , startText(font)
    , winText(font)
{
}

bool Game::initialize()
{
    if (!loadAssets()) {
        return false;
    }

    initializeGameObjects();
    initializeUI();
    initializeAudio();

    if (!loadInitialMap()) {  // Changed from != 0 to !
        return false;
    }

    calculateStartMovement();

    // Initialize game objects
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
    if (!font.openFromFile("Assets/Fonts/arial.ttf")) {
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

    player->SetGameState(gameState.get());
    enemyManager->SetGameState(gameState.get());
    player->SetEnemyManager(enemyManager.get());
}

void Game::initializeUI()
{
    startText = sf::Text(font, "Stage Start");
    startText.setCharacterSize(10);
    startText.setFillColor(sf::Color::Yellow);
    startText.setPosition(sf::Vector2f(112, 50));

    winText = sf::Text(font, "Stage Clear");
    winText.setCharacterSize(10);
    winText.setFillColor(sf::Color::Yellow);
    winText.setPosition(sf::Vector2f(112, 50));

    lossText = sf::Text(font, "Game Over");
    lossText.setCharacterSize(10);
    lossText.setFillColor(sf::Color::Red);
    lossText.setPosition(sf::Vector2f(112, 50));

    livesText = sf::Text(font, "");
    livesText.setCharacterSize(10);
    livesText.setFillColor(sf::Color::Red);
    livesText.setPosition(sf::Vector2f(112, 16));
}

void Game::initializeAudio()
{
    victory.setVolume(30);


    startMusic.setLoop(false);
    lossMusic.setLoop(false);
    noLivesMusic.setLoop(false);

    startMusic.setVolume(35);
    lossMusic.setVolume(35);
    noLivesMusic.setVolume(35);
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
    }

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

    if (lossDelayTimer >= LOSS_DELAY) {
        if (player->getLives() <= 0) {
            restartFromFirstStage();
        }
        else {
            restartCurrentStage();
        }

        lossMusic.stop();
        noLivesMusic.stop();
        lossSceneInitialized = false;
        gameState->setGameState(States::START);
    }
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
    noLivesMusic.play();
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

void Game::render()
{
    window.clear(sf::Color::Black);

    map->draw(window);
    player->Draw(window);
    enemyManager->Draw(window);

    sf::String lives = std::to_string(player->getLives());
    livesText.setString(lives);
    window.draw(livesText);

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
        // No additional UI for game state
        break;
    }

    window.display();
}

void Game::cleanup()
{
    // Any cleanup needed when the game ends
    startMusic.stop();
    lossMusic.stop();
    noLivesMusic.stop();
}