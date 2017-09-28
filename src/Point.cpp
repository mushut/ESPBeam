/*
 * Point.cpp
 *
 *  Created on: 15 Sep 2017
 *      Author: Eibl-PC
 */

#include <math.h>
#include "Point.h"

Point::~Point() {
	// TODO Auto-generated destructor stub
}

Point::Point(double x_, double y_) {
	// TODO Auto-generated constructor stub

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

double Point::distance(Point secondPoint) {
	double result = sqrt(pow((secondPoint.getPointX() - pointX),2)+pow((secondPoint.getPointY() - pointY),2));
}

Point::Point() {
}
