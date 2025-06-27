#include "Animation.h"
#include <iostream>



Animation::Animation(sf::Texture* texture, sf::Vector2u imageCount, float switchTime, int sizeX, int sizeY) : sizeX(16), sizeY(16)
{
    this->imageCount = imageCount;
    this->switchTime = switchTime;

    totalTime = 0.0f;
    currentImage.x = 0;

    uvRect.size.x = sizeX;
    uvRect.size.y = sizeY;

  // debug std::cout << "Animation created - imageCount: " << imageCount.x << "x" << imageCount.y
  // debug     << ", switchTime: " << switchTime << ", size: " << sizeX << "x" << sizeY << std::endl;
}

Animation::~Animation()
{
}

void Animation::Update(int animationRow, float deltaTime, sf::Sprite& sprite)
{
    currentImage.y = animationRow;

    totalTime += deltaTime;

    if (totalTime >= switchTime) {
        totalTime -= switchTime;
        currentImage.x++;
        if (currentImage.x >= imageCount.x) {
            currentImage.x = 0;
        }

       // debug std::cout << "Animation frame changed - currentImage: " << currentImage.x
       // debug     << ", row: " << currentImage.y << std::endl;
    }
    uvRect.position = sf::Vector2i(currentImage.x * uvRect.size.x, currentImage.y * uvRect.size.y);
    sprite.setTextureRect(uvRect);
}
