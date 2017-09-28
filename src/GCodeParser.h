/*
 * GCodeParser.h
 *
 *  Created on: 24 Aug 2017
 *      Author: Eibl-PC
 */

#ifndef GCODEPARSER_H_
#define GCODEPARSER_H_
using namespace std;

#include <Cstring>
#include <string>
#include <vector>
#include "GCommand.h"
#include "stdio.h"
#include "stdlib.h"
#include "ctype.h"

class GCodeParser {
public:
	GCodeParser();
	virtual ~GCodeParser();

	GCommand *parseGCode(char gCodeRaw[50]);
private:
	GCommand gCommand;
	vector<string> gCommandParts;
};

#endif /* GCODEPARSER_H_ */
