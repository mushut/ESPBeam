/*
 * StepperDriver.cpp
 *
 *  Created on: 29 Sep 2017
 *      Author: Eibl-PC
 */

#include <math.h>
#include <stdlib.h>
#include "StepperDriver.h"

using namespace std;

enum driveDirection {
	vertical,
	y_oriented,
	x_oriented
};

xSemaphoreHandle sbRIT = xSemaphoreCreateBinary();
static bool RIT_running;

DigitalIoPin *xLimit1;
DigitalIoPin *xLimit2;
DigitalIoPin *xStep;
DigitalIoPin *xDir;

DigitalIoPin *yLimit1;
DigitalIoPin *yLimit2;
DigitalIoPin *yStep;
DigitalIoPin *yDir;

bool isCalibrating;
bool isRunning;
bool xStepperDir;
bool yStepperDir;
bool xStepperPulse;
bool yStepperPulse;

static int initTime;
static int xTotalSteps;
static int yTotalSteps;
int xLimitsHit;
int yLimitsHit;
int xStepCount;
int yStepCount;

double stepperResolution = 0;	// Resolution gathered from calibration information
int direction = 0;

double realX1 = 0;	// millimeter value
double realX2 = 0;	// millimeter value
double realY1 = 0;	// millimeter value
double realY2 = 0;	// millimeter value
double delta_realX = 0;
double delta_realY = 0;

int appX1 = 0;
int appX2 = 0;
int appY1 = 0;
int appY2 = 0;
int delta_appX = 0;
int delta_appY = 0;
double app_slope = 0;

bool directionX = 0;
bool directionY = 0;

int currentY = 0;
int currentX = 0;

int i = 0;
int j = 0;

/* Start timer */
void RIT_start(int count, int us)
{
	uint64_t cmp_value;
	// Determine approximate compare value based on clock rate and passed interval
	cmp_value = (uint64_t) Chip_Clock_GetSystemClockRate() * (uint64_t) us / 1000000;
	// disable timer during configuration
	Chip_RIT_Disable(LPC_RITIMER);
	RIT_running = true;
	// enable automatic clear on when compare value==timer value
	// this makes interrupts trigger periodically
	Chip_RIT_EnableCompClear(LPC_RITIMER);
	// reset the counter
	Chip_RIT_SetCounter(LPC_RITIMER, 0);
	Chip_RIT_SetCompareValue(LPC_RITIMER, cmp_value);
	// start counting
	Chip_RIT_Enable(LPC_RITIMER);
	// Enable the interrupt signal in NVIC (the interrupt controller)
	NVIC_EnableIRQ(RITIMER_IRQn);
	// wait for ISR to tell that we're done
	if(xSemaphoreTake(sbRIT, portMAX_DELAY) == pdTRUE) {
		// Disable the interrupt signal in NVIC (the interrupt controller)
		NVIC_DisableIRQ(RITIMER_IRQn);
	}
	else {
		// unexpected error
	}
}

/* Update timer */
void RIT_update(int us) {

	uint64_t cmp_value = (uint64_t) Chip_Clock_GetSystemClockRate() * (uint64_t) us / 1000000; // Calculate compare value
	Chip_RIT_Disable(LPC_RITIMER);						// Disable timer
	Chip_RIT_EnableCompClear(LPC_RITIMER);				// Set clear on comparison match
	Chip_RIT_SetCounter(LPC_RITIMER, 0);				// Reset counter
	Chip_RIT_SetCompareValue(LPC_RITIMER, cmp_value);	// Give timer a new compare value
	Chip_RIT_Enable(LPC_RITIMER);						// Enable timer
}

/* Check all limit switches */
bool checkLimits() {
	if(xLimit1->read() && xStepperDir == false) return false;
	else if(xLimit2->read() && xStepperDir == true) return false;
	else if(yLimit1->read() && yStepperDir == false) return false;
	else if(yLimit2->read() && yStepperDir == true) return false;
	else return true;
}

/* RIT IRQ Handler */
extern "C" {
void RIT_IRQHandler(void)
{
	// This used to check if a context switch is required
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;
	// Tell timer that we have processed the interrupt.
	// Timer then removes the IRQ until next match occurs
	Chip_RIT_ClearIntStatus(LPC_RITIMER); // clear IRQ flag

	if(RIT_running) {

		/* Running */
		if(isRunning) {

			/* Check if any limit is triggered */
			if(checkLimits()){

				double slope = delta_realY / delta_realX;
				double realY0 = realY1 - slope * realX1;
				double yDifference = currentY - (slope * currentX + realY0);
				double xDifference = currentX - (currentY - realY0) / slope;

				if(directionY == false) yDifference = -yDifference;
				if(directionX == false) xDifference = -xDifference;

				switch(direction) {
				case vertical:
					if(j < abs(delta_appY)) {
						yStep->write(yStepperPulse);
						yStepperPulse = !yStepperPulse;

						if(delta_appY != 0) {
							if(directionY) currentY++;
							else currentY--;
						}


						j++;
					}
					else {
						i = delta_appX;
					}
					break;

				case x_oriented:
					if(i < abs(delta_appX)) {

						if (yDifference <= 0) {
							yStep->write(yStepperPulse);
							yStepperPulse = !yStepperPulse;

							// Change current position depending on the direction
							if(directionY) currentY++;
							else currentY--;

							j++;
						}
						xStep->write(xStepperPulse);
						xStepperPulse = !xStepperPulse;

						// Change current position depending on the direction
						if(directionX) currentX++;
						else currentX--;

						i++;
					}
					else {
						if(j < abs(delta_appY)){
							yStep->write(yStepperPulse);
							yStepperPulse = !yStepperPulse;

							// Change current position depending on the direction
							if(directionY) currentY++;
							else currentY--;

							j++;
						}
					}
					break;

				case y_oriented:
					if(j < abs(delta_appY)) {

						if (xDifference <= 0) {
							xStep->write(xStepperPulse);
							xStepperPulse = !xStepperPulse;

							// Change current position depending on the direction
							if(directionX) currentX++;
							else currentX--;

							i++;
						}
						yStep->write(yStepperPulse);
						yStepperPulse = !yStepperPulse;

						// Change current position depending on the direction
						if(directionY) currentY++;
						else currentY--;

						j++;
					}
					else {
						if(i < abs(delta_appX)){

							xStep->write(xStepperPulse);
							xStepperPulse = !xStepperPulse;

							// Change current position depending on the direction
							if(directionX) currentX++;
							else currentX--;

							i++;
						}
					}
					break;
				}
			}

			if(i >= abs(delta_appX) && j >= abs(delta_appY)) {
				RIT_running = false;
			}
		}

		/* Calibration */
		if(isCalibrating) {

			/* X-AXIS */
			if(xLimitsHit < 2) {

				// Run
				xDir->write(xStepperDir);
				xStep->write(xStepperPulse);
				xStepperPulse = !xStepperPulse;

				// Run to both ends
				if(xLimit1->read() && (xStepperDir == false)){
					xTotalSteps = xStepCount;
					xStepCount = 0;
					xLimitsHit++;
					xStepperDir = true;
				}
				else if(xLimit2->read() && (xStepperDir == true)) {
					xTotalSteps = xStepCount;
					xStepCount = 0;
					xLimitsHit++;
					xStepperDir = false;
				}

				xStepCount++;
			}

			/* Y-AXIS */
			if(yLimitsHit < 2) {

				// Run
				yDir->write(yStepperDir);
				yStep->write(yStepperPulse);
				yStepperPulse = !yStepperPulse;

				// Run to both ends
				if(yLimit1->read() && (yStepperDir == false)){
					yTotalSteps = yStepCount;
					yStepCount = 0;
					yLimitsHit++;
					yStepperDir = true;
				}
				else if(yLimit2->read() && (yStepperDir == true)) {
					yTotalSteps = yStepCount;
					yStepCount = 0;
					yLimitsHit++;
					yStepperDir = false;
				}

				yStepCount++;
			}

			// Finish calibration
			if(xLimitsHit >= 2 && yLimitsHit >= 2) {
				RIT_running = false;
			}

		}
	}
	else {
		Chip_RIT_Disable(LPC_RITIMER); // disable timer
		// Give semaphore and set context switch flag if a higher priority task was woken up
		xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityWoken);
	}
	// End the ISR and (possibly) do a context switch
	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}
}

/* Constructor */
StepperDriver::StepperDriver() {

	yLimit1 = new DigitalIoPin(1, 3, DigitalIoPin::pullup, true);
	yLimit2 = new DigitalIoPin(0, 0, DigitalIoPin::pullup, true);
	yStep = new DigitalIoPin(0, 27, DigitalIoPin::output, false);
	yDir = new DigitalIoPin(0, 28, DigitalIoPin::output, false);

	xLimit1 = new DigitalIoPin(0, 29, DigitalIoPin::pullup, true);
	xLimit2 = new DigitalIoPin(0, 9, DigitalIoPin::pullup, true);
	xStep = new DigitalIoPin(0, 24, DigitalIoPin::output, false);
	xDir = new DigitalIoPin(1, 0, DigitalIoPin::output, false);

	isCalibrating = true;
	isRunning = false;
	xStepperDir = true;
	yStepperDir = true;
	xStepperPulse = true;
	yStepperPulse = true;

	initTime = 150;
	xTotalSteps = 0;
	yTotalSteps = 0;
	xLimitsHit = 0;
	yLimitsHit = 0;
	xStepCount = 0;
	yStepCount = 0;

}

/* Destructor */
StepperDriver::~StepperDriver() {
	// TODO Auto-generated destructor stub
}

/* Plot */
void StepperDriver::plot(Point point) {

	realX2 = point.getPointX() / stepperResolution;	// Steps
	realY2 = point.getPointY() / stepperResolution;	// Steps
	delta_realX = realX2 - realX1;
	delta_realY = realY2 - realY1;

	appX1 = (int)round(realX1);
	appX2 = (int)round(realX2);
	appY1 = (int)round(realY1);
	appY2 = (int)round(realY2);
	delta_appX = appX2 - appX1;
	delta_appY = appY2 - appY1;

	directionX = 0;
	directionY = 0;

	// Direction
	if (delta_appX > 0) {
		directionX = true;
	}
	else if (delta_appX < 0) {
		directionX = false;
	}

	if (delta_appY > 0) {
		directionY = true;
	}
	else if (delta_appY < 0) {
		directionY = false;
	}

	xDir->write(directionX);
	yDir->write(directionY);

	// Convert values to absolute values
	//	delta_appX = abs(delta_appX);
	//	delta_appY = abs(delta_appY);
	//	delta_realX = abs(delta_realX);
	//	delta_realY = abs(delta_realY);

	// Up or down
	if (delta_appX == 0) {
		direction = vertical;
	}

	// Diagonal or horizontal movement
	else if (delta_appX != 0) {
		app_slope = ((double)delta_appY) / delta_appX;

		if (app_slope >= 1 || app_slope <= -1) {
			direction = y_oriented;
		}

		else if (app_slope < 1 && app_slope > -1) {
			direction = x_oriented;
		}
	}

	RIT_running = true;

	RIT_start(10000, initTime);

	realX1 = realX2;
	realY1 = realY2;
	i = 0;
	j = 0;
}

/* Calibrate plotter */
void StepperDriver::calibrate() {
	isCalibrating = true;
	isRunning = false;

	RIT_start(10000, 150);

	stepperResolution = (((310.0 / xTotalSteps) + (380.0 / yTotalSteps)) / 2);

	isCalibrating = false;
	isRunning = true;
}

