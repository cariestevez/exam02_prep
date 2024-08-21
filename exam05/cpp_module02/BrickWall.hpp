#ifndef BRICKWALL_HPP
#define BRICKWALL_HPP

#include <iostream>
#include "ATarget.hpp"

class ATarget;

class BrickWall : public ATarget
{
	public:
		BrickWall();
		virtual ~BrickWall();

		virtual ATarget *clone() const;
};

#endif
