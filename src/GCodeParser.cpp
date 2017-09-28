/*
 * GCodeParser.cpp
 *
 *  Created on: 24 Aug 2017
 *      Author: Eibl-PC
 */

#include "GCodeParser.h"

GCodeParser::GCodeParser() {
	gCommandParts.clear();
}

GCodeParser::~GCodeParser() {
}

GCommand *GCodeParser::parseGCode(char gCodeRaw[]) {

	int i = 0,
			j = 0,
			x = 0;
	char code[6][10] = {0};

	// Parse until end of input
	for(i = 0; gCodeRaw[i] != 0; i++) {

		// Put chars in array until "\n" or space is read
		if(gCodeRaw[i] != 10 && gCodeRaw[i] != 32) {
			code[x][j++] = gCodeRaw[i];
		}

		// Write code to pointer array and reset code value
		else{
			x++;
			j = 0;
		}
	}

	// Create and return command

	/* Case "M10" (Initialisation)*/
	if(strcmp(code[0], "M10") == 0) {
		gCommand.gCodeCommand = M10;
	}

	/* Case "M1" (Servo)*/
	else if(strcmp(code[0], "M1") == 0) {
		gCommand.gCodeCommand = M1;
		strcpy(gCommand.penState, code[1]);
	}

	/* Case "G1" (Stepper)*/
	else if(strcmp(code[0], "G1") == 0) {

		// Initialise variables
		int i = 0;
		char xcoord[10] = {0},
			ycoord[10] = {0};

		// Parse x-coordinate
		for(i = 1; code[1][i] != 0; i++) {
			xcoord[i - 1] = code[1][i];
		}

		// Parse y-coordinate
		for(i = 1; code[2][i] != 0; i++) {
			ycoord[i - 1] = code[2][i];
		}

		// Convert coordinate from array to double
		double x = atof(xcoord),
				y = atof(ycoord);

		// Format command
		gCommand.gCodeCommand = G1;
		gCommand.point.setPoint(x, y);
	}

	/* Case "G28" */
	else if(strcmp(code[0], "G28") == 0) {
		gCommand.gCodeCommand = G28;
	}

	/* Just in case */
	else {
		gCommand.gCodeCommand = CLR;
	}

	return &gCommand;
}
