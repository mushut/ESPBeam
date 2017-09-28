/*
 * Semaphore.cpp
 *
 *  Created on: 28 Aug 2017
 *      Author: Eibl-PC
 */

#include "Semaphore.h"

Semaphore::Semaphore(semaphoreType type) {
	switch(type) {
	case binary:
		xSemaphore = xSemaphoreCreateBinary();
		break;
	case counting:
		xSemaphore = xSemaphoreCreateCounting( 10, 0 );
		break;
	case mutex:
		xSemaphore = xSemaphoreCreateMutex();
		break;
	default:
		xSemaphore = xSemaphoreCreateMutex();
		break;
	}

}

Semaphore::~Semaphore() {
	// TODO Auto-generated destructor stub
}

void Semaphore::take() {
	xSemaphoreTake(xSemaphore, portMAX_DELAY);
}

void Semaphore::give() {
	xSemaphoreGive(xSemaphore);
}

bool Semaphore::isAvailable() {
	if(xSemaphoreTake(xSemaphore, portMAX_DELAY)){
		return true;
	}
	return false;
}
