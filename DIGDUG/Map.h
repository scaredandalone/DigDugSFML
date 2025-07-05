#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <map>



class Map {
private:
    static const int TILE_SIZE = 16;
    static const int MAP_WIDTH = 224;  // 14 tiles
    static const int MAP_HEIGHT = 240; // 15 tiles
    static const int TILES_X = MAP_WIDTH / TILE_SIZE;     // 14 tiles
    static const int TILES_Y = MAP_HEIGHT / TILE_SIZE;  // 15 tiles

    std::vector<std::vector<int>> tileData;
    std::vector<sf::Sprite> tileSprites;
    sf::Texture tileTexture;
    sf::Sprite tileSprite;
    std::map<char, int> charToTileType;
    std::map<int, int> tileTypeToTexture;  // Maps tile type to texture index
    std::vector<std::pair<char, sf::Vector2f>> entitySpawns;
    // store rock spawn info
    struct RockSpawnInfo {
        sf::Vector2f position;
        int textureIndex;
    };
    std::vector<RockSpawnInfo> rockSpawns;

    int currentLevel;

    void buildTiles();
    void setupTileMappings();
    void setupTextureMapping();

public:
    Map();

    bool loadFromFile(const std::string& filename);
    void draw(sf::RenderWindow& window);

    int getTileAt(float x, float y);
    void setTileAt(float x, float y, int tileType);
    int getTileAtGrid(int gridX, int gridY);
    bool isSolid(float x, float y);

    sf::Vector2i getMapSize() const;
    sf::Vector2i getGridSize() const;
    void printInfo();

    const std::vector<std::pair<char, sf::Vector2f>>& getEntitySpawns() const { return entitySpawns; }
    const std::vector<RockSpawnInfo>& getRockSpawns() const { return rockSpawns; }
    void setCurrentLevel(int level);
};