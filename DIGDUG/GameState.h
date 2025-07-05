
enum class States {
    START,
    GAME,
    WIN,
    LOSS,
    HIGHSCORE
};

class GameState {
    States currentGameState;  // Use States, not GameState
public:
    States getGameState() const { return currentGameState; }
    void setGameState(States state) { currentGameState = state; }
};