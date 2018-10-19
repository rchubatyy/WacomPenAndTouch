#include "Widget.h"

int Widget::qt = 0;



Widget::Widget(){
	_startX = 50;
	_startY = 50;
	_width = DEFAULT_WIDTH;
	_height = DEFAULT_HEIGHT;
	_angle = 0;
	_color = DEFAULT_COLOR;
	qt++;
	_id = qt;
	_currentImage = 0;
}

Widget::Widget(int startX, int startY) {
	_startX = startX;
	_startY = startY;
	_width = DEFAULT_WIDTH;
	_height = DEFAULT_HEIGHT;
	_angle = 0;
	_color = DEFAULT_COLOR;
	qt++;
	_id = qt;
	_currentImage = 0;
}

Widget::Widget(int startX, int startY, int width, int height, float angle) {
	_startX = startX;
	_startY = startY;
	_width = width;
	_height = height;
	_angle = angle;
	_color = DEFAULT_COLOR;
	qt++;
	_id = qt;
	_currentImage = 0;
	nowDrawing = false;
	pointsDrawn.reserve(32000);
	colorsDrawn.reserve(32000);
	pointPressures.reserve(32000);
	imagePaths.reserve(75);
}


Widget::~Widget(){
}

int Widget::getQt() {
	return qt;
}

int Widget::getId() const
{
	return _id;
}

ofPoint Widget::getStartPoint() const {
	return ofPoint(_startX, _startY);
}

ofPoint Widget::getDim() const {
	return ofPoint(_width, _height);
}

float Widget::getAngle() const {
	return _angle;
}

void Widget::setup(int startX, int startY, int width, int height, float angle) {
	_startX = startX;
	_startY = startY;
	_width = width;
	_height = height;
	_angle = angle;
}

void Widget::scroll(int x, int y)
{
	_startX += x;
	_startY += y;
}

void Widget::moveTo(int x, int y)
{
	_startX = x;
	_startY = y;
}

void Widget::relatResize(int x, int y)
{
	if (!_notResizable) {
		_width += x;
		_height += y;
	}
}

void Widget::resize(int x, int y)
{
	_width = x;
	_height = y;
}

void Widget::relatRotate(float a)
{
	_angle += a;
}

void Widget::resetAngle() {
	_angle = 0;
}

void Widget::showBar()
{
	_showBar = true;
	moveSlider();
}

void Widget::hideBar()
{
	_showBar = false;
}

void Widget::drawBar()
{
	if (_showBar && imagePaths.size() > 1) {
		ofPushStyle();
		ofSetLineWidth(3);
		ofSetColor(ofColor::blue);
		ofDrawLine(_startX, _startY - 10, _startX + 200, _startY - 10);
		ofSetLineWidth(1);
		ofNoFill();
		ofSetColor(ofColor::black);
		ofDrawCircle(_startX + 200 * _sliderPos, _startY - 10, 6);
		ofFill();
		ofSetColor(ofColor::red);
		ofDrawCircle(_startX + 200 * _sliderPos, _startY - 10, 6);
		ofPopStyle();
	}
}

void Widget::moveSlider()
{
	if (_currentImage == imagePaths.size()) _sliderPos = 1;
	else if (imagePaths.size() > 1) {
		float div = 1.0 / ((float)imagePaths.size() - 1);
		_sliderPos = (_currentImage-1) * div;
	}
}

void Widget::setSlider(float pos) {
	if (!ofInRange(pos, 0, 1)) return;
	_sliderPos = pos;
}

float Widget::getSlider() const
{
	return _sliderPos;
}

bool Widget::sliderOn() const
{
	return _showBar;
}

void Widget::update() {

}

void Widget::draw() {
	if (_currentImage) {
		ofPushMatrix();
		ofTranslate(_startX, _startY);
		ofRotate(_angle);
		if (image.getImageType() != OF_IMAGE_UNDEFINED)
		image.draw(0, 0, _width, _height);
		ofPushStyle();
		ofNoFill();
		ofSetColor(ofColor::black);
		ofDrawRectangle(0, 0, _width, _height);
		ofPopStyle();
		//ofImage image;
		//image.loadImage(imagePaths[_currentImage - 1]);		
		drawPenData();
		ofPopMatrix();
		drawBar();
	}
	else {
		ofPushMatrix();
		ofTranslate(_startX, _startY);
		ofRotate(_angle);
		ofPushStyle();
		//ofSetColor(ofColor::white);
		//ofDrawRectangle(-OFFSET, -OFFSET, _width + OFFSET * 2, OFFSET);
		//ofDrawRectangle(-OFFSET, 0, OFFSET, _height);
		//ofPushStyle();
		ofSetColor(_color);
		ofDrawRectangle(0, 0, _width, _height);
		ofNoFill();
		ofSetColor(ofColor::black);
		ofDrawRectangle(0, 0, _width, _height);
		//ofPopStyle();
		//ofDrawRectangle(_width, 0, OFFSET, _height);
		//ofDrawRectangle(-OFFSET, _height, _width + OFFSET * 2, OFFSET);
		//ofNoFill();
		//ofSetColor(ofColor::black);
		//ofDrawRectangle(-OFFSET, -OFFSET, _width + OFFSET * 2, _height + OFFSET * 2);
		ofPopStyle();
		drawPenData();
		ofPopMatrix();
	
	}
}

void Widget::drawPenData()
{
	ofPushStyle();
	for (int i = 0; i < pointsDrawn.size(); i++)
	{
		if (i < pointsDrawn.size() && ofInRange(pointsDrawn[i].x, 0, _width) && ofInRange(pointsDrawn[i].y, 0, _height)) {
			
			if (i < colorsDrawn.size())
				ofSetColor(colorsDrawn[i]);
			if (i < pointsDrawn.size() && i < pointPressures.size())
				ofDrawCircle(pointsDrawn[i], 5 * pointPressures[i]);
			
		}
	}
	ofPopStyle();
}

int Widget::getCurrentImage() const
{
	return _currentImage;
}

bool Widget::setCurrentImage(int index)
{
	if (index < 1 || index > imagePaths.size())
		return false;
	else {
	//ofImage image;
		if (imagePaths.size()==1)
	resize(600, 400);
	image.loadImage(imagePaths[index - 1]);
	//resize(image.getWidth(), image.getHeight());
	_currentImage = index;
	return true;
	}
}

bool Widget::pointInsideWidget(const ofPoint& point)
{
		std::vector<ofPoint> points;
		float angleInRad = ofDegToRad(_angle);
		float angleSin = sin(angleInRad);
		float angleCos = cos(angleInRad);
		points.push_back(ofPoint(_startX + _width * angleCos, _startY + _width * angleSin));
		points.push_back(ofPoint(_startX, _startY));
		points.push_back(ofPoint(_startX - _height * angleSin, _startY + _height * angleCos));
		points.push_back(ofPoint(points[2].x + _width * angleCos, points[2].y + _width * angleSin));
		return ofInsidePoly(point, points);
}

/*bool Widget::pointInsideWidget(const ofPoint& p1, const ofPoint& p2) {
	if (!pointInsideWidget(p1)) return false;
	if (!pointInsideWidget(p2)) return false;
	return true;
}*/

void Widget::repaint(ofColor color)
{
	_color = color;
}


