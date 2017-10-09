/*
 * Servo.h
 *
 *  Created on: 9 Oct 2017
 *      Author: Eibl-PC
 */

#ifndef SERVO_H_
#define SERVO_H_

class Servo {
public:
	Servo();
	virtual ~Servo();
	void rotate(char* angle);
};

#endif /* SERVO_H_ */
