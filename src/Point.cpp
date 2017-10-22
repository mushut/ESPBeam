/*
 * Point.cpp
 *
 *  Created on: 15 Sep 2017
 *      Author: Eibl-PC
 */

#include "Point.h"

Point::~Point() {
	// TODO Auto-generated destructor stub
}

void Point::setPoint(double x_, double y_) {
	x = x_;
	y = y_;
}

double Point::getPointX() {
	return x;
}

double Point::getPointY() {
	return y;
}

Point::Point() {
}
