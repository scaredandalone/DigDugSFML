#pragma once
#include <SFML/Graphics.hpp>
class Animation
{
public:
	sf::IntRect uvRect;

public:
	Animation(sf::Texture* texture, sf::Vector2u imageCount, float switchTime, int sizeX, int sizeY, bool shouldLoop = true);
	~Animation();

	void Update(int animationRow, float deltaTime, sf::Sprite& sprite);
	sf::Vector2u currentImage;
	sf::Vector2u imageCount;

	bool IsAnimationComplete() const;
	void ResetAnimation();
	void SetLooping(bool loop);
private:
	bool animationComplete;
	bool isLooping;

	float totalTime;
	float switchTime;

	int sizeX;
	int sizeY;
};