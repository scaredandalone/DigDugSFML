#pragma once
#include <SFML/Graphics.hpp>
class Animation
{
public:
	sf::IntRect uvRect;

public:
	Animation(sf::Texture* texture, sf::Vector2u imageCount, float switchTime, int sizeX, int sizeY);
	~Animation();

	void Update(int animationRow, float deltaTime, sf::Sprite& sprite);
	sf::Vector2u currentImage;
	sf::Vector2u imageCount;
private:

	float totalTime;
	float switchTime;

	int sizeX;
	int sizeY;
};

