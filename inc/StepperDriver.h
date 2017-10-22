/*
 * StepperDriver.h
 *
 *  Created on: 29 Sep 2017
 *      Author: Eibl-PC
 */

#ifndef STEPPERDRIVER_H_
#define STEPPERDRIVER_H_

#include "Point.h"

class StepperDriver {
public:
	StepperDriver();
	virtual ~StepperDriver();
	void plot(Point point);
	void calibrate();
	void reset();
	static void setTime(int time);
private:
	char report[96];
};

#endif /* STEPPERDRIVER_H_ */
