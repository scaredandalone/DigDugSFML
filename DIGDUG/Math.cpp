#include "Math.h"

bool Math::CheckHitboxCollision(const sf::FloatRect& rect1, const sf::FloatRect& rect2)
{
	return rect1.findIntersection(rect2).has_value();
}