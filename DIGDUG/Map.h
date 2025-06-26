#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class Map {
private:
    static const int TILE_SIZE = 16;
    static const int MAP_WIDTH = 224;  // pixels
    static const int MAP_HEIGHT = 248; // pixels
    static const int TILES_X = MAP_WIDTH / TILE_SIZE;   // 14 tiles
    static const int TILES_Y = MAP_HEIGHT / TILE_SIZE;  // 15 tiles

    std::vector<std::vector<int>> tileData;
    std::vector<sf::RectangleShape> tiles;
    sf::Texture tileTexture;

    sf::Color getTileColor(int tileType);
    void buildTiles();

public:
    Map();

    bool loadFromFile(const std::string& filename);
    void draw(sf::RenderWindow& window);

    // tile access
    int getTileAt(float x, float y);
    void setTileAt(float x, float y, int tileType);
    int getTileAtGrid(int gridX, int gridY);
    bool isSolid(float x, float y);

    // info / debug 
    sf::Vector2i getMapSize() const;
    sf::Vector2i getGridSize() const;
    void printInfo();
};