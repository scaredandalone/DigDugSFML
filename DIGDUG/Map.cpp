#include "Map.h"
#include <fstream>
#include <iostream>

Map::Map() : tileSprite(tileTexture) {
    tileData.resize(TILES_Y, std::vector<int>(TILES_X, 0));

    // Load texture once in constructor
    if (!tileTexture.loadFromFile("Assets/Map/tilesheet.png")) {
        std::cerr << "Failed to load tilesheet texture!" << std::endl;
    }
    tileSprite.setTexture(tileTexture);

}

//sf::Sprite Map::getTileSprite(int tileType) {
//    tileSprite.setTextureRect(sf::IntRect({ tileType * TILE_SIZE, 0 }, { TILE_SIZE, TILE_SIZE }));
//    return tileSprite;
//}

bool Map::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open map file: " << filename << std::endl;
        return false;
    }

    std::string line;
    int row = 0;

    while (std::getline(file, line) && row < TILES_Y) {
        std::string cleanLine;

        // Extract only digits from the line
        for (char c : line) {
            if (c >= '0' && c <= '9') {
                cleanLine += c;
            }
        }

        // Fill the row with data from cleanLine
        for (int col = 0; col < TILES_X && col < static_cast<int>(cleanLine.length()); col++) {
            tileData[row][col] = cleanLine[col] - '0';
        }

        // Fill remaining columns with 0 if line is incomplete
        for (int col = static_cast<int>(cleanLine.length()); col < TILES_X; col++) {
            tileData[row][col] = 0;
        }

        row++;
    }

    // Fill remaining rows with 0 if file has fewer lines
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
    tileSprites.clear();
    tileSprites.reserve(TILES_X * TILES_Y);

    for (int row = 0; row < TILES_Y; row++) {
        for (int col = 0; col < TILES_X; col++) {
            int tileType = tileData[row][col];

            if (tileType == 0) continue;

            tileSprite.setTexture(tileTexture);

            tileSprite.setTextureRect(sf::IntRect({ tileType * TILE_SIZE, 0}, { TILE_SIZE, TILE_SIZE }));

            tileSprite.setPosition(sf::Vector2f(col * TILE_SIZE, row * TILE_SIZE));

            tileSprites.push_back(tileSprite);
        }
    }
}

void Map::draw(sf::RenderWindow& window) {
    for (const auto& tileSprite : tileSprites) {
        window.draw(tileSprite);
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
    // For now, any non-zero tile is considered solid
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