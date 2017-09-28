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
	char part[10] = {0};

	// Parse until end of input
	for(i = 0; gCodeRaw[i] != 0; i++) {

		// Put chars in array until "\n" or space is read
		if(gCodeRaw[i] != 10 && gCodeRaw[i] != 32) {
			code[x][j++] = gCodeRaw[i];
		}

		// Write code to pointer array and reset code value
		else{
			x++;
//			strcpy(code[x++],part);
			j = 0;
//			memset(part, 0, sizeof(part));
		}
	}

	// Create and return command

	/* Case "M10" */
	if(strcmp(code[0], "M10") == 0) {
		gCommand.gCodeCommand = M10;
	}

	/* Case "M1" */
	else if(strcmp(code[0], "M1") == 0) {
		gCommand.gCodeCommand = M1;
		strcpy(gCommand.penState, code[1]);
	}

	/* Case "G1" */
	else if(strcmp(code[0], "G1") == 0) {
//		char *xcoord = code[1]++,
//				*ycoord = code[2]++;
//
//		double x = atof(xcoord),
//				y = atof(ycoord);
//
//		gCommand.gCodeCommand = G1;
//		gCommand.point.setPoint(x, y);
	}

	/* Case "G28" */
	else if(strcmp(code[0], "G28") == 0) {
		gCommand.gCodeCommand = G28;
	}

	return &gCommand;
}
