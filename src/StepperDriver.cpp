/*
 * StepperDriver.cpp
 *
 *  Created on: 29 Sep 2017
 *      Author: Eibl-PC
 */

#include <math.h>
#include "StepperDriver.h"

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
bool stepperPulse;

static int initTime;
static int xTotalSteps;
static int yTotalSteps;
int xLimitsHit;
int yLimitsHit;
int xStepCount;
int yStepCount;
int x;
int y;
int prev_x = 0;
int prev_y = 0;

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

			double realX1;	// millimeter value
			double realX2;	// millimeter value
			double realY1;	// millimeter value
			double realY2;	// millimeter value
			double delta_realX = realX2 - realX1;
			double delta_realY = realY2 - realY1;

			double StepperResolution;	// Resolution gathered from calibration information
			int appX1 = (int)round(realX1/StepperResolution);
			int appX2 = (int)round(realX2/StepperResolution);
			int appY1 = (int)round(realY1/StepperResolution);
			int appY2 = (int)round(realY2/StepperResolution);
			int delta_appX = appX2 - appX1;
			int delta_appY = appY2 - appY1;
			double app_slope = 0;

			bool directionX = 0;
			bool directionY = 0;

			int currentY = appY1;
			int currentX = appX1;

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

			// Up or down
			if (delta_appX == 0) {
				// Drive only up or down
			}

			// Left or right
			if (delta_appY == 0) {
				// Drive only left or right
			}

			// Diagonal movement
			if (delta_appX != 0) {
				app_slope = ((double)delta_appY) / delta_appX;

				if (app_slope > 1 || app_slope < -1) {
					// drive x before y

					// LOOP
					int i = 0
					while (i < delta_appX) {
						if (i == 0) {
							driveX();
							i++;
							currentX++;
						}
						else {
							if ((currentY - (delta_realY / delta_realX) * currentX + (realY1 - delta_realY * realX1 / delta_realX)) > 0) {
								driveY();
								currentY++;
							}
							driveX();
							i++;
							currentX++;
						}
					}
				}
				else if (app_slope < 1 && app_slope > -1) {
					// drive y before x

					int i = 0
					while (i < delta_appY) {
						if (i == 0) {
							driveY();
							i++;
							currentY++;
						}
						else {
							if ((currentX - (currentY - realY1 - delta_realY * realX1 / delta_realX) * delta_realX / delta_realY) > 0) {
								driveY();
								currentY++;
							}
							driveX();
							i++;
							currentX++;
						}
					}
				}
			}

			// Last commands
			prev_x = x;
			prev_y = y;
		}

		/* Calibration */
		if(isCalibrating) {

			/* X-AXIS */

			if(xLimitsHit < 2) {

				// Run
				xDir->write(xStepperDir);
				xStep->write(stepperPulse);

				// Run to both ends
				if(xLimit1->read() && (xStepperDir == true)){
					xTotalSteps = xStepCount;
					xStepCount = 0;
					xLimitsHit++;
					xStepperDir = false;
				}
				else if(xLimit2->read() && (xStepperDir == false)) {
					xTotalSteps = xStepCount;
					xStepCount = 0;
					xLimitsHit++;
					xStepperDir = true;
				}

			}

			/* Y-AXIS */

			if(yLimitsHit < 2) {

				// Run
				yDir->write(yStepperDir);
				yStep->write(stepperPulse);

				// Run to both ends
				if(yLimit1->read() && (yStepperDir == true)){
					yTotalSteps = yStepCount;
					yStepCount = 0;
					yLimitsHit++;
					yStepperDir = false;
				}
				else if(yLimit2->read() && (yStepperDir == false)) {
					yTotalSteps = yStepCount;
					yStepCount = 0;
					yLimitsHit++;
					yStepperDir = true;
				}
			}

			// Finish calibration
			if(xLimitsHit >= 2 && yLimitsHit >= 2) {
				RIT_running = false;
			}

		}

		// Toggle pulse
		stepperPulse = !stepperPulse;

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

	xLimit1 = new DigitalIoPin(1, 9, DigitalIoPin::pullup, true);
	xLimit2 = new DigitalIoPin(1, 10, DigitalIoPin::pullup, true);
	xStep = new DigitalIoPin(0, 9, DigitalIoPin::output, false);
	xDir = new DigitalIoPin(0, 29, DigitalIoPin::output, false);

	yLimit1 = new DigitalIoPin(0, 27, DigitalIoPin::pullup, true);
	yLimit2 = new DigitalIoPin(0, 28, DigitalIoPin::pullup, true);
	yStep = new DigitalIoPin(0, 24, DigitalIoPin::output, false);
	yDir = new DigitalIoPin(1, 0, DigitalIoPin::output, false);

	isCalibrating = true;
	isRunning = false;
	xStepperDir = true;
	yStepperDir = true;
	stepperPulse = true;

	initTime = 700;
	xTotalSteps = 0;
	yTotalSteps = 0;
	xLimitsHit = 0;
	yLimitsHit = 0;
	xStepCount = 0;
	yStepCount = 0;
	x = 0;
	y = 0;

}

/* Destructor */
StepperDriver::~StepperDriver() {
	// TODO Auto-generated destructor stub
}

/* Plot */
void StepperDriver::plot(Point point) {
	x = point.getPointX();
	y = point.getPointY();
	RIT_start(10000, initTime);
}

/* Calibrate plotter */
void StepperDriver::calibrate() {
	isCalibrating = true;
	isRunning = false;

	RIT_start(10000, initTime);

	isCalibrating = false;
	isRunning = true;
}
