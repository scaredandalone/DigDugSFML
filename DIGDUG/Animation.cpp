#include "Animation.h"
#include <iostream>

Animation::Animation(sf::Texture* texture, sf::Vector2u imageCount, float switchTime, int sizeX, int sizeY, bool shouldLoop) : sizeX(16), sizeY(16)
{
    this->imageCount = imageCount;
    this->switchTime = switchTime;
    this->isLooping = shouldLoop;
    totalTime = 0.0f;
    currentImage.x = 0;
    currentImage.y = 0;
    animationComplete = false;
    uvRect.size.x = sizeX;
    uvRect.size.y = sizeY;
    uvRect.position = sf::Vector2i(0, 0);
    // debug std::cout << "Animation created - imageCount: " << imageCount.x << "x" << imageCount.y
    // debug     << ", switchTime: " << switchTime << ", size: " << sizeX << "x" << sizeY << ", looping: " << shouldLoop << std::endl;
}

Animation::~Animation()
{
}

void Animation::Update(int animationRow, float deltaTime, sf::Sprite& sprite)
{
    currentImage.y = animationRow;

    // Don't update if animation is complete and not looping
    if (animationComplete && !isLooping) {
        return;
    }

    totalTime += deltaTime;

    if (totalTime >= switchTime) {
        totalTime -= switchTime;
        currentImage.x++;

        if (currentImage.x >= imageCount.x) {
            if (isLooping) {
                currentImage.x = 0; // Loop back to first frame
            }
            else {
                currentImage.x = imageCount.x; // Stay on last frame
                animationComplete = true; // Animation completed one full cycle
            }
        }
        // debug std::cout << "Animation frame changed - currentImage: " << currentImage.x
        // debug     << ", row: " << currentImage.y << ", looping: " << isLooping << std::endl;
    }

    uvRect.position = sf::Vector2i(currentImage.x * uvRect.size.x, currentImage.y * uvRect.size.y);
    sprite.setTextureRect(uvRect);
}

bool Animation::IsAnimationComplete() const
{
    return animationComplete;
}

void Animation::ResetAnimation()
{
    currentImage.x = 0;
    currentImage.y = 0;
    totalTime = 0.0f;
    animationComplete = false;
}

void Animation::SetLooping(bool loop)
{
    isLooping = loop;
    if (loop && animationComplete) {
        // If we're setting it to loop and it was complete, reset it
        animationComplete = false;
    }
}

