#pragma once
#include <string>
#include <vector>

class StageManager
{
private:
	std::vector<std::string> mapFiles;
	std::string mapDirectory; 
    int currentStage = 0;

public:
		StageManager(const std::string& directory = "Assets/Map");
        void loadMapList();
        void addMapFile(const std::string& filename);
        std::string getMapFile(int level) const;
        std::string loadMapData(int level) const;
        std::vector<std::string> loadMapLines(int level) const;
        int getMapCount() const;
        const std::vector<std::string>& getMapFiles() const;
        void printAvailableMaps() const;
        void incrementStage();
        void setCurrentStage(int level);
        int getCurrentStage() const { return currentStage; }
        
};

