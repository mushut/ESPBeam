/*
 * GCodeParser.h
 *
 *  Created on: 24 Aug 2017
 *      Author: Eibl-PC
 */

#ifndef GCODEPARSER_H_
#define GCODEPARSER_H_

#include "GCommand.h"

class GCodeParser {
public:
	GCodeParser();
	virtual ~GCodeParser();
	GCommand *parseGCode(char input[30]);

private:
	GCommand gCommand;
};

#endif /* GCODEPARSER_H_ */
