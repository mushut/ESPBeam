/*
 * GCommand.h
 *
 *  Created on: 25 Aug 2017
 *      Author: Eibl-PC
 */

#ifndef GCOMMAND_H_
#define GCOMMAND_H_

#include "Point.h"

enum code{
	M1,		// Servo
	M10,	// Init
	G1,		// Plot
	G28,	// Unknown
	M4,		// Laser
	CLR		// Default
};

struct GCommand {
	code gCodeCommand;
	Point point;
	char penState[10];
};

#endif /* GCOMMAND_H_ */
