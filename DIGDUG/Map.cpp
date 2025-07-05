
#include "Map.h"
#include "StageManager.h"
#include <fstream>
#include <iostream>

Map::Map() : tileSprite(tileTexture), currentLevel(0) {
    tileData.resize(TILES_Y, std::vector<int>(TILES_X, 0));
    setupTileMappings();
    setupTextureMapping();

    if (!tileTexture.loadFromFile("Assets/Map/tilesheet.png")) {
        std::cerr << "Failed to load tilesheet texture!" << std::endl;
    }
    tileSprite.setTexture(tileTexture);
}

void Map::setupTileMappings() {
    // Keep tile types consistent - always map to 0-5
    charToTileType.clear();
    charToTileType['0'] = 0;  // Empty/tunnel
    charToTileType['1'] = 1;  // Surface
    charToTileType['2'] = 2;  // Dirt type 1
    charToTileType['3'] = 3;  // Dirt type 2
    charToTileType['4'] = 4;  // Dirt type 3
    charToTileType['5'] = 5;  // Dirt type 4
    charToTileType['P'] = 0;  // Enemy spawn
    charToTileType['*'] = 0;  // Player spawn
    charToTileType['R'] = 0;  // Rock will be placed on an empty tile initially
}

void Map::setupTextureMapping() {
    // Map tile types to texture indices based on current level
    tileTypeToTexture.clear();

    tileTypeToTexture[6] = 0; // for the rock, so the enemy cant pathfind to it

    if (currentLevel == 0) {
        tileTypeToTexture[0] = 0;  // Empty
        tileTypeToTexture[1] = 1;  // Surface
        tileTypeToTexture[2] = 2;  // Dirt variations
        tileTypeToTexture[3] = 3;
        tileTypeToTexture[4] = 4;
        tileTypeToTexture[5] = 5;
    }
    else if (currentLevel <= 1) {
        tileTypeToTexture[0] = 0;  // Empty
        tileTypeToTexture[1] = 1;  // Surface
        tileTypeToTexture[2] = 6;  // Different dirt textures
        tileTypeToTexture[3] = 7;
        tileTypeToTexture[4] = 8;
        tileTypeToTexture[5] = 9;
    }
    else if (currentLevel >= 2) {
        tileTypeToTexture[0] = 0;  // Empty
        tileTypeToTexture[1] = 1;  // Surface
        tileTypeToTexture[2] = 10; // Different dirt textures
        tileTypeToTexture[3] = 11;
        tileTypeToTexture[4] = 12;
        tileTypeToTexture[5] = 13;
    }
}

void Map::setCurrentLevel(int level) {
    currentLevel = level;
    setupTextureMapping(); // Only update texture mapping, not tile types
    buildTiles(); // Rebuild sprites with new textures
    std::cout << "Map level set to: " << currentLevel << std::endl;
}

bool Map::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open map file: " << filename << std::endl;
        return false;
    }

    std::string line;
    int row = 0;
    entitySpawns.clear();
    rockSpawns.clear(); // Clear previous rock spawns

    while (std::getline(file, line) && row < TILES_Y) {
        for (int col = 0; col < TILES_X && col < static_cast<int>(line.length()); col++) {
            char c = line[col];
            if (charToTileType.find(c) != charToTileType.end()) {
                tileData[row][col] = charToTileType[c];
                if (c == 'P' || c == '*') {
                    sf::Vector2f spawnPos(col * TILE_SIZE + TILE_SIZE / 2.0f, row * TILE_SIZE + TILE_SIZE / 2.0f);
                    entitySpawns.emplace_back(c, spawnPos);
                }
                else if (c == 'R') {
                    // Handle Rock spawn: get texture from tile to the right
                    sf::Vector2f spawnPos(col * TILE_SIZE + TILE_SIZE / 2.0f, row * TILE_SIZE + TILE_SIZE / 2.0f);
                    RockSpawnInfo rockInfo;
                    rockInfo.position = spawnPos;

                    // Determine texture index from the tile to the right
                    if (col + 1 < TILES_X) {
                        char tileRightChar = line[col + 1];
                        int tileRightType = 0;
                        if (charToTileType.find(tileRightChar) != charToTileType.end()) {
                            tileRightType = charToTileType[tileRightChar];
                        }
                        rockInfo.textureIndex = tileTypeToTexture[tileRightType];
                    }
                    else {
                        // Default to a specific texture if no tile to the right or out of bounds
                        rockInfo.textureIndex = tileTypeToTexture[1]; // Example: Use surface texture
                    }
                    rockSpawns.push_back(rockInfo);
                    tileData[row][col] = 6; // The 'R' tile itself is considered BEDROCK
                }
            }
            else {
                tileData[row][col] = 0;
            }
        }
        for (int col = static_cast<int>(line.length()); col < TILES_X; col++) {
            tileData[row][col] = 0;
        }
        row++;
    }

    for (int r = row; r < TILES_Y; r++) {
        for (int c = 0; c < TILES_X; c++) {
            tileData[r][c] = 0;
        }
    }

    file.close();
    buildTiles();
    std::cout << "Map loaded successfully from " << filename << std::endl;
    std::cout << "Found " << entitySpawns.size() << " entity spawns" << std::endl;
    std::cout << "Found " << rockSpawns.size() << " rock spawns" << std::endl;
    return true;
}

void Map::buildTiles() {
    tileSprites.clear();
    tileSprites.reserve(TILES_X * TILES_Y);

    for (int row = 0; row < TILES_Y; row++) {
        for (int col = 0; col < TILES_X; col++) {
            int tileType = tileData[row][col];
            if (tileType == 0) continue;

            // Use texture mapping to get the correct texture index
            int textureIndex = tileTypeToTexture[tileType];

            sf::Sprite sprite = tileSprite;
            sprite.setTextureRect(sf::IntRect({ textureIndex * TILE_SIZE, 0 }, { TILE_SIZE, TILE_SIZE }));
            sprite.setPosition(sf::Vector2f(col * TILE_SIZE, row * TILE_SIZE));
            tileSprites.push_back(sprite);
        }
    }
}

void Map::draw(sf::RenderWindow& window) {
    for (const auto& sprite : tileSprites) {
        window.draw(sprite);
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
    std::cout << "  Current level: " << currentLevel << std::endl;
}