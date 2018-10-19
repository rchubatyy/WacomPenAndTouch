#pragma once

#ifndef WIDGET_H_
#define WIDGET_H_

#include "ofMain.h"

//#define OFFSET 25
#define DEFAULT_WIDTH 200
#define DEFAULT_HEIGHT 200
#define DEFAULT_COLOR ofColor::ivory

class Widget {
private:
	int _startX, _startY, _width, _height;
	float _angle;
	int _id;
	bool _notResizable = false;
	bool _showBar;
	float _sliderPos;
	//bool _canDraw = true;
	int _currentImage;
	ofImage image;
	static int qt;
	ofColor _color = ofColor::ivory;
protected:

public:
	Widget();
	Widget(int startX, int startY);
	Widget(int startX, int startY, int width, int height, float angle);
	~Widget();
	vector<ofPoint> pointsDrawn;
	vector<ofColor> colorsDrawn;
	vector<float> pointPressures;
	vector<string> imagePaths; 
	//vector<ofImage> images;
	static int getQt();
	bool nowDrawing;
	int getId() const;
	ofPoint getStartPoint() const;
	ofPoint getDim() const;
	float getAngle() const;
	void setup(int startX, int startY, int width, int height, float angle);
	void scroll(int x, int y);
	void moveTo(int x, int y);
	void relatResize(int x, int y);
	void resize(int x, int y);
	void relatRotate(float a);
	void resetAngle();
	void showBar();
	void hideBar();
	void drawBar();
	void moveSlider();
	void setSlider(float pos);
	float getSlider() const;
	bool sliderOn() const;
	void update();
	void draw();
	void drawPenData();
	int getCurrentImage() const;
	bool setCurrentImage(int index);
	//bool pointInsideWidget(ofPoint point);
	bool pointInsideWidget(const ofPoint& point);
	//bool pointInsideWidget(const ofPoint& p1, const ofPoint& p2);
	void repaint(ofColor color);
	friend class ofApp;
};
#endif
