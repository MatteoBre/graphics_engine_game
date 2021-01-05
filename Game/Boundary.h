#pragma once
#include "maths_funcs.h"
#include <iostream>

class Square {
public:
	Square(vec2 topRight, vec2 bottomLeft) {
		this->topRight = topRight;
		this->bottomLeft = bottomLeft;
	}

	bool isInSquare(vec2 point) {
		// Check X
		if (point.v[0] > bottomLeft.v[0] || point.v[0] < topRight.v[0]) {
			return false;
		}

		// Check Z
		if (point.v[1] > bottomLeft.v[1] || point.v[1] < topRight.v[1]) {
			return false;
		}
		return true;
	}
private:
	vec2 topRight;
	vec2 bottomLeft;
};

class Boundary
{
	public:
		Boundary(std::vector<Square> squares) {
			this->squares = squares;
		}

		bool isInBoundary(vec2 point) {
			bool result;
			for (int i = 0; i < squares.size(); i++) {
				result = squares.at(i).isInSquare(point);
				if (result)
					return true;
			}
			return false;
		}

		std::vector<Square> squares;
};
