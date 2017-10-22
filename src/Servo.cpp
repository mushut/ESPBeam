/*
 * Servo.cpp
 *
 *  Created on: 9 Oct 2017
 *      Author: Eibl-PC
 */

#include "Servo.h"
#include "chip.h"
#include "string.h"
#include "StepperDriver.h"

int Servo::pos;

void SCT_Init() {
	Chip_SWM_MovablePortPinAssign(SWM_SCT0_OUT0_O, 0, 10);
	Chip_SCT_Init(LPC_SCTLARGE0);

	LPC_SCTLARGE0->CONFIG |= (1 << 17); // two 16-bit timers, auto limit
	LPC_SCTLARGE0->CTRL_L |= (36-1) << 5; // set prescaler, SCTimer/PWM clock = 1 MHz

	LPC_SCTLARGE0->MATCHREL[0].L = 40000-1; // match 0 @ 10/12MHz = 10 usec (100 kHz PWM freq)
	LPC_SCTLARGE0->MATCHREL[1].L = 3000; // match 1 used for duty cycle (in 10 steps)

	LPC_SCTLARGE0->EVENT[0].STATE = 0xFFFFFFFF; // event 0 happens in all states
	LPC_SCTLARGE0->EVENT[0].CTRL = (1 << 12); // match 0 condition only

	LPC_SCTLARGE0->EVENT[1].STATE = 0xFFFFFFFF; // event 1 happens in all states
	LPC_SCTLARGE0->EVENT[1].CTRL = (1 << 0) | (1 << 12); // match 1 condition only

	LPC_SCTLARGE0->OUT[0].SET = (1 << 0); // event 0 will set SCTx_OUT0
	LPC_SCTLARGE0->OUT[0].CLR = (1 << 1); // event 1 will clear SCTx_OUT0

	LPC_SCTLARGE0->CTRL_L &= ~(1 << 2); // unhalt it by clearing bit 2 of CTRL reg
}

Servo::Servo() {
	SCT_Init();
}

Servo::~Servo() {
	// TODO Auto-generated destructor stub
}

void Servo::rotate(char* angle) {

	// Down
	if(strcmp(angle, "90") == 0) {
		LPC_SCTLARGE0->MATCHREL[1].L = 3000;
		pos = down;
	}

	// Up
	else if(strcmp(angle, "160") == 0) {
		LPC_SCTLARGE0->MATCHREL[1].L = 2200;
		pos = up;
	}
}

bool Servo::isDown() {
	if(pos == down) {
		return true;
	}
	return false;
}
