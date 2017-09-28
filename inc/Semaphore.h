/*
 * Semaphore.h
 *
 *  Created on: 28 Aug 2017
 *      Author: Eibl-PC
 */

#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include "FreeRTOS.h"
#include "semphr.h"

class Semaphore {
public:
	enum semaphoreType {
		binary,
		counting,
		mutex
	};
	Semaphore(semaphoreType type);
	virtual ~Semaphore();
	void take();
	void give();
	bool isAvailable();
private:
	xSemaphoreHandle xSemaphore;
};

#endif /* SEMAPHORE_H_ */
