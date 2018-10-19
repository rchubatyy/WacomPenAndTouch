#pragma once

#ifndef PENTHREAD_H_
#define PENTHREAD_H_

#include "ofMain.h"
#include "MyTabletObserver.h"

//#define ERASER_EPSILON 2.5

class PenThread : public ofThread {
public:
	void start();
	void stop();
	void threadedFunction();
	void getPenData();
	void processPenData();
	float getPenAngle();
	float getPenVAngle();
	float getPenOrientation();
	float getPenPositiveOrientation();
	float pressure, penX, penY, rotX, rotY, rotZ, rotAngle;
	bbTabletDevice &td = bbTabletDevice::getInstance();
	MyTabletObserver g_callback;
	bbTabletEvent evt;
	int penSide, pressedButtons;
	float oldAngle, oldOrientation, tiltCount;
	ofPoint penPos;
	ofPoint rotAxis;
	long droppedEvents = 0;
	long fails;
	bool drawing;
	bool selectingColor;
	bool openedFromSwipe;
	bool secondPointChosen;
	bool canStraightOrCurve, doneStraightOrCurve;
	ofPoint straightOrCurve;
	short colorSelected;
	//float oldSat, oldBri, satCount, briCount;
	void getSaturationFromTilt();
	void getBrightnessFromTilt();
	bool picked, adjusting, movingSlider;
	int touchAndPenOnWidget, wFingIndex;
	ofPoint lastPointOnWidget;
	int ct;
	//float ct;
};

#endif
