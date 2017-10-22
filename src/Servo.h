/*
 * Servo.h
 *
 *  Created on: 9 Oct 2017
 *      Author: Eibl-PC
 */

#ifndef SERVO_H_
#define SERVO_H_

class Servo {

	enum position {
		down,
		up
	};

public:
	Servo();
	virtual ~Servo();
	void rotate(char* angle);
	static bool isDown();
private:
	static int pos;
};

#endif /* SERVO_H_ */
