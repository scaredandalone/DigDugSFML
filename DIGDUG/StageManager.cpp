#include "StageManager.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>

StageManager::StageManager(const std::string& directory) : mapDirectory(directory) {
    loadMapList();
}

void StageManager::loadMapList() {
    mapFiles.clear();

    try {
        for (const auto& entry : std::filesystem::directory_iterator(mapDirectory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".rmap") {
                mapFiles.push_back(entry.path().string());
            }
        }

        std::sort(mapFiles.begin(), mapFiles.end());

        std::cout << "Loaded " << mapFiles.size() << " map files." << std::endl;
    }
    catch (const std::filesystem::filesystem_error& ex) {
        std::cerr << "Error loading maps from directory: " << ex.what() << std::endl;
    }
}

void StageManager::addMapFile(const std::string& filename) {
    std::string fullPath = mapDirectory + filename;
    mapFiles.push_back(fullPath);
}

std::string StageManager::getMapFile(int level) const {
    if (level < 0 || level >= mapFiles.size()) {
        std::cerr << "Invalid level: " << level << ". Available levels: 0-" << (mapFiles.size() - 1) << std::endl;
        return "";
    }
    return mapFiles[level];
}

std::string StageManager::loadMapData(int level) const {
    std::string filename = getMapFile(level);
    if (filename.empty()) {
        return "";
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open map file: " << filename << std::endl;
        return "";
    }

    std::string mapData;
    std::string line;
    while (std::getline(file, line)) {
        mapData += line + "\n";
    }
    file.close();

    return mapData;
}

std::vector<std::string> StageManager::loadMapLines(int level) const {
    std::string filename = getMapFile(level);
    std::vector<std::string> lines;

    if (filename.empty()) {
        return lines;
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open map file: " << filename << std::endl;
        return lines;
    }

    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    file.close();

    return lines;
}

int StageManager::getMapCount() const {
    return mapFiles.size();
}

const std::vector<std::string>& StageManager::getMapFiles() const {
    return mapFiles;
}

void StageManager::printAvailableMaps() const {
    std::cout << "Available maps:" << std::endl;
    for (int i = 0; i < mapFiles.size(); ++i) {
        std::cout << "Level " << i << ": " << mapFiles[i] << std::endl;
    }
}