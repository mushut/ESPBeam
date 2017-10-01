/*
 * StepperDriver.h
 *
 *  Created on: 29 Sep 2017
 *      Author: Eibl-PC
 */

#ifndef STEPPERDRIVER_H_
#define STEPPERDRIVER_H_

#include "DigitalIoPin.h"
#include "Point.h"

class StepperDriver {
public:
	StepperDriver();
	virtual ~StepperDriver();
	void moveTo(Point point);
private:
	//DigitalIoPin xStep = new DigitalIoPin();
	//DigitalIoPin yStep = new DigitalIoPin();
	//DigitalIoPin xDir = new DigitalIoPin();
	//DigitalIoPin yDir = new DigitalIoPin();

	//DigitalIoPin limitSw1 = new DigitalIoPin();
	//DigitalIoPin limitSw2 = new DigitalIoPin();
	//DigitalIoPin limitSw3 = new DigitalIoPin();
	//DigitalIoPin limitSw4 = new DigitalIoPin();
};

#endif /* STEPPERDRIVER_H_ */
