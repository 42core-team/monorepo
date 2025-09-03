#ifndef VISUALIZER_H
#define VISUALIZER_H

#include "Board.h"
#include "Config.h"
#include "Core.h"
#include "Object.h"
#include "Unit.h"

class Visualizer
{
  public:
	static void visualizeGameState(unsigned long long tick, bool force = false);
};

#endif // VISUALIZER_H
