#include "Map.h"
#include <fstream>
#include <iostream>

Map::Map() {
    // Initialize tile data grid
    tileData.resize(TILES_Y, std::vector<int>(TILES_X, 0));
}

sf::Color Map::getTileColor(int tileType) {
    switch (tileType) {
    case 0: return sf::Color::Transparent;    // Empty/air
    case 1: return sf::Color(139, 69, 19);    // Brown dirt
    case 2: return sf::Color(105, 105, 105);  // Gray stone
    case 3: return sf::Color(34, 139, 34);    // Green grass
    case 4: return sf::Color(65, 105, 225);   // Blue water
    default: return sf::Color::Magenta;       // Error color
    }
}

bool Map::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Failed to open map file: " << filename << std::endl;
        return false;
    }

    std::string line;
    int row = 0;

    while (std::getline(file, line) && row < TILES_Y) {
        std::string cleanLine;
        for (char c : line) {
            if (c >= '0' && c <= '9') {
                cleanLine += c;
            }
        }

        for (int col = 0; col < TILES_X && col < cleanLine.length(); col++) {
            tileData[row][col] = cleanLine[col] - '0';
        }

        // if line isnt comlpet
        for (int col = cleanLine.length(); col < TILES_X; col++) {
            tileData[row][col] = 0;
        }

        row++;
    }

    // remaining rows are 0 if file has fewer lines
    for (int r = row; r < TILES_Y; r++) {
        for (int c = 0; c < TILES_X; c++) {
            tileData[r][c] = 0;
        }
    }

    file.close();
    buildTiles();
    std::cout << "Map loaded successfully from " << filename << std::endl;
    return true;
}

void Map::buildTiles() {
    tiles.clear();
    tiles.reserve(TILES_X * TILES_Y);

    for (int row = 0; row < TILES_Y; row++) {
        for (int col = 0; col < TILES_X; col++) {
            int tileType = tileData[row][col];

            // skip empty tiles
            if (tileType == 0) continue;

            sf::RectangleShape tile;
            tile.setSize(sf::Vector2f(TILE_SIZE, TILE_SIZE));
            tile.setPosition(sf::Vector2f(col * TILE_SIZE + TILE_SIZE / 2.0f, row * TILE_SIZE + TILE_SIZE / 2.0f));
            tile.setOrigin(sf::Vector2f(tile.getSize().x / 2.0f, tile.getSize().y / 2.0f));
            tile.setFillColor(getTileColor(tileType));

            tiles.push_back(tile);
        }
    }
}

void Map::draw(sf::RenderWindow& window) {
    for (const auto& tile : tiles) {
        window.draw(tile);
    }
}

int Map::getTileAt(float x, float y) {
    int col = static_cast<int>(x) / TILE_SIZE;
    int row = static_cast<int>(y) / TILE_SIZE;

    if (row >= 0 && row < TILES_Y && col >= 0 && col < TILES_X) {
        return tileData[row][col];
    }
    return -1;
}

void Map::setTileAt(float x, float y, int tileType) {
    int col = static_cast<int>(x) / TILE_SIZE;
    int row = static_cast<int>(y) / TILE_SIZE;

    if (row >= 0 && row < TILES_Y && col >= 0 && col < TILES_X) {
        tileData[row][col] = tileType;
        buildTiles();
    }
}

int Map::getTileAtGrid(int gridX, int gridY) {
    if (gridY >= 0 && gridY < TILES_Y && gridX >= 0 && gridX < TILES_X) {
        return tileData[gridY][gridX];
    }
    return -1;
}

bool Map::isSolid(float x, float y) {
    int tileType = getTileAt(x, y);
    return tileType > 0;
}

sf::Vector2i Map::getMapSize() const {
    return sf::Vector2i(MAP_WIDTH, MAP_HEIGHT);
}

sf::Vector2i Map::getGridSize() const {
    return sf::Vector2i(TILES_X, TILES_Y);
}


void Map::printInfo() {
    std::cout << "Map Info:" << std::endl;
    std::cout << "  Pixel size: " << MAP_WIDTH << "x" << MAP_HEIGHT << std::endl;
    std::cout << "  Grid size: " << TILES_X << "x" << TILES_Y << " tiles" << std::endl;
    std::cout << "  Tile size: " << TILE_SIZE << "x" << TILE_SIZE << " pixels" << std::endl;
    std::cout << "  Total tiles: " << (TILES_X * TILES_Y) << std::endl;
}