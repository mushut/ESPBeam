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
	M1,
	M10,
	G1,
	G28,
	CLR
};

struct GCommand {
	code gCodeCommand;
	Point point();
	char pin[10];
	char penState[10];
};

#endif /* GCOMMAND_H_ */
