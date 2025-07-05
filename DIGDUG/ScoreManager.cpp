#include "ScoreManager.h"
#include "Player.h"
#include "Entity.h"
#include "Pooka.h"

ScoreManager::ScoreManager(Player* playerRef) : player(playerRef), highScore(1000), showNewHighScore(false), newHighScoreTimer(0.0f) {
    if (!player) {
        std::cout << "Warning: ScoreManager created with null player reference!" << std::endl;
    }
}

void ScoreManager::OnEnemyKilled(std::shared_ptr<Entity> enemy, KillMethod method) {
    if (!enemy || !player) {
        std::cout << "ScoreManager: Invalid enemy or player reference in OnEnemyKilled" << std::endl;
        return;
    }

    if (HasEnemyBeenScored(enemy.get())) {
        std::cout << "ScoreManager: Enemy already scored, skipping..." << std::endl;
        return;
    }

    if (!IsEnemyReadyForScoring(enemy, method)) {
        std::cout << "ScoreManager: Enemy not ready for scoring yet" << std::endl;
        return;
    }

    int baseScore = enemy->getScoreAwarded();
    std::cout << "ScoreManager: Raw enemy score value: " << baseScore << std::endl;

    if (baseScore < 0 || baseScore > 10000) {
        std::cout << "ScoreManager: WARNING - Invalid enemy score detected: " << baseScore << std::endl;
        return;
    }

    int finalScore = baseScore;
    std::string killMethodStr = "";

    switch (method) {
    case KillMethod::INFLATION:
        finalScore = baseScore;
        killMethodStr = "inflation";
        std::cout << "Enemy killed by inflation! Base score: " << baseScore << std::endl;
        break;

    case KillMethod::ROCK:
        finalScore = baseScore * ROCK_KILL_MULTIPLIER;
        killMethodStr = "rock crush";
        std::cout << "Enemy crushed by rock! Base score: " << baseScore
            << " x" << ROCK_KILL_MULTIPLIER << " = " << finalScore << std::endl;
        break;
    }

    MarkEnemyAsScored(enemy.get());
    player->addScore(finalScore);
    // UpdateHighScore called in restartFromFirstStage instead

    std::cout << "ScoreManager: Added " << finalScore << " points for " << killMethodStr
        << " kill. Total score: " << player->getScore() << ", High score: " << highScore << std::endl;
}

bool ScoreManager::HasEnemyBeenScored(Entity* enemy) const {
    return scoredEnemies.find(enemy) != scoredEnemies.end();
}

void ScoreManager::MarkEnemyAsScored(Entity* enemy) {
    if (enemy) {
        scoredEnemies.insert(enemy);
    }
}

bool ScoreManager::IsEnemyReadyForScoring(std::shared_ptr<Entity> enemy, KillMethod method) const {
    if (!enemy) return false;

    if (enemy->isActive()) {
        std::cout << "ScoreManager: Enemy still active, not ready for scoring" << std::endl;
        return false;
    }

    if (method == KillMethod::INFLATION) {
        auto pooka = std::dynamic_pointer_cast<Pooka>(enemy);
        if (pooka) {
            if (pooka->getPumpState() < 4) {
                std::cout << "ScoreManager: Pooka not fully inflated (state: "
                    << pooka->getPumpState() << "), not ready for scoring" << std::endl;
                return false;
            }
        }
    }

    return true;
}

void ScoreManager::ClearScoredEnemies() {
    scoredEnemies.clear();
    std::cout << "ScoreManager: Cleared scored enemies tracking" << std::endl;
}

void ScoreManager::OnTunnelDug(int tileType) {
    if (!player) {
        std::cout << "ScoreManager: Invalid player reference in OnTunnelDug" << std::endl;
        return;
    }

    int points = 0;
    switch (tileType) {
    case 2:
        points = TUNNEL_SCORE_BASE_1;
        break;
    case 3:
        points = TUNNEL_SCORE_BASE_2;
        break;
    case 4:
        points = TUNNEL_SCORE_BASE_3;
        break;
    case 5:
        points = TUNNEL_SCORE_BASE_4;
        break;
    default:
        return;
    }

    player->addScore(points);
    // UpdateHighScore called in restartFromFirstStage instead
    std::cout << "ScoreManager: Dug tunnel type " << tileType << " for " << points
        << " points. Total score: " << player->getScore() << ", High score: " << highScore << std::endl;
}

void ScoreManager::AddBonusPoints(int points, const std::string& reason) {
    if (!player) {
        std::cout << "ScoreManager: Invalid player reference in AddBonusPoints" << std::endl;
        return;
    }

    player->addScore(points);
    // UpdateHighScore called in restartFromFirstStage instead
    std::cout << "ScoreManager: Bonus points awarded - " << points << " points";
    if (!reason.empty()) {
        std::cout << " (" << reason << ")";
    }
    std::cout << ". Total score: " << player->getScore() << ", High score: " << highScore << std::endl;
}

void ScoreManager::ResetScore() {
    if (!player) {
        std::cout << "ScoreManager: Invalid player reference in ResetScore" << std::endl;
        return;
    }

    player->clearScore();
    ClearScoredEnemies();
    std::cout << "ScoreManager: Score reset to 0, High score: " << highScore << std::endl;
}

void ScoreManager::ResetHighScore() {
    highScore = 0;
    showNewHighScore = false;
    newHighScoreTimer = 0.0f;
    std::cout << "ScoreManager: High score reset to 0" << std::endl;
}

int ScoreManager::GetHighScore() const {
    return highScore;
}

void ScoreManager::UpdateHighScore() {
    if (player && player->getScore() > highScore) {
        highScore = player->getScore();
        showNewHighScore = true;
        newHighScoreTimer = 0.0f;
        std::cout << "ScoreManager: New high score: " << highScore << std::endl;
    }
}

void ScoreManager::UpdateNewHighScoreTimer(float deltaTime) {
    if (showNewHighScore) {
        newHighScoreTimer += deltaTime;
        if (newHighScoreTimer >= NEW_HIGHSCORE_DURATION) {
            showNewHighScore = false;
            newHighScoreTimer = 0.0f;
        }
    }
}

bool ScoreManager::ShouldShowNewHighScore() const {
    return showNewHighScore;
}

std::string ScoreManager::GetNewHighScoreText() const {
    return "NEW HIGHSCORE";
}