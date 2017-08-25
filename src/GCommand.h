/*
 * GCommand.h
 *
 *  Created on: 25 Aug 2017
 *      Author: Eibl-PC
 */

#ifndef GCOMMAND_H_
#define GCOMMAND_H_

class GCommand {
public:
	GCommand(char codeInput[5], char xCoordInput[10], char yCoordInput[10], char pin[5]);
	GCommand(char codeInput[5], char penStateInput[5]);
	virtual ~GCommand();
private:
	char code[5];
	char xCoord[10];
	char yCoord[10];
	char pin[5];
	char penState[5];
};

#endif /* GCOMMAND_H_ */
