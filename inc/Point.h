/*
 * Point.h
 *
 *  Created on: 15 Sep 2017
 *      Author: Eibl-PC
 */

#ifndef POINT_H_
#define POINT_H_

class Point {
public:
	Point();
	Point(double x_, double y_);
	virtual ~Point();
	void setPoint(double x_, double y_);
private:
	double x;
	double y;
};

#endif /* POINT_H_ */
