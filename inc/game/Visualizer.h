#ifndef VISUALIZER_H
#define VISUALIZER_H

#include "Config.h"
#include "Board.h"
#include "Object.h"
#include "Core.h"
#include "Unit.h"

class Visualizer
{
	public:
		static void visualizeGameState(unsigned long long tick);
};

#endif // VISUALIZER_H
