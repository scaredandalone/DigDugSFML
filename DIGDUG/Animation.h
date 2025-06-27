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

private:
	sf::Vector2u imageCount;
	sf::Vector2u currentImage;

	float totalTime;
	float switchTime;

	int sizeX;
	int sizeY;
};

