#pragma once
#include <memory>
#include <iostream>
#include <unordered_set>

class Entity;
class Player;

enum class KillMethod {
    INFLATION,
    ROCK,
    TUNNEL_BONUS
};

class ScoreManager {
private:
    Player* player;
    std::unordered_set<Entity*> scoredEnemies; // Track enemies that have been scored
    int highScore;
    bool showNewHighScore; // Flag for new high score notification
    float newHighScoreTimer; // Timer for new high score notification

public:
    ScoreManager(Player* playerRef);

    // Main scoring methods
    void OnEnemyKilled(std::shared_ptr<Entity> enemy, KillMethod method);
    void OnTunnelDug(int tileType);

    // Utility methods
    void AddBonusPoints(int points, const std::string& reason = "");
    void ResetScore();
    void ResetHighScore();
    void ClearScoredEnemies();
    int GetHighScore() const; 
    void UpdateHighScore(); 
    void UpdateNewHighScoreTimer(float deltaTime); 
    bool ShouldShowNewHighScore() const; 
    std::string GetNewHighScoreText() const; 

    // Score multipliers and constants
    static constexpr int ROCK_KILL_MULTIPLIER = 2;
    static constexpr int TUNNEL_SCORE_BASE_1 = 10;  // For tile type 2
    static constexpr int TUNNEL_SCORE_BASE_2 = 20;  // For tile type 3  
    static constexpr int TUNNEL_SCORE_BASE_3 = 30;  // For tile type 4
    static constexpr int TUNNEL_SCORE_BASE_4 = 40;  // For tile type 5
    static constexpr float NEW_HIGHSCORE_DURATION = 3.0f; // Duration for new high score text

private:
    bool HasEnemyBeenScored(Entity* enemy) const;
    void MarkEnemyAsScored(Entity* enemy);
    bool IsEnemyReadyForScoring(std::shared_ptr<Entity> enemy, KillMethod method) const;
};