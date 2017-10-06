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
#include "chip.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

class StepperDriver {
public:
	StepperDriver();
	virtual ~StepperDriver();
	void plot(Point point);
	void calibrate();
};

#endif /* STEPPERDRIVER_H_ */
