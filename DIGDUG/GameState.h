
enum class States {
    START,
    GAME,
    WIN,
    LOSS,
    HIGHSCORE,
    FLEE
};

class GameState {
    States currentGameState;  
public:
    States getGameState() const { return currentGameState; }
    void setGameState(States state) { currentGameState = state; }
};