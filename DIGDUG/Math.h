#pragma once
#include <SFML/Graphics.hpp>
class Math
{
public:
	static bool CheckHitboxCollision(const sf::FloatRect& rect1, const sf::FloatRect& rect2);
};

