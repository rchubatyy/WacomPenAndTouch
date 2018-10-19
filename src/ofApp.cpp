#include "ofApp.h"
ofEvent<vector<WacomMTFinger>> touch;
bool ofApp::canDraw = true;
bool ofApp::canStraightOrCurve = false;
bool ofApp::rulerDraw = false;
ofPoint ofApp::straightOrCurve(0, 0);
vector<Widget> ofApp::widgets(1);
bool ofApp::secondColorPaletteDraw = false;
ofPoint ofApp::secondColorPalettePoint(0,0);
ofColor ofApp::currentColor(ofColor::black);
ofColor ofApp::colorToBeSet(ofColor::black);
ofPoint ofApp::touchedPts[10] = { ofPoint() ,ofPoint() ,ofPoint() ,ofPoint() ,ofPoint() ,ofPoint() ,ofPoint() ,ofPoint() ,ofPoint() ,ofPoint()};
ofPoint ofApp::rulerPt1(0, 0);
ofPoint ofApp::rulerPt2(0, 0);
vector<ofPoint> ofApp::pointsDrawn(1);
vector<ofColor> ofApp::colorsDrawn(1);
vector<float> ofApp::pointPressures(1);
int ofApp::canSwitchImage = 0;
int ofApp::canBringFront = 0;
int ofApp::widgetForSwitchingImage = 0;
bool ofApp::paletteShown = false;
bool ofApp::paletteShown2 = false;
ofPoint ofApp::palettePoint(0, 0);
ofColor ofApp::recentColors[18] =
{ ofColor::red,ofColor::orange,ofColor::yellow,ofColor::green,ofColor::deepSkyBlue,ofColor::blue,ofColor::darkViolet ,ofColor::white,ofColor::black,
  ofColor::white,ofColor::white,ofColor::white,ofColor::white,ofColor::white,ofColor::white,ofColor::white,ofColor::white,ofColor::white };
int ofApp::sliderMoveWidget = 0;
float ofApp::sliderPos = 0;
float lineAngle;
int drL;
int ofApp::w = 0;
ofFile ofApp::log;
ofBuffer ofApp::data;
//PenThread penThread;

//--------------------------------------------------------------
void ofApp::setup() {
#ifndef _DEBUG
	ofHideCursor();
#else
	ofSetWindowPosition(0, 0);
#endif
	log.open("timestamps.log", ofFile::Append, false);
	data = ofBufferFromFile(log.getAbsolutePath(), false);
	ofBufferToFile(log.getAbsolutePath(), data, false);
	initBBTabletAndMultiTouch();
	closeIcon.loadImage("icons/close.jpg");
	widgetIcon.loadImage("icons/widget.jpg");
	rulerIcon.loadImage("icons/ruler.jpg");
	colorIcon.loadImage("icons/colors.jpg");
	cameraIcon.loadImage("icons/camera.jpg");
	fingerDrawIcon.loadImage("icons/fingerdraw.jpg");
	deleteIcon.loadImage("icons/delete.jpg");
	ruler.loadImage("ruler.jpg");
	currentTool = widgetTool;
	widgets.clear();
	pointsDrawn.clear();
	colorsDrawn.clear();
	pointPressures.clear();
	widgets.reserve(100);
	pointsDrawn.reserve(32000);
	colorsDrawn.reserve(32000);
	pointPressures.reserve(32000);
	canDraw = true;
	rulerLock = false;
	initColors();
	currentColor = ofColor::black;
	colorToBeSet = ofColor::black;
	ofAddListener(touch, this, &ofApp::onTouched);
	ofAddListener(click1, this, &ofApp::onClick1);
	ofAddListener(click2, this, &ofApp::onClick2);
}

//--------------------------------------------------------------
void ofApp::update(){
	if (canAddWidget) {
		for (int i = 0; i < newWidgetPos.size(); i++)
			if(widgets.size()<100)
			widgets.push_back(Widget(newWidgetPos[i].x-100, newWidgetPos[i].y-100, 200, 200, 0));
		newWidgetPos.clear();
		canAddWidget = false;
	}
	if (canDelete)
		deleteWidget();
	if (canBringFront)
		bringFront(canBringFront);
	if (canAddImage)
		addImage(widgetForAddingImage);

	if (canSwitchImage)
		switchImage(widgetForSwitchingImage, canSwitchImage);
	if (sliderMoveWidget)
		moveSlider(sliderPos, sliderMoveWidget);
	if (penThread.pressedButtons == 2 || penThread.pressedButtons == 3) {
		if (!bt1pressed)
		ofNotifyEvent(click1, penThread.penPos);
	}
	else
		bt1pressed = false;
	if (penThread.pressedButtons == 4 || penThread.pressedButtons == 5) {
		if (!bt2pressed) {
			bt2pressed = true;
			ofNotifyEvent(click2, penThread.penPos);
		}
	}
	else
		bt2pressed = false;
	drL = pointsDrawn.size();
	for (int i = 0; i < widgets.size(); i++)
		drL += widgets[i].pointsDrawn.size();
}

//--------------------------------------------------------------
void ofApp::draw() {
	/*ofPushStyle();
	ofSetColor(ofColor::orange);
	ofDrawRectangle(42, 519, 40, 40);
	ofSetColor(ofColor::black);
	ofDrawRectangle(480, 495, 40, 40);
	ofDrawRectangle(801, 384, 40, 40);
	ofPopStyle();*/
	drawAllPenData();
	for (int i = 0; i < widgets.size(); i++)
		widgets[i].draw();
	drawRuler();
	drawStraightOrCurve();
	drawSecondPalette();
	drawColorPalette();
	drawToolbar();
	//drawPenCursor();
}

//--------------------------------------------------------------
void ofApp::exit() {
	data.clear();
	log.close();
	stopBBTabletAndMultiTouch();
	WacomMTQuit();
	ofRemoveListener(touch, this, &ofApp::onTouched);
	ofRemoveListener(click1, this, &ofApp::onClick1);
	ofRemoveListener(click2, this, &ofApp::onClick2);
}

//--------------------------------------------------------------
void ofApp::initColors() {
	recentColors[0] = ofColor::red;
	recentColors[1] = ofColor::orange;
	recentColors[2] = ofColor::yellow;
	recentColors[3] = ofColor::green;
	recentColors[4] = ofColor::deepSkyBlue;
	recentColors[5] = ofColor::blue;
	recentColors[6] = ofColor::darkViolet;
	recentColors[7] = ofColor::white;
	recentColors[8] = ofColor::black;
	static bool init = true;
	if (init)
		for (int i = 9; i < 18; i++)
			recentColors[i] = ofColor::white;
	init = false;
}

//--------------------------------------------------------------
void ofApp::initBBTabletAndMultiTouch() {
	WacomMTError res = WacomMTInitialize(WACOM_MULTI_TOUCH_API_VERSION);
	deviceDetect = WacomMTGetAttachedDeviceIDs(NULL, 0);
	if (res == WMTErrorSuccess){
		if (deviceDetect) {
			WacomMTRegisterAttachCallback(ofApp::AttachCallback, NULL);
			WacomMTRegisterDetachCallback(ofApp::DetachCallback, NULL);
		}
		else{
			ofSystemAlertDialog("Did not connect the Wacom tablet.");
			ofExit(0);
		}
	}
	else{
		ofSystemAlertDialog("Could not load the Wacom tablet driver.");
		ofExit(0);
	}
	penThread.start();
}

//--------------------------------------------------------------
void ofApp::stopBBTabletAndMultiTouch() {
	penThread.stop();
	UnregisterCallback(0);
}

//--------------------------------------------------------------
void ofApp::restartBBTabletAndMultiTouch() {
	tD.reinitTablets();
	RegisterCallback(0);
}

//--------------------------------------------------------------
/*void ofApp::getPenData() {
	pressure = g_callback.getPressure();
	penX = ofGetWidth() * g_callback.getX();
	penY = ofGetHeight() * (1 - g_callback.getY());
	penPos = ofPoint(penX, penY);
	penSide = g_callback.getType();
	pressedButtons = g_callback.getButtons();
	rotX = g_callback.getRotationX();
	rotY = g_callback.getRotationY();
	rotZ = g_callback.getRotationZ();
	rotAngle = g_callback.getAngle();
}*/

//--------------------------------------------------------------
void ofApp::AttachCallback(WacomMTCapability deviceInfo, void* userRef) {
	WacomMTError res = WacomMTRegisterFingerReadCallback(0, NULL, WMTProcessingModeNone, FingerCallback, NULL);
	//penThread.start();
}

//--------------------------------------------------------------

void ofApp::DetachCallback(int deviceID, void * userRef)
{
	WacomMTError res = WacomMTUnRegisterFingerReadCallback(0, NULL, WMTProcessingModeNone, NULL);
	//penThread.stop();
}

//--------------------------------------------------------------
WacomMTError ofApp::RegisterCallback(int deviceID) {
	return WacomMTRegisterFingerReadCallback(deviceID, NULL, WMTProcessingModeNone, FingerCallback, NULL);
}

//--------------------------------------------------------------
WacomMTError ofApp::UnregisterCallback(int deviceID) {
	return WacomMTUnRegisterFingerReadCallback(deviceID, NULL, WMTProcessingModeNone, NULL);
}

//--------------------------------------------------------------
int ofApp::FingerCallback(WacomMTFingerCollection* fingerData, void* userRef) {
	int totalFingers = fingerData->FingerCount;
	WacomMTFinger * fingerArray = fingerData->Fingers;
	vector<WacomMTFinger> fingerVector;
	fingerVector.reserve(10);
	for (int i = 0; i < totalFingers; i++)
		for (int j = 0; j < 10; j++){
			if (j >= totalFingers)
				continue;
			if (fingerArray[j].FingerID == i + 1) {
				fingerVector.push_back(fingerArray[j]);
				break;
			}
		}
	if(!fingerVector.empty())
	ofNotifyEvent(touch, fingerVector);
	return 0;
}

void ofApp::drawStraightOrCurve()
{
	if (canStraightOrCurve) {
		ofPushStyle();
		ofSetColor(currentColor);
		ofDrawCircle(straightOrCurve, 20);
		if (penThread.canStraightOrCurve) {
			ofSetLineWidth(5 * penThread.pressure);
			ofDrawArrow(straightOrCurve, penThread.straightOrCurve, 10 * penThread.pressure);
			float angle = getPointsAngle(straightOrCurve, penThread.straightOrCurve);
			ofPoint midPoint = ofPoint((penThread.straightOrCurve.x + straightOrCurve.x) / 2, (penThread.straightOrCurve.y + straightOrCurve.y) / 2);
			float half = ofDist(straightOrCurve.x, straightOrCurve.y, midPoint.x, midPoint.y);
			ofPoint endPoint1 = ofPoint(midPoint.x + half * penThread.rotAngle * cos(ofDegToRad(angle + 90)),
				midPoint.y + half * penThread.rotAngle * sin(ofDegToRad(angle + 90)));
			ofPoint endPoint2 = ofPoint(midPoint.x - half * penThread.rotAngle * cos(ofDegToRad(angle + 90)),
				midPoint.y - half * penThread.rotAngle * sin(ofDegToRad(angle + 90)));
			
				if (ofInRange(ofAngleDifferenceDegrees(angle, penThread.getPenOrientation()), 0, 180))
					ofDrawArrow(midPoint, endPoint1, 10 * penThread.pressure);
				else
					ofDrawArrow(midPoint, endPoint2, 10 * penThread.pressure);
		}
			ofPopStyle();
	}
}

void ofApp::drawColorPalette()
{
	ofPushStyle();
	if (paletteShown || paletteShown2) {
		if (palettePoint.x <= ofGetWidth() / 2) {
			ofNoFill();
			ofSetColor(ofColor::black);
			ofDrawRectangle(palettePoint.x - 20, palettePoint.y - 30, 470, 280);
			ofFill();
			ofSetColor(ofColor::whiteSmoke);
			ofDrawRectangle(palettePoint.x - 20, palettePoint.y - 30, 470, 280);
			for (int i = 0; i < 9; i++) {
				ofNoFill();
				ofSetColor(ofColor::black);
				ofDrawRectangle(palettePoint.x + 40 + 40 * i, palettePoint.y + 30, 30, 50);
				ofFill();
				ofSetColor(recentColors[i]);
				ofDrawRectangle(palettePoint.x + 40 + 40 * i, palettePoint.y + 30, 30, 50);
			}
			for (int i = 9; i < 18; i++) {
				ofNoFill();
				ofSetColor(ofColor::black);
				ofDrawRectangle(palettePoint.x + 40 + 40 * (i-9), palettePoint.y + 130, 30, 50);
				ofFill();
				ofSetColor(recentColors[i]);
				ofDrawRectangle(palettePoint.x + 40 + 40 * (i-9), palettePoint.y + 130, 30, 50);
			}
			ofSetColor(ofColor::orange);
			ofDrawTriangle(palettePoint.x + 395, palettePoint.y + 90, palettePoint.x + 420, palettePoint.y + 40, palettePoint.x + 445, palettePoint.y + 90);
			ofDrawTriangle(palettePoint.x + 395, palettePoint.y + 120, palettePoint.x + 420, palettePoint.y + 170, palettePoint.x + 445, palettePoint.y + 120);

		}
		else {
			ofNoFill();
			ofSetColor(ofColor::black);
			ofDrawRectangle(palettePoint.x - 450, palettePoint.y - 30, 470, 280);
			ofFill();
			ofSetColor(ofColor::whiteSmoke);
			ofDrawRectangle(palettePoint.x - 450, palettePoint.y - 30, 470, 280);
			for (int i = 0; i < 9; i++) {
				ofNoFill();
				ofSetColor(ofColor::black);
				ofDrawRectangle(palettePoint.x - 390 + 40 * i, palettePoint.y + 30, 30, 50);
				ofFill();
				ofSetColor(recentColors[i]);
				ofDrawRectangle(palettePoint.x - 390 + 40 * i, palettePoint.y + 30, 30, 50);
			}
			for (int i = 9; i < 18; i++) {
				ofNoFill();
				ofSetColor(ofColor::black);
				ofDrawRectangle(palettePoint.x - 390 + 40 * (i - 9), palettePoint.y + 130, 30, 50);
				ofFill();
				ofSetColor(recentColors[i]);
				ofDrawRectangle(palettePoint.x - 390 + 40 * (i - 9), palettePoint.y + 130, 30, 50);
			}
			ofSetColor(ofColor::orange);
			ofDrawTriangle(palettePoint.x - 445, palettePoint.y + 90, palettePoint.x - 420, palettePoint.y + 40, palettePoint.x - 395, palettePoint.y + 90);
			ofDrawTriangle(palettePoint.x - 445, palettePoint.y + 120, palettePoint.x - 420, palettePoint.y + 170, palettePoint.x - 395, palettePoint.y + 120);
		}
	}
	ofPopStyle();
}

bool ofApp::onPalette(ofPoint point)
{
	if (!paletteShown) return false;
	else {
		if (palettePoint.x <= ofGetWidth() / 2) {
			if (ofInRange(point.x, palettePoint.x - 20, palettePoint.x + 450)
				&& ofInRange(point.y, palettePoint.y - 30, palettePoint.y + 250))
			{
				return true;
			}
		}
		else {
			if (ofInRange(point.x, palettePoint.x - 450, palettePoint.x + 20)
				&& ofInRange(point.y, palettePoint.y - 30, palettePoint.y + 250))
			{
				return true;
			}
		}

	}
	return false;
}

void ofApp::deleteWidget()
{
	widgets.erase(widgets.end()-1);
	canDelete = false;
}

void ofApp::switchImage(int w, int x)
{
	widgets[w].setCurrentImage(x);
	canSwitchImage = 0;
}

void ofApp::addImage(int w)
{
	if (!w || w > widgets.size()) {
		canAddImage = false;
		return;
	}
	if (widgets[w - 1].imagePaths.size()<75)
	widgets[w - 1].imagePaths.push_back("photos/"
		+ ofToString(widgets[w - 1].imagePaths.size() + 1) + ".jpg");
	widgets[w - 1].setCurrentImage(widgets[w - 1].imagePaths.size());
	widgets[w - 1].hideBar();
	canBringFront = w;
	canAddImage = false;
}

void ofApp::bringFront(int w)
{
		for (int i = canBringFront - 1; i < widgets.size() - 1; i++)
			swap(widgets.at(i), widgets.at(i + 1));
		canBringFront = 0;
}

void ofApp::moveSlider(float pos, int w)
{
	if (!ofInRange(pos, 0, 1)) return;
	widgets[w-1].setSlider(pos);
	if (ofInRange(pos,0.99, 1)) 
		widgets[w-1].setCurrentImage(widgets[w-1].imagePaths.size());
	else if (widgets[w-1].imagePaths.size() > 1) {
		int size = widgets[w - 1].imagePaths.size() - 1;
		float div = 1.0 / ((float)widgets[w - 1].imagePaths.size() - 1);
		widgets[w-1].setCurrentImage(pos / div + 1);
	}
	
	sliderMoveWidget = 0;
}

void ofApp::drawRuler() {
	if (rulerDraw) {
		ofPushMatrix();
		ofTranslate(rulerPt1.x, rulerPt1.y);
		ofRotate(getPointsAngle(rulerPt1, rulerPt2));
		ruler.draw(-RULER_WIDTH/2, -RULER_WIDTH/2, RULER_LENGTH, RULER_WIDTH);
		ofPopMatrix();
	}
}

void ofApp::drawSecondPalette()
{
	if (secondColorPaletteDraw) {
		ofPushStyle();
		ofSetColor(ofColor::olive);
		ofDrawRectangle(secondColorPalettePoint, 100, 150);
		ofSetColor(colorToBeSet);
		ofDrawCircle(secondColorPalettePoint + ofPoint(50,100), 48);
		ofSetColor(currentColor);
		ofDrawCircle(secondColorPalettePoint + ofPoint(75, 10), 10);
		ofPopStyle();
	}
}

void ofApp::drawToolbar() {

	ofPushStyle();
	ofSetColor(ofColor::olive);
	ofDrawRectangle(ofGetWidth() - 100, 0, 100, ofGetHeight());
	ofPopStyle();

	closeIcon.draw(ofGetWidth() - 100, 0, 100, 100);

	ofPushStyle();
	if (currentTool == widgetTool)
		ofSetColor(ofColor::lightGreen);
	widgetIcon.draw(ofGetWidth() - 100, 100, 100, 100);
	ofPopStyle();

	ofPushStyle();
	if (currentTool == rulerTool)
		ofSetColor(ofColor::lightGreen);
	if (currentTool == rulerToolv2)
		ofSetColor(ofColor::lightPink);
	rulerIcon.draw(ofGetWidth() - 100, 200, 100, 100);
	ofPopStyle();

	ofPushStyle();
	if (currentTool == colorTool)
		ofSetColor(ofColor::lightGreen);
	if (currentTool == colorToolv2)
		ofSetColor(ofColor::lightPink);
	colorIcon.draw(ofGetWidth() - 100, 300, 100, 100);
	ofPopStyle();

	ofPushStyle();
	if (currentTool == photoTool)
		ofSetColor(ofColor::lightGreen);
	cameraIcon.draw(ofGetWidth() - 100, 400, 100, 100);
	ofPopStyle();

	ofPushStyle();
	if (currentTool == fingerDrawTouchTool)
		ofSetColor(ofColor::lightGreen);
	fingerDrawIcon.draw(ofGetWidth() - 100, 500, 100, 100);
	ofPopStyle();

	//palette.draw(ofGetWidth() - 100, ofGetHeight() - 250, 100, 150);
	
		ofPushStyle();
		ofSetColor(colorToBeSet);
		ofDrawCircle(ofGetWidth() - 50, ofGetHeight() - 150, 50);
		ofSetColor(currentColor);
		ofDrawCircle(ofGetWidth() - 20, ofGetHeight() - 220, 10);
		if (!canDraw) {
			if (currentColor.r < 100 && currentColor.g < 100 && currentColor.b < 100)
				ofSetColor(ofColor::white);
			else
			ofSetColor(ofColor::black);
			ofDrawBitmapString("Tap to \n enable \n drawing.", ofGetWidth() - 80, ofGetHeight() - 160);
		}
		ofPopStyle();


	deleteIcon.draw(ofGetWidth() - 100, ofGetHeight() - 100, 100, 100);
}

void ofApp::drawAllPenData()
{
	ofPushStyle();
	for (int i = 0; i < pointsDrawn.size(); i++)
	{		
		if (i<colorsDrawn.size())
		ofSetColor(colorsDrawn[i]);
		ofSetCircleResolution(8);
		if (i<pointsDrawn.size()&& i<pointPressures.size())
		ofDrawCircle(pointsDrawn[i], 5 * pointPressures[i]);		
	}
	ofPopStyle();
}

void ofApp::drawPenCursor()
{
	if (deviceDetect)
		if (tD.getNextEvent(evt))
		{
			ofPushStyle();
			ofNoFill();
			ofSetColor(ofColor::crimson);
			ofDrawCircle(penThread.penPos, 2);
			ofPopStyle();
		}
		else {
			penThread.oldAngle = 0;
			penThread.oldOrientation = 0;
			penThread.tiltCount = 0;
		}
}

bool ofApp::pointOnRuler(ofPoint point)
{
	float angle = ofDegToRad(getPointsAngle(rulerPt1, rulerPt2));
	float startX = rulerPt1.x + (RULER_WIDTH / 2 * sin(angle) - RULER_WIDTH / 2 * cos(angle));
	float startY = rulerPt1.y - (RULER_WIDTH / 2 * sin(angle) + RULER_WIDTH / 2 * cos(angle));
	std::vector<ofPoint> points;
	points.push_back(ofPoint(startX + RULER_LENGTH * cos(angle), startY + RULER_LENGTH * sin(angle)));
	points.push_back(ofPoint(startX, startY));
	points.push_back(ofPoint(startX - RULER_WIDTH * sin(angle), startY + RULER_WIDTH * cos(angle)));
	points.push_back(ofPoint(points[2].x + RULER_LENGTH * cos(angle), points[2].y + RULER_LENGTH * sin(angle)));
	return ofInsidePoly(point, points);
}

bool ofApp::pointOnRulerCmSide(ofPoint point)
{
	float angle = ofDegToRad(getPointsAngle(rulerPt1, rulerPt2));
	float startX = rulerPt1.x + (RULER_WIDTH / 2 * sin(angle) - RULER_WIDTH / 2 * cos(angle));
	float startY = rulerPt1.y - (RULER_WIDTH / 2 * sin(angle) + RULER_WIDTH / 2 * cos(angle));
	std::vector<ofPoint> points;
	points.push_back(ofPoint(startX + RULER_LENGTH * cos(angle), startY + RULER_LENGTH * sin(angle)));
	points.push_back(ofPoint(startX, startY));
	points.push_back(ofPoint(startX - RULER_WIDTH/2 * sin(angle), startY + RULER_WIDTH/2 * cos(angle)));
	points.push_back(ofPoint(points[2].x + RULER_LENGTH * cos(angle), points[2].y + RULER_LENGTH * sin(angle)));
	return ofInsidePoly(point, points);
}

bool ofApp::pointOnRulerInchSide(ofPoint point)
{
	float angle = ofDegToRad(getPointsAngle(rulerPt1, rulerPt2));
	float startX = rulerPt1.x - RULER_WIDTH / 2 * cos(angle);
	float startY = rulerPt1.y - RULER_WIDTH / 2 * sin(angle);
	std::vector<ofPoint> points;
	points.push_back(ofPoint(startX + RULER_LENGTH * cos(angle), startY + RULER_LENGTH * sin(angle)));
	points.push_back(ofPoint(startX, startY));
	points.push_back(ofPoint(startX - RULER_WIDTH / 2 * sin(angle), startY + RULER_WIDTH / 2 * cos(angle)));
	points.push_back(ofPoint(points[2].x + RULER_LENGTH * cos(angle), points[2].y + RULER_LENGTH * sin(angle)));
	return ofInsidePoly(point, points);
}

ofPoint ofApp::perfectPointForCm(ofPoint point)
{
	ofPoint pt = point;
	float angle = ofDegToRad(getPointsAngle(rulerPt1, rulerPt2));
	while (pointOnRulerCmSide(pt)) {
		pt.x += sin(angle);
		pt.y -= cos(angle);
	}
	return pt;

}

ofPoint ofApp::perfectPointForInch(ofPoint point)
{
	ofPoint pt = point;
	float angle = ofDegToRad(getPointsAngle(rulerPt1, rulerPt2));
	while (pointOnRulerInchSide(pt)) {
		pt.x -= sin(angle);
		pt.y += cos(angle);
	}
	return pt;
}

//--------------------------------------------------------------
float ofApp::getPointsAngle(ofPoint a, ofPoint b) {
	if (b.x > a.x)
	{
		if (b.y > a.y)
			return ofRadToDeg(acos(abs(b.x - a.x) / ofDist(a.x, a.y, b.x, b.y)));
		else
			return -ofRadToDeg(acos(abs(b.x - a.x) / ofDist(a.x, a.y, b.x, b.y)));
	}
	else {
		if (b.y > a.y)
			return 90 + ofRadToDeg(asin(abs(b.x - a.x) / ofDist(a.x, a.y, b.x, b.y)));
		else
			return 180 + ofRadToDeg(acos(abs(b.x - a.x) / ofDist(a.x, a.y, b.x, b.y)));
	}
}

int ofApp::getHueFromTilt(float a, float b)
{
	float hue;
	if (b >= 0)
		hue = ofRadToDeg(acos(a));
	else
		hue = 180+ofRadToDeg(acos(-a));
	hue /= 360;
	hue *= 255;
	return (int) hue;
}

//--------------------------------------------------------------

int ofApp::getWidgetIdByPos(ofPoint point)
{
	for (int i = widgets.size(); i > 0; i--)
		if (!widgets.empty() && widgets[i - 1].pointInsideWidget(point))
			return i;
	return 0;
}

void ofApp::onTouched(vector<WacomMTFinger> & fingers)
{
	WacomMTFinger& finger = fingers[0];
	int numFingers = fingers.size();
	ofPoint p1, p2;
	for (int i=0; i<numFingers; i++)
		touchedWidget[i] = getWidgetIdByPos(ofPoint(fingers[i].X, fingers[i].Y));
	tool oldTool = currentTool;
	if (finger.X > ofGetWidth() - 100) {
		paletteShown = false;
		if (finger.Confidence && finger.TouchState == WMTFingerStateUp)
			if (finger.Y <= 100)
				ofExit();
			else if (ofInRange(finger.Y, 101, 200)) {
				currentTool = widgetTool;
				secondColorPaletteDraw = false;
				canDraw = true;
			}
			else if (ofInRange(finger.Y, 201, 300)) {
				if (currentTool == rulerTool)
					currentTool = rulerToolv2;
				else
				currentTool = rulerTool;
				secondColorPaletteDraw = false;
				canDraw = true;
			}
			else if (ofInRange(finger.Y, 301, 400)) {
				if (currentTool == colorTool) 
					currentTool = colorToolv2;
				else {
					currentTool = colorTool;
					secondColorPaletteDraw = false;
					canDraw = true;
				}
			}
			else if (ofInRange(finger.Y, 401, 500))
				currentTool = photoTool;
			else if (ofInRange(finger.Y, 501, 600)) {
				currentTool = fingerDrawTouchTool;
				secondColorPaletteDraw = false;
				canDraw = true;
			}
			else if (ofInRange(finger.Y, ofGetHeight() - 200, ofGetHeight() - 101))
				canDraw = !canDraw;
			else if (ofInRange(finger.Y, ofGetHeight() - 100, ofGetHeight())) {
				if (touchedWidget[0]) {
					//widgets.erase(widgets.begin()+ touchedWidget[0]-1);
					//widgets.shrink_to_fit();
					canDelete = true;
				}
				else {
					widgets.clear();
					pointsDrawn.clear();
					colorsDrawn.clear();
					pointPressures.clear();
					//widgets.shrink_to_fit();
					//pointsDrawn.shrink_to_fit();
					//colorsDrawn.shrink_to_fit();
					//pointPressures.shrink_to_fit();
					penThread.fails = 0;
				}
			}
				
	}
	else {
		switch (currentTool) {
		case widgetTool:
			for (int i = 0; i<numFingers; i++){
			if (!touchedWidget[i])
			{
				if (fingers[i].TouchState == WMTFingerStateDown && fingers[i].Confidence) {
					newWidgetPos.push_back(ofPoint(fingers[i].X, fingers[i].Y));
					canAddWidget = true;
					touchedPts[i] = ofPoint(fingers[i].X, fingers[i].Y);					
				}
			}
			else {
				if (fingers[i].TouchState == WMTFingerStateDown) {
					touchedPts[i] = ofPoint(fingers[i].X, fingers[i].Y);
					canBringFront = touchedWidget[i];
					float tmpAngle = ofAngleDifferenceDegrees(getPointsAngle(penThread.penPos, touchedPts[i]), getPointsAngle(penThread.penPos, ofPoint(fingers[i].X, fingers[i].Y)));
					//ofPoint p1, p2;
					float angleForWidget = ofAngleDifferenceDegrees(widgets[touchedWidget[i] - 1].getAngle(),
						getPointsAngle(widgets[touchedWidget[i] - 1].getStartPoint(), ofPoint(fingers[i].X, fingers[i].Y)));
					float distFromStart = ofDist(widgets[touchedWidget[i] - 1].getStartPoint().x, widgets[touchedWidget[i] - 1].getStartPoint().y,
						fingers[i].X, fingers[i].Y);
					wp1[i].x = distFromStart * cos(ofDegToRad(angleForWidget));
					wp1[i].y = distFromStart * sin(ofDegToRad(angleForWidget));
					if (widgets[touchedWidget[i] - 1].getCurrentImage() && !widgets[touchedWidget[i] - 1].getAngle() && !canDraw)
					{
						widgets[touchedWidget[i] - 1].showBar(); //not sure about it... If it does not work, I will need to add some code in main GL thread...
					}
						// showBar[i] = touchedWidget[i];
				}
				if (fingers[i].TouchState == WMTFingerStateHold) {
					ofPoint tmp = ofPoint(fingers[i].X, fingers[i].Y) - touchedPts[i];
					bool twoTouches = false;
					int k; //second touched finger
					for (int j = 0; j < numFingers; j++) {
						if (j == i) continue;
						if (getWidgetIdByPos(ofPoint(fingers[j].X, fingers[j].Y)) == touchedWidget[i]) {
							twoTouches = true;
							k = j;
							break;
						}
					}
					float tmpAngle = ofAngleDifferenceDegrees(getPointsAngle(penThread.penPos, touchedPts[i]), getPointsAngle(penThread.penPos, ofPoint(fingers[i].X, fingers[i].Y)));
					//ofPoint p1, p2;
					float angleForWidget = ofAngleDifferenceDegrees(widgets[touchedWidget[i] - 1].getAngle(),
						getPointsAngle(widgets[touchedWidget[i] - 1].getStartPoint(), ofPoint(fingers[i].X, fingers[i].Y)));
					float distFromStart = ofDist(widgets[touchedWidget[i] - 1].getStartPoint().x, widgets[touchedWidget[i] - 1].getStartPoint().y,
					fingers[i].X, fingers[i].Y);
					p1.x = distFromStart * cos(ofDegToRad(angleForWidget));
					p1.y = distFromStart * sin(ofDegToRad(angleForWidget));
					if (penThread.pressedButtons == 4 || penThread.pressedButtons == 5)
						ofNotifyEvent(click2, penThread.penPos);
					if (twoTouches){
						widgets[touchedWidget[i] - 1].resetAngle();
						while (widgets[touchedWidget[i] - 1].getStartPoint().y > (ofGetHeight() - 100))
							widgets[touchedWidget[i] - 1].scroll(0, -1);
						while (widgets[touchedWidget[i] - 1].getStartPoint().x > (ofGetWidth() - 200))
							widgets[touchedWidget[i] - 1].scroll(-1, 0);
						angleForWidget = ofAngleDifferenceDegrees(widgets[touchedWidget[i] - 1].getAngle(),
							getPointsAngle(widgets[touchedWidget[i] - 1].getStartPoint(), ofPoint(fingers[k].X, fingers[k].Y)));
						distFromStart = ofDist(widgets[touchedWidget[i] - 1].getStartPoint().x, widgets[touchedWidget[i] - 1].getStartPoint().y,
							fingers[k].X, fingers[k].Y);
						p2.x = distFromStart * cos(ofDegToRad(angleForWidget));
						p2.y = distFromStart * sin(ofDegToRad(angleForWidget));
						ofPoint tmp2 = p1 - wp1[i];
						if (p1.x > p2.x && p1.y > p2.y) {
							widgets[touchedWidget[i] - 1].relatResize(tmp.x, tmp.y);
						}
						else if (p1.x < p2.x && p1.y > p2.y) {
							widgets[touchedWidget[i] - 1].relatResize(-tmp.x, tmp.y);
							widgets[touchedWidget[i] - 1].scroll(tmp.x, 0);
						}
						else if (p1.x < p2.x && p1.y < p2.y) {
							widgets[touchedWidget[i] - 1].relatResize(-tmp.x, -tmp.y);
							widgets[touchedWidget[i] - 1].scroll(tmp.x, tmp.y);
						}
						else if (p1.x > p2.x && p1.y < p2.y) {
							widgets[touchedWidget[i] - 1].relatResize(tmp.x, -tmp.y);
							widgets[touchedWidget[i] - 1].scroll(0, tmp.y);
						}
					}
					else if (penThread.pressure && !canDraw && widgets[touchedWidget[i] - 1].pointInsideWidget(penThread.penPos) && !penThread.picked && !penThread.adjusting) {
						widgets[touchedWidget[i] - 1].resetAngle();
						while (widgets[touchedWidget[i] - 1].getStartPoint().y > (ofGetHeight() - 100))
							widgets[touchedWidget[i] - 1].scroll(0, -1);
						while (widgets[touchedWidget[i] - 1].getStartPoint().x > (ofGetWidth() - 200))
							widgets[touchedWidget[i] - 1].scroll(-1, 0);
						angleForWidget = ofAngleDifferenceDegrees(widgets[touchedWidget[i] - 1].getAngle(),
							getPointsAngle(widgets[touchedWidget[i] - 1].getStartPoint(), penThread.penPos));
						distFromStart = ofDist(widgets[touchedWidget[i] - 1].getStartPoint().x, widgets[touchedWidget[i] - 1].getStartPoint().y,
							penThread.penX, penThread.penY);
						p2.x = distFromStart * cos(ofDegToRad(angleForWidget));
						p2.y = distFromStart * sin(ofDegToRad(angleForWidget));
						ofPoint tmp2 = p1 - wp1[i];
						if (p1.x > p2.x && p1.y > p2.y) {
							widgets[touchedWidget[i] - 1].relatResize(tmp.x, tmp.y);}
						else if (p1.x < p2.x && p1.y > p2.y) {
							widgets[touchedWidget[i] - 1].relatResize(-tmp.x, tmp.y);
							widgets[touchedWidget[i] - 1].scroll(tmp.x, 0);}
						else if (p1.x < p2.x && p1.y < p2.y) {
							widgets[touchedWidget[i] - 1].relatResize(-tmp.x, -tmp.y);
							widgets[touchedWidget[i] - 1].scroll(tmp.x, tmp.y);}
						else if (p1.x > p2.x && p1.y < p2.y) {
							widgets[touchedWidget[i] - 1].relatResize(tmp.x, -tmp.y);
							widgets[touchedWidget[i] - 1].scroll(0, tmp.y);}
						}
					else
						widgets[touchedWidget[i] - 1].scroll(tmp.x, tmp.y);
					touchedPts[i] = ofPoint(fingers[i].X, fingers[i].Y);
					tmpAngle = ofAngleDifferenceDegrees(getPointsAngle(penThread.penPos, touchedPts[i]), getPointsAngle(penThread.penPos, ofPoint(fingers[i].X, fingers[i].Y)));
					//ofPoint p1, p2;
					angleForWidget = ofAngleDifferenceDegrees(widgets[touchedWidget[i] - 1].getAngle(),
						getPointsAngle(widgets[touchedWidget[i] - 1].getStartPoint(), ofPoint(fingers[i].X, fingers[i].Y)));
					distFromStart = ofDist(widgets[touchedWidget[i] - 1].getStartPoint().x, widgets[touchedWidget[i] - 1].getStartPoint().y,
						fingers[i].X, fingers[i].Y);
					wp1[i].x = distFromStart * cos(ofDegToRad(angleForWidget));
					wp1[i].y = distFromStart * sin(ofDegToRad(angleForWidget));
				}
				if (fingers[i].TouchState == WMTFingerStateUp) {
					if (widgets[touchedWidget[i] - 1].getCurrentImage())
					widgets[touchedWidget[i] - 1].hideBar();
					touchedWidget[i] = 0;
				}
			}
			}
			break;
		case rulerTool:
			if (finger.TouchState == WMTFingerStateHold && numFingers > 1) {
				rulerPt1 = ofPoint(finger.X, finger.Y);
				rulerPt2 = ofPoint(fingers[1].X, fingers[1].Y);
				rulerDraw = true;
			}
			if (numFingers>1 && fingers[1].TouchState == WMTFingerStateDown){
					rulerPt1 = ofPoint(finger.X, finger.Y);
					rulerPt2 = ofPoint(fingers[1].X, fingers[1].Y);
				rulerDraw = true;
			}
			if (finger.TouchState == WMTFingerStateUp && finger.FingerID == 1 && !rulerLock)
				rulerDraw = false;
			/*if (numFingers > 2 && fingers[2].TouchState == WMTFingerStateDown
				&& pointOnRuler(ofPoint(fingers[2].X, fingers[2].Y)))
				rulerLock = !rulerLock;*/
			break;
		case colorTool:
			if (finger.TouchState == WMTFingerStateDown && !touchedWidget[0]) {
				secondColorPaletteDraw = !secondColorPaletteDraw;
				if (secondColorPaletteDraw) {
					secondColorPalettePoint = ofPoint(finger.X, finger.Y);
					canDraw = false;
					data.append("Open palette at the point: (" + ofToString(finger.X) + ", " + ofToString(finger.Y) + ") at\n");
					data.append(ofGetTimestampString("%H:%M:%S.%i.\n"));
					ofBufferToFile(log.getAbsolutePath(), data, false);
				}
				else canDraw = true;
				
			}
			if (penThread.pressedButtons == 4 || penThread.pressedButtons == 5)
				ofNotifyEvent(click2, penThread.penPos);
			break;
		case photoTool:
			if (touchedWidget[0] && finger.TouchState == WMTFingerStateUp && finger.Confidence) {
				canAddImage = true;
				widgetForAddingImage = touchedWidget[0];
			}
			break;
		case fingerDrawTouchTool:
			for (int i = 0; i < numFingers; i++) {
				if (!touchedWidget[i]) {
					for (int j = 0; j < pointsDrawn.size(); j++)
						//if (w[widgetId - 1].pointsDrawn[i] == penPosForWidget) {
						if (ofInRange(pointsDrawn[j].x, fingers[i].X - 5, fingers[i].X + 5)
							&& ofInRange(pointsDrawn[j].y, fingers[i].Y - 5, fingers[i].Y + 5)) {
							//pointsDrawn.erase(pointsDrawn.begin() + i);
							//colorsDrawn.erase(colorsDrawn.begin() + i);
							//pointPressures.erase(pointPressures.begin() + i);
							if (pointPressures[j] <= 1) {
								pointPressures[j] *= 4;
								break;
							}

						}
				}
				else {
					for (int j = 0; j < widgets[touchedWidget[i]-1].pointsDrawn.size(); j++){
						float angleForWidget = ofAngleDifferenceDegrees(widgets[touchedWidget[i] - 1].getAngle(),
							getPointsAngle(widgets[touchedWidget[i] - 1].getStartPoint(), ofPoint(fingers[i].X,fingers[i].Y)));
						float distFromStart = ofDist(widgets[touchedWidget[i] - 1].getStartPoint().x, widgets[touchedWidget[i] - 1].getStartPoint().y, fingers[i].X, fingers[i].Y);
						ofPoint fPosForWidget;
						fPosForWidget.x = distFromStart * cos(ofDegToRad(angleForWidget));
						fPosForWidget.y = distFromStart * sin(ofDegToRad(angleForWidget));
						//if (w[widgetId - 1].pointsDrawn[i] == penPosForWidget) {
						if (ofInRange(widgets[touchedWidget[i] - 1].pointsDrawn[j].x, fPosForWidget.x - 5, fPosForWidget.x + 5)
							&& ofInRange(widgets[touchedWidget[i] - 1].pointsDrawn[j].y, fPosForWidget.y - 5, fPosForWidget.y + 5)) {
							//pointsDrawn.erase(pointsDrawn.begin() + i);
							//colorsDrawn.erase(colorsDrawn.begin() + i);
							//pointPressures.erase(pointPressures.begin() + i);
							if (widgets[touchedWidget[i] - 1].pointPressures[j] <= 1) {
								widgets[touchedWidget[i] - 1].pointPressures[j] *= 4;
								break;
							}
						}
						}
				}

			}
			break;
		case rulerToolv2:
			if (finger.TouchState == WMTFingerStateDown) {
				canStraightOrCurve = true;
				straightOrCurve = ofPoint(finger.X, finger.Y);
				data.append("First line point chosen: (" + ofToString(straightOrCurve.x) + ", " + ofToString(straightOrCurve.y) + ") at\n");
				data.append(ofGetTimestampString("%H:%M:%S.%i.\n"));
				ofBufferToFile(log.getAbsolutePath(), data, false);
			}
			else if (finger.TouchState == WMTFingerStateHold)
				straightOrCurve = ofPoint(finger.X, finger.Y);
			else if (finger.TouchState == WMTFingerStateUp)
				canStraightOrCurve = false;
			break;
		case colorToolv2:
			if (!paletteShown2) {
				if (finger.TouchState == WMTFingerStateDown) {

					paletteShown = true;
					palettePoint = ofPoint(finger.X, finger.Y);
					canDraw = false;
					data.append("Open palette at the point: (" + ofToString(palettePoint.x) + ", " + ofToString(palettePoint.y) + ") at\n");
					data.append(ofGetTimestampString("%H:%M:%S.%i.\n"));
					ofBufferToFile(log.getAbsolutePath(), data, false);
				}
				else if (finger.TouchState == WMTFingerStateHold)
					palettePoint = ofPoint(finger.X, finger.Y);
				else if (finger.TouchState == WMTFingerStateUp) {
					paletteShown = false;
					canDraw = true;
				}
			}
			break;
		case vidTool:
			break;
		}
	}
	for (int i = 0; i<numFingers; i++)
		touchedWidget[i] = 0;
}

void ofApp::onClick1(ofPoint & pos)
{
	bt1pressed = true;
	if (secondColorPaletteDraw) {
		if (ofInRange(penThread.penX, secondColorPalettePoint.x, secondColorPalettePoint.x + 100)
			&& ofInRange(penThread.penY, secondColorPalettePoint.y+50, secondColorPalettePoint.y + 150)) {
			currentColor = colorToBeSet;
			recentColors[17] = recentColors[16];
			recentColors[16] = recentColors[15];
			recentColors[15] = recentColors[14];
			recentColors[14] = recentColors[13];
			recentColors[13] = recentColors[12];
			recentColors[12] = recentColors[11];
			recentColors[11] = recentColors[10];
			recentColors[10] = recentColors[9];
			recentColors[9] = currentColor;
			data.append("Selected a color (" + ofToString((int)currentColor.r) + ", " + ofToString((int)currentColor.g) + ", " + ofToString((int)currentColor.b) + ")\n");
			data.append("at the point: (" + ofToString(penThread.penX) + ", " + ofToString(penThread.penY) + ") at\n");
			data.append(ofGetTimestampString("%H:%M:%S.%i.\n"));
			ofBufferToFile(log.getAbsolutePath(), data, false);
		}
	}
	if (ofInRange(penThread.penX, ofGetWidth() - 100, ofGetWidth()) && ofInRange(penThread.penY, ofGetHeight() - 200, ofGetHeight() - 100)) {
		currentColor = colorToBeSet;
		recentColors[17] = recentColors[16];
		recentColors[16] = recentColors[15];
		recentColors[15] = recentColors[14];
		recentColors[14] = recentColors[13];
		recentColors[13] = recentColors[12];
		recentColors[12] = recentColors[11];
		recentColors[11] = recentColors[10];
		recentColors[10] = recentColors[9];
		recentColors[9] = currentColor;
		data.append("Selected a color (" + ofToString((int)currentColor.r) + ", " + ofToString((int)currentColor.g) + ", " + ofToString((int)currentColor.b) + ")\n");
		data.append("at the point: (" + ofToString(penThread.penX) + ", " + ofToString(penThread.penY) + ") at\n");
		data.append(ofGetTimestampString("%H:%M:%S.%i.\n"));
		ofBufferToFile(log.getAbsolutePath(), data, false);
	}
	if (canStraightOrCurve && penThread.canStraightOrCurve && pointsDrawn.size() < 32000) {
		ofPoint currentPt;
		float angle = getPointsAngle(straightOrCurve, penThread.straightOrCurve);
		lineAngle = angle;
		float width = penThread.pressure;
		currentPt = straightOrCurve;
		while (!ofInRange(currentPt.x, penThread.straightOrCurve.x - 1, penThread.straightOrCurve.x + 1)
			|| !ofInRange(currentPt.y, penThread.straightOrCurve.y - 1, penThread.straightOrCurve.y + 1)) {
			pointsDrawn.push_back(currentPt);
			colorsDrawn.push_back(currentColor);
			pointPressures.push_back(width);
			if (pointsDrawn.size() == 32000) break;
			currentPt.x += cos(ofDegToRad(angle));
			currentPt.y += sin(ofDegToRad(angle));
		}
		data.append("Traced line from (" + ofToString(straightOrCurve.x) + ", " + ofToString(straightOrCurve.y) + ")\n");
		data.append("to  (" + ofToString(penThread.straightOrCurve.x) + ", " + ofToString(penThread.straightOrCurve.y) + ")\n");
		data.append("with " + ofToString(penThread.pressure) + "% pressure at\n");
		data.append(ofGetTimestampString("%H:%M:%S.%i.\n"));
		ofBufferToFile(log.getAbsolutePath(), data, false);
	}
}

void ofApp::onClick2(ofPoint & pos)
{
	if (secondColorPaletteDraw) {
		if (ofInRange(penThread.penX, secondColorPalettePoint.x, secondColorPalettePoint.x + 100)
			&& ofInRange(penThread.penY, secondColorPalettePoint.y+50, secondColorPalettePoint.y + 150))
			for (int i = 0; i < 10; i++) {
				int w = touchedWidget[i];
				if (w)
					widgets[w - 1].repaint(colorToBeSet);
			}
	}
	if (ofInRange(penThread.penX, ofGetWidth() - 100, ofGetWidth()) && ofInRange(penThread.penY, ofGetHeight() - 200, ofGetHeight() - 100))
		for (int i = 0; i < 10; i++) {
			int w = touchedWidget[i];
			if (w)
				widgets[w-1].repaint(colorToBeSet);
		}
	if (canStraightOrCurve && penThread.canStraightOrCurve && pointsDrawn.size() < 32000) {
		ofPoint currentPt, ptToBeDrawn;
		float angle = getPointsAngle(straightOrCurve, penThread.straightOrCurve);
		float width = penThread.pressure;
		currentPt = straightOrCurve;
		ptToBeDrawn = straightOrCurve;
		ofPoint midPoint = ofPoint((penThread.straightOrCurve.x + straightOrCurve.x) / 2, (penThread.straightOrCurve.y + straightOrCurve.y) / 2);
		float half = ofDist(straightOrCurve.x, straightOrCurve.y, midPoint.x, midPoint.y);
		float index = -half;
		//ofPoint endPoint1 = ofPoint(midPoint.x + half * penThread.rotAngle * cos(ofDegToRad(angle + 90)),
			//midPoint.y + half * penThread.rotAngle * sin(ofDegToRad(angle + 90)));
		//float height = ofDist(midPoint.x, midPoint.y, endPoint1.x, endPoint1.y);
		float height = half * penThread.rotAngle;
		//ofPoint endPoint2 = ofPoint(midPoint.x - half * penThread.rotAngle * cos(ofDegToRad(angle + 90)),
			//midPoint.y - half * penThread.rotAngle * sin(ofDegToRad(angle + 90)));
		while ((!ofInRange(currentPt.x, penThread.straightOrCurve.x - 1, penThread.straightOrCurve.x + 1)
			|| !ofInRange(currentPt.y, penThread.straightOrCurve.y - 1, penThread.straightOrCurve.y + 1)) || index <= half) {
				pointsDrawn.push_back(ptToBeDrawn);
				colorsDrawn.push_back(currentColor);
				pointPressures.push_back(width);
				if (pointsDrawn.size() == 32000) break;
				currentPt.x += cos(ofDegToRad(angle));
				currentPt.y += sin(ofDegToRad(angle));
				index++;
				if (ofInRange(ofAngleDifferenceDegrees(angle, penThread.getPenOrientation()), 0, 180)) {	
					ptToBeDrawn.x = currentPt.x + sqrt((height*height*half*half - height*height*index*index) / half / half) * cos(ofDegToRad(angle + 90));
					ptToBeDrawn.y = currentPt.y + sqrt((height*height*half*half - height*height*index*index) / half / half) * sin(ofDegToRad(angle + 90));
				}
				else
				{
					ptToBeDrawn.x = currentPt.x - sqrt((height*height*half*half - height*height*index*index) / half / half) * cos(ofDegToRad(angle + 90));
					ptToBeDrawn.y = currentPt.y - sqrt((height*height*half*half - height*height*index*index) / half / half) * sin(ofDegToRad(angle + 90));
				}
		}
	}
}

