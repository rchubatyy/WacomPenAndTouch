#include "PenThread.h"
#include "ofApp.h"

void PenThread::start() {
	td.initTablet(ofGetWin32Window(), bbTabletDevice::SEPARATE_POINTER);
	td.addObserver(g_callback);
	//td.setDropOldEvents(true);
	startThread();
}

void PenThread::stop() {
	td.removeObserver(g_callback);
	//td.closeDevice();
	stopThread();
}

void PenThread::threadedFunction() {
	while (isThreadRunning()) {
			getPenData();
			if (pressure)
			processPenData();
			else {
				ofApp::colorToBeSet = ofApp::currentColor;
				oldAngle = 0;
				oldOrientation = 0;
				tiltCount = 0;
				drawing = false;
				selectingColor = false;
				secondPointChosen = false;
				if(!ofApp::widgets.empty())
				for (int i = 0; i < ofApp::widgets.size(); i++)
					ofApp::widgets[i].nowDrawing = false;
				canStraightOrCurve = false;
				//oldSat = 0;
				//oldBri = 0;
				//satCount = 0;
				//briCount = 0;
				if(picked){
						ofApp::data.append("Color selected (" + ofToString((int)ofApp::currentColor.r) + ", " + ofToString((int)ofApp::currentColor.g) + ", " + ofToString((int)ofApp::currentColor.b) + ")\n");
						ofApp::data.append("at the point: (" + ofToString(penX) + ", " + ofToString(penY) + ") at\n");
						ofApp::data.append(ofGetTimestampString("%H:%M:%S.%i.\n"));
						ofBufferToFile(ofApp::log.getAbsolutePath(), ofApp::data, false);
					}
				if (picked&&openedFromSwipe) {
					ofApp::paletteShown = false;
					ofApp::canDraw = true;
					openedFromSwipe = false;
				}
				picked = false;
				if (adjusting) {
					ofApp::data.append("Brightness set to: " + ofToString(ofApp::recentColors[0].getBrightness()) + " at\n");
					ofApp::data.append(ofGetTimestampString("%H:%M:%S.%i.\n"));
					ofBufferToFile(ofApp::log.getAbsolutePath(), ofApp::data, false);
				}
				adjusting = false;
				movingSlider = false;
				touchAndPenOnWidget = 0;
				//ofApp::paletteShown2 = false;
			}
	}

}

void PenThread::getPenData() {
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
}

void PenThread::processPenData() {
	vector<Widget>& w = ofApp::widgets;
	int widgetId = ofApp::getWidgetIdByPos(penPos);
	/*int a = ofApp::getWidgetIdByPos(ofApp::touchedPts[0]);
	int* fWidgetId = &a;
	for (int i = 0; i < 10; i++) {
		*fWidgetId = ofApp::getWidgetIdByPos(ofApp::touchedPts[i]);
		fWidgetId++;
	}
	fWidgetId -= 10;*/
	int fWidgetId[10];
	for (int i = 0; i < 10; i++)
		fWidgetId[i] = ofApp::getWidgetIdByPos(ofApp::touchedPts[i]);
	float angleForWidget, distFromStart;
	ofPoint penPosForWidget;
	vector<ofColor>& rCol = ofApp::colorsDrawn;
	vector<ofPoint>& rPt = ofApp::pointsDrawn;
	vector<float>& rPr = ofApp::pointPressures;
	if (ofApp::canStraightOrCurve) {
		canStraightOrCurve = true;
		straightOrCurve = penPos;
		if (!secondPointChosen) {
			ofApp::data.append("Second line point chosen: (" + ofToString(straightOrCurve.x) + ", " + ofToString(straightOrCurve.y) + ") at\n");
			ofApp::data.append(ofGetTimestampString("%H:%M:%S.%i.\n"));
			ofBufferToFile(ofApp::log.getAbsolutePath(), ofApp::data, false);
			secondPointChosen = true;
		}
	}
	else {
		if (penSide == 1) {
			if (!ofApp::canDraw) {
				if (touchAndPenOnWidget && touchAndPenOnWidget == fWidgetId[wFingIndex] && touchAndPenOnWidget != widgetId
					&& !ofApp::paletteShown2 && !ofApp::paletteShown && !movingSlider) {
					//&& !ofInRange(penX, ofApp::palettePoint.x, ofApp::palettePoint.x+470)
					//&& !ofInRange(penY, ofApp::palettePoint.y, ofApp::palettePoint.y + 280) ) {
					ofApp::data.append("Open palette with pen swiping at the point: (" + ofToString(penX) + ", " + ofToString(penY) + ") at\n");
					ofApp::data.append(ofGetTimestampString("%H:%M:%S.%i.\n"));
					ofBufferToFile(ofApp::log.getAbsolutePath(), ofApp::data, false);
					float upperLimit = w[touchAndPenOnWidget - 1].getStartPoint().y;
					float leftLimit = w[touchAndPenOnWidget - 1].getStartPoint().x;
					float rightLimit = w[touchAndPenOnWidget - 1].getStartPoint().x + w[touchAndPenOnWidget - 1].getDim().x;
					float lowerLimit = w[touchAndPenOnWidget - 1].getStartPoint().y + w[touchAndPenOnWidget - 1].getDim().y;
					if (penX <= ofGetWidth() / 2 - 20) {
						if (ofInRange(penX, leftLimit, rightLimit) && ofInRange(penY, upperLimit - 10, upperLimit)) {
							openedFromSwipe = true;
							ofApp::paletteShown = true;
							ofApp::palettePoint = ofPoint(penX + 20, penY - 250);
						}
						if (ofInRange(penX, leftLimit - 10, leftLimit) && ofInRange(penY, upperLimit, lowerLimit)) {
							openedFromSwipe = true;
							ofApp::paletteShown = true;
							ofApp::palettePoint = ofPoint(penX - 450, penY + 30);
						}
						if (ofInRange(penX, rightLimit, rightLimit + 10) && ofInRange(penY, upperLimit, lowerLimit)) {
							openedFromSwipe = true;
							ofApp::paletteShown = true;
							ofApp::palettePoint = ofPoint(penX + 20, penY + 30);
						}
						if (ofInRange(penX, leftLimit, rightLimit) && ofInRange(penY, lowerLimit, lowerLimit + 10)) {
							openedFromSwipe = true;
							ofApp::paletteShown = true;
							ofApp::palettePoint = ofPoint(penX + 20, penY + 30);
						}
					}
					else {
						if (ofInRange(penX, leftLimit, rightLimit) && ofInRange(penY, upperLimit - 10, upperLimit)) {
							openedFromSwipe = true;
							ofApp::paletteShown = true;
							ofApp::palettePoint = ofPoint(penX + 20, penY - 250);
						}
						if (ofInRange(penX, leftLimit - 10, leftLimit) && ofInRange(penY, upperLimit, lowerLimit)) {
							openedFromSwipe = true;
							ofApp::paletteShown = true;
							ofApp::palettePoint = ofPoint(penX - 20, penY + 30);
						}
						if (ofInRange(penX, rightLimit, rightLimit + 10) && ofInRange(penY, upperLimit, lowerLimit)) {
							openedFromSwipe = true;
							ofApp::paletteShown = true;
							ofApp::palettePoint = ofPoint(penX + 450, penY + 30);
						}
						if (ofInRange(penX, leftLimit, rightLimit) && ofInRange(penY, lowerLimit, lowerLimit + 10)) {
							openedFromSwipe = true;
							ofApp::paletteShown = true;
							ofApp::palettePoint = ofPoint(penX + 20, penY + 30);
						}
					}
				}
				touchAndPenOnWidget = 0;
				for (int i = 0; i < 10; i++) {
					if (fWidgetId[i] && widgetId && fWidgetId[i] == widgetId) {
						touchAndPenOnWidget = widgetId;
						wFingIndex = i;
						lastPointOnWidget = penPos;
						if (!ofApp::onPalette(penPos)) {
							openedFromSwipe = false;
							ofApp::paletteShown = false;
						}
						break;
					}
				}
				for (int i = w.size(); i > 0; i--) {
					if (w[i - 1].sliderOn() && ofInRange(penX, w[i - 1].getStartPoint().x, w[i - 1].getStartPoint().x + 200) &&
						ofInRange(penY, w[i - 1].getStartPoint().y - 15, w[i - 1].getStartPoint().y+5))
						//float angle = ofDegToRad(w[i-1].getAngle());
						//if (w[i - 1].sliderOn() && if (w[i - 1].sliderOn() && ofInRange(penX, w[i - 1].getStartPoint().x, w[i - 1].getStartPoint().x + 200) &&
							//ofInRange(penY, w[i - 1].getStartPoint().y - 15, w[i - 1].getStartPoint().y+5))
						// I would like to think about being able to tilt the bar...
						//w[i - 1].moveSlider((penX - w[i - 1].getStartPoint().x)/100);
					{
						movingSlider = true;
						ofApp::sliderMoveWidget = i;
						ofApp::sliderPos = (penX - w[i - 1].getStartPoint().x) / 200;
						break;
					}
				}
			}
			if (ofApp::paletteShown || ofApp::paletteShown) {
				//picked = false;
				if (ofApp::palettePoint.x <= ofGetWidth() / 2) {
					for (int i = 0; i < 18; i++) {
						if (i < 9) {
							if (ofInRange(penX, ofApp::palettePoint.x + 40 + 40 * i, ofApp::palettePoint.x + 70 + 40 * i)
								&& ofInRange(penY, ofApp::palettePoint.y + 30, ofApp::palettePoint.y + 80) && !adjusting) {
								ofApp::currentColor = ofApp::recentColors[i];
								colorSelected = i;
								/*if (!picked) {
									ofApp::data.append("Selected a color (" + ofToString((int)ofApp::currentColor.r) + ", " + ofToString((int)ofApp::currentColor.g) + ", " + ofToString((int)ofApp::currentColor.b) + ")\n");
									ofApp::data.append("at the point: (" + ofToString(penX) + ", " + ofToString(penY) + ") at\n");
									ofApp::data.append(ofGetTimestampString("%H:%M:%S.%i.\n"));
									ofBufferToFile(ofApp::log.getAbsolutePath(), ofApp::data, false);
								}*/
								picked = true;
								break;
							}
						}
						else
							if (ofInRange(penX, ofApp::palettePoint.x + 40 + 40 * (i - 9), ofApp::palettePoint.x + 70 + 40 * (i - 9))
								&& ofInRange(penY, ofApp::palettePoint.y + 130, ofApp::palettePoint.y + 180) && !adjusting) {
								ofApp::currentColor = ofApp::recentColors[i];
								colorSelected = i;
								/*if (!picked) {
									ofApp::data.append("Selected a color (" + ofToString((int)ofApp::currentColor.r) + ", " + ofToString((int)ofApp::currentColor.g) + ", " + ofToString((int)ofApp::currentColor.b) + ")\n");
									ofApp::data.append("at the point: (" + ofToString(penX) + ", " + ofToString(penY) + ") at\n");
									ofApp::data.append(ofGetTimestampString("%H:%M:%S.%i.\n"));
									ofBufferToFile(ofApp::log.getAbsolutePath(), ofApp::data, false);
								}*/
								picked = true;
								break;
							}

					}
					if (ofInRange(penX, ofApp::palettePoint.x + 390, ofApp::palettePoint.x + 420)
						&& ofInRange(penY, ofApp::palettePoint.y - 30, ofApp::palettePoint.y + 250) && !picked) 
						getBrightnessFromTilt();
					if (ofInRange(penX, ofApp::palettePoint.x - 20, ofApp::palettePoint.x + 389)
						&& ofInRange(penY, ofApp::palettePoint.y + 81, ofApp::palettePoint.y + 129) && !picked) {
						adjusting = true;
						ofApp::initColors();
					}
					else if (ofInRange(penX, ofApp::palettePoint.x + 421, ofApp::palettePoint.x + 450)
						&& ofInRange(penY, ofApp::palettePoint.y - 30, ofApp::palettePoint.y + 250) && !picked)
						getBrightnessFromTilt();
				}
				else {
					for (int i = 0; i < 18; i++) {
						if (i < 9) {
							if (ofInRange(penX, ofApp::palettePoint.x - 390 + 40 * i, ofApp::palettePoint.x - 360 + 40 * i)
								&& ofInRange(penY, ofApp::palettePoint.y + 30, ofApp::palettePoint.y + 80) && !adjusting) {
								ofApp::currentColor = ofApp::recentColors[i];
								colorSelected = i;
								/*if (!picked) {
									ofApp::data.append("Selected a color (" + ofToString((int)ofApp::currentColor.r) + ", " + ofToString((int)ofApp::currentColor.g) + ", " + ofToString((int)ofApp::currentColor.b) + ")\n");
									ofApp::data.append("at the point: (" + ofToString(penX) + ", " + ofToString(penY) + ") at\n");
									ofApp::data.append(ofGetTimestampString("%H:%M:%S.%i.\n"));
									ofBufferToFile(ofApp::log.getAbsolutePath(), ofApp::data, false);
								}*/
								picked = true;
								break;
							}
						}
						else
							if (ofInRange(penX, ofApp::palettePoint.x - 390 + 40 * (i - 9), ofApp::palettePoint.x - 360 + 40 * (i - 9))
								&& ofInRange(penY, ofApp::palettePoint.y + 130, ofApp::palettePoint.y + 180) && !adjusting) {
								ofApp::currentColor = ofApp::recentColors[i];
								colorSelected = i;
								/*if (!picked) {
									ofApp::data.append("Selected a color (" + ofToString((int)ofApp::currentColor.r) + ", " + ofToString((int)ofApp::currentColor.g) + ", " + ofToString((int)ofApp::currentColor.b) + ")\n");
									ofApp::data.append("at the point: (" + ofToString(penX) + ", " + ofToString(penY) + ") at\n");
									ofApp::data.append(ofGetTimestampString("%H:%M:%S.%i.\n"));
									ofBufferToFile(ofApp::log.getAbsolutePath(), ofApp::data, false);
								}*/
								picked = true;
								break;
							}
						if (ofInRange(penX, ofApp::palettePoint.x - 419, ofApp::palettePoint.x - 390)
							&& ofInRange(penY, ofApp::palettePoint.y - 30, ofApp::palettePoint.y + 250) && !picked) 
							getBrightnessFromTilt();
						if (ofInRange(penX, ofApp::palettePoint.x - 389, ofApp::palettePoint.x + 20)
							&& ofInRange(penY, ofApp::palettePoint.y + 81, ofApp::palettePoint.y + 129) && !picked ) {
							adjusting = true;
							ofApp::initColors();
						}
						else if (ofInRange(penX, ofApp::palettePoint.x - 450, ofApp::palettePoint.x - 420)
							&& ofInRange(penY, ofApp::palettePoint.y - 30, ofApp::palettePoint.y + 250) && !picked) 
							getBrightnessFromTilt();
					}
				}

			}
			if (!ofApp::canDraw && widgetId  && !picked && !adjusting) {
				//ofApp::canBringFront = widgetId;
				if (pressure && pressure <= 0.8 && !movingSlider) {
					if (!oldOrientation)
					{
						oldOrientation = getPenOrientation();
						rotAxis = penPos;
					}
					else {
						//float tempOr = getPenOrientation() - oldOrientation;
						float tempOr = ofAngleDifferenceDegrees(oldOrientation, getPenOrientation());
						if (tempOr == tempOr) {
							/*w[widgetId - 1].moveTo(penX + ((penY - w[widgetId - 1].getStartPoint().y) * sin(ofDegToRad(tempOr))
								- (penX - w[widgetId - 1].getStartPoint().x) * cos(ofDegToRad(tempOr))),
								penY - ((penX - w[widgetId - 1].getStartPoint().x) * sin(ofDegToRad(tempOr))
									+ (penY - w[widgetId - 1].getStartPoint().y) * cos(ofDegToRad(tempOr))));*/
							w[widgetId - 1].moveTo(rotAxis.x + ((rotAxis.y - w[widgetId - 1].getStartPoint().y) * sin(ofDegToRad(tempOr))
								- (rotAxis.x - w[widgetId - 1].getStartPoint().x) * cos(ofDegToRad(tempOr))),
								rotAxis.y - ((rotAxis.x - w[widgetId - 1].getStartPoint().x) * sin(ofDegToRad(tempOr))
									+ (rotAxis.y - w[widgetId - 1].getStartPoint().y) * cos(ofDegToRad(tempOr))));

							w[widgetId - 1].relatRotate(tempOr);
						}
						oldOrientation = getPenOrientation();
					}
				}
				if (pressure > 0.8 && !movingSlider) {
					oldOrientation = 0;
					if (!oldAngle) oldAngle = getPenAngle();
					else if (w[widgetId - 1].getCurrentImage()) {
						float tempAng = ofAngleDifferenceDegrees(oldAngle, getPenAngle());
						if (tempAng == tempAng) {
							tiltCount -= tempAng;
						}
						if (tiltCount >= 30 && w[widgetId - 1].getCurrentImage() < w[widgetId - 1].imagePaths.size()) {
							//w[widgetId - 1].setCurrentImage(w[widgetId - 1].getCurrentImage() + 1);
							ofApp::widgetForSwitchingImage = widgetId - 1;
							ofApp::canSwitchImage = w[widgetId - 1].getCurrentImage() + 1;
							tiltCount = 0;
						}
						if (tiltCount <= -30 && w[widgetId - 1].getCurrentImage() > 1) {
							//w[widgetId - 1].setCurrentImage(w[widgetId - 1].getCurrentImage() - 1);
							ofApp::widgetForSwitchingImage = widgetId - 1;
							ofApp::canSwitchImage = w[widgetId - 1].getCurrentImage() - 1;
							tiltCount = 0;
						}
						oldAngle = getPenAngle();
					}
				}
				return;
			}
			if (ofApp::secondColorPaletteDraw) {
				if (ofInRange(penX, ofApp::secondColorPalettePoint.x, ofApp::secondColorPalettePoint.x + 100)
					&& ofInRange(penY, ofApp::secondColorPalettePoint.y + 50, ofApp::secondColorPalettePoint.y + 150)) {
					ofApp::colorToBeSet.setHsb(ofApp::getHueFromTilt(rotX, rotY), 255 * pressure, 255 * rotAngle);
					if (!selectingColor) {
						selectingColor = true;
						ofApp::data.append("Touched tilt palette on (" + ofToString(penX) + ", " + ofToString(penY) + ") at\n");
						ofApp::data.append(ofGetTimestampString("%H:%M:%S.%i.\n"));
						ofBufferToFile(ofApp::log.getAbsolutePath(), ofApp::data, false);
					}
				}
			}
			if (penX > ofGetWidth() - 100) {
				if (ofInRange(penY, ofGetHeight() - 200, ofGetHeight() - 100)) {
					ofApp::colorToBeSet.setHsb(ofApp::getHueFromTilt(rotX, rotY), 255 * pressure, 255 * rotAngle);
					if (!selectingColor) {
						selectingColor = true;
						ofApp::data.append("Touched tilt palette on (" + ofToString(penX) + ", " + ofToString(penY) + ") at\n");
						ofApp::data.append(ofGetTimestampString("%H:%M:%S.%i.\n"));
						ofBufferToFile(ofApp::log.getAbsolutePath(), ofApp::data, false);
					}
				}
			}
			else if (ofApp::canDraw) {
				if (widgetId) {

					if (!ofApp::rulerDraw || !ofApp::pointOnRuler(penPos)) {
						angleForWidget = ofAngleDifferenceDegrees(w[widgetId - 1].getAngle(),
							ofApp::getPointsAngle(w[widgetId - 1].getStartPoint(), penPos));
						distFromStart = ofDist(w[widgetId - 1].getStartPoint().x, w[widgetId - 1].getStartPoint().y, penX, penY);
						penPosForWidget.x = distFromStart * cos(ofDegToRad(angleForWidget));
						penPosForWidget.y = distFromStart * sin(ofDegToRad(angleForWidget));
						if (ofInRange(penPosForWidget.x, 0, w[widgetId - 1].getDim().x) && ofInRange(penPosForWidget.y, 0, w[widgetId - 1].getDim().y)
							&& w[widgetId - 1].pointsDrawn.size()<32000)
						{
							if (w[widgetId - 1].pointsDrawn.empty() || w[widgetId - 1].pointsDrawn.back() != penPosForWidget) {
								w[widgetId - 1].colorsDrawn.push_back(ofApp::currentColor);
								w[widgetId - 1].pointsDrawn.push_back(penPosForWidget);
								w[widgetId - 1].pointPressures.push_back(pressure);
								//w[widgetId - 1].pointsDrawn.shrink_to_fit();
								//w[widgetId - 1].colorsDrawn.shrink_to_fit();
								//w[widgetId - 1].pointPressures.shrink_to_fit();
								if (w[widgetId - 1].pointsDrawn.size() > 1) {
									if (!ofInRange(w[widgetId - 1].pointsDrawn.back().x - w[widgetId - 1].pointsDrawn[w[widgetId - 1].pointsDrawn.size() - 2].x, -2, 2)
										|| !ofInRange(w[widgetId - 1].pointsDrawn.back().y - w[widgetId - 1].pointsDrawn[w[widgetId - 1].pointsDrawn.size() - 2].y, -2, 2))
										//if (!drawing)
											//drawing = true;				
											if(!w[widgetId - 1].nowDrawing){
											drawing=false;
											for (int i=0; i<w.size();i++){
											if (i==widgetId-1)
												w[i].nowDrawing = true;
											else w[i].nowDrawing=false;}
											}
										else {
											float angle = ofApp::getPointsAngle(w[widgetId - 1].pointsDrawn[w[widgetId - 1].pointsDrawn.size() - 2], w[widgetId - 1].pointsDrawn.back());
											w[widgetId - 1].colorsDrawn.push_back(ofApp::currentColor);
											w[widgetId - 1].pointsDrawn.push_back(ofPoint(w[widgetId - 1].pointsDrawn[w[widgetId - 1].pointsDrawn.size() - 2].x + cos(ofDegToRad(angle)),
												w[widgetId - 1].pointsDrawn[w[widgetId - 1].pointsDrawn.size() - 2].y + sin(ofDegToRad(angle))));
											w[widgetId - 1].pointPressures.push_back(pressure);
											//w[widgetId - 1].pointsDrawn.shrink_to_fit();
											//w[widgetId - 1].colorsDrawn.shrink_to_fit();
											//w[widgetId - 1].pointPressures.shrink_to_fit();
										}
								}
							}
						}
					}
					else {
						if (ofApp::pointOnRulerCmSide(penPos))
						{
							angleForWidget = ofAngleDifferenceDegrees(w[widgetId - 1].getAngle(),
								ofApp::getPointsAngle(w[widgetId - 1].getStartPoint(), ofApp::perfectPointForCm(penPos)));
							distFromStart = ofDist(w[widgetId - 1].getStartPoint().x, w[widgetId - 1].getStartPoint().y, ofApp::perfectPointForCm(penPos).x, ofApp::perfectPointForCm(penPos).y);
							penPosForWidget.x = distFromStart * cos(ofDegToRad(angleForWidget));
							penPosForWidget.y = distFromStart * sin(ofDegToRad(angleForWidget));

							if (ofInRange(penPosForWidget.x, 0, w[widgetId - 1].getDim().x) && ofInRange(penPosForWidget.y, 0, w[widgetId - 1].getDim().y)
								&& w[widgetId - 1].pointsDrawn.size() < 32000)
							{
								if (w[widgetId - 1].pointsDrawn.empty() || w[widgetId - 1].pointsDrawn.back() != penPosForWidget) {
									w[widgetId - 1].colorsDrawn.push_back(ofApp::currentColor);
									w[widgetId - 1].pointsDrawn.push_back(penPosForWidget);
									w[widgetId - 1].pointPressures.push_back(pressure);
									//w[widgetId - 1].pointsDrawn.shrink_to_fit();
									//w[widgetId - 1].colorsDrawn.shrink_to_fit();
									//w[widgetId - 1].pointPressures.shrink_to_fit();
									if (w[widgetId - 1].pointsDrawn.size() > 1) {
										if (!ofInRange(w[widgetId - 1].pointsDrawn.back().x - w[widgetId - 1].pointsDrawn[w[widgetId - 1].pointsDrawn.size() - 2].x, -2, 2)
											|| !ofInRange(w[widgetId - 1].pointsDrawn.back().y - w[widgetId - 1].pointsDrawn[w[widgetId - 1].pointsDrawn.size() - 2].y, -2, 2))
											//if (!drawing)
												//drawing = true;
										if(!w[widgetId - 1].nowDrawing){
										drawing=false;
										for (int i=0; i<w.size();i++){
										if (i==widgetId-1) 
											w[i].nowDrawing = true;
										 else w[i].nowDrawing=false;}
										}
											else {
												float angle = ofApp::getPointsAngle(w[widgetId - 1].pointsDrawn[w[widgetId - 1].pointsDrawn.size() - 2], w[widgetId - 1].pointsDrawn.back());
												w[widgetId - 1].colorsDrawn.push_back(ofApp::currentColor);
												w[widgetId - 1].pointsDrawn.push_back(ofPoint(w[widgetId - 1].pointsDrawn[w[widgetId - 1].pointsDrawn.size() - 2].x + cos(ofDegToRad(angle)),
													w[widgetId - 1].pointsDrawn[w[widgetId - 1].pointsDrawn.size() - 2].y + sin(ofDegToRad(angle))));
												w[widgetId - 1].pointPressures.push_back(pressure);
												//w[widgetId - 1].pointsDrawn.shrink_to_fit();
												//w[widgetId - 1].colorsDrawn.shrink_to_fit();
												//w[widgetId - 1].pointPressures.shrink_to_fit();
											}
									}
								}
							}
						}
						if (ofApp::pointOnRulerInchSide(penPos))
						{
							angleForWidget = ofAngleDifferenceDegrees(w[widgetId - 1].getAngle(),
								ofApp::getPointsAngle(w[widgetId - 1].getStartPoint(), ofApp::perfectPointForInch(penPos)));
							distFromStart = ofDist(w[widgetId - 1].getStartPoint().x, w[widgetId - 1].getStartPoint().y, ofApp::perfectPointForInch(penPos).x, ofApp::perfectPointForInch(penPos).y);
							penPosForWidget.x = distFromStart * cos(ofDegToRad(angleForWidget));
							penPosForWidget.y = distFromStart * sin(ofDegToRad(angleForWidget));

							if (ofInRange(penPosForWidget.x, 0, w[widgetId - 1].getDim().x) && ofInRange(penPosForWidget.y, 0, w[widgetId - 1].getDim().y)
								&& w[widgetId - 1].pointsDrawn.size() < 32000)
							{
								if (w[widgetId - 1].pointsDrawn.empty() || w[widgetId - 1].pointsDrawn.back() != penPosForWidget) {
									w[widgetId - 1].colorsDrawn.push_back(ofApp::currentColor);
									w[widgetId - 1].pointsDrawn.push_back(penPosForWidget);
									w[widgetId - 1].pointPressures.push_back(pressure);
									//w[widgetId - 1].pointsDrawn.shrink_to_fit();
									//w[widgetId - 1].colorsDrawn.shrink_to_fit();
									//w[widgetId - 1].pointPressures.shrink_to_fit();
									if (w[widgetId - 1].pointsDrawn.size() > 1) {
										if (!ofInRange(w[widgetId - 1].pointsDrawn.back().x - w[widgetId - 1].pointsDrawn[w[widgetId - 1].pointsDrawn.size() - 2].x, -2, 2)
											|| !ofInRange(w[widgetId - 1].pointsDrawn.back().y - w[widgetId - 1].pointsDrawn[w[widgetId - 1].pointsDrawn.size() - 2].y, -2, 2))
											//if (!drawing)
												//drawing = true;
										if(!w[widgetId - 1].nowDrawing){
										drawing=false;
										for (int i=0; i<w.size();i++){
										if (i==widgetId-1) 
											w[i].nowDrawing = true;
										 else w[i].nowDrawing=false;}
										}
											else {
												float angle = ofApp::getPointsAngle(w[widgetId - 1].pointsDrawn[w[widgetId - 1].pointsDrawn.size() - 2], w[widgetId - 1].pointsDrawn.back());
												w[widgetId - 1].colorsDrawn.push_back(ofApp::currentColor);
												w[widgetId - 1].pointsDrawn.push_back(ofPoint(w[widgetId - 1].pointsDrawn[w[widgetId - 1].pointsDrawn.size() - 2].x + cos(ofDegToRad(angle)),
													w[widgetId - 1].pointsDrawn[w[widgetId - 1].pointsDrawn.size() - 2].y + sin(ofDegToRad(angle))));
												w[widgetId - 1].pointPressures.push_back(pressure);
												//w[widgetId - 1].pointsDrawn.shrink_to_fit();
												//w[widgetId - 1].colorsDrawn.shrink_to_fit();
												//w[widgetId - 1].pointPressures.shrink_to_fit();
											}
									}
								}
							}
						}
					}
				}
				else {
					if (!ofApp::rulerDraw || !ofApp::pointOnRuler(penPos)) {
						if ((rPt.empty() || rPt.back() != penPos) && rPt.size() < 32000) {
							//if (ofInRange(penX, rPt.back().x - 1, rPt.back().x + 1) && ofInRange(penY, rPt.back().y - 1, rPt.back().y + 1)) {
								rCol.push_back(ofApp::currentColor);
								rPt.push_back(penPos);
								rPr.push_back(pressure);
								//rPt.shrink_to_fit();
								//rCol.shrink_to_fit();
								//rPr.shrink_to_fit();
								if (rPt.size() > 1) {
									if (!ofInRange(rPt.back().x - rPt[rPt.size() - 2].x, -2, 2)
										|| !ofInRange(rPt.back().y - rPt[rPt.size() - 2].y, -2, 2))
										if (!drawing)
										{
											drawing = true;
											for (int i = 0; i < w.size(); i++)
												w[i].nowDrawing = false;
										}
										else {
											float angle = ofApp::getPointsAngle(rPt[rPt.size() - 2], rPt.back());
											rCol.push_back(ofApp::currentColor);
											rPt.push_back(ofPoint(rPt[rPt.size() - 2].x + cos(ofDegToRad(angle)), rPt[rPt.size() - 2].y + sin(ofDegToRad(angle))));
											rPr.push_back(pressure);
											//rPt.shrink_to_fit();
											//rCol.shrink_to_fit();
											//rPr.shrink_to_fit();
										}
								}
							//}
						}
					}
					else {
						if (ofApp::pointOnRulerCmSide(penPos))
						{
							ofPoint p = ofApp::perfectPointForCm(penPos);
							if ((rPt.empty() || rPt.back() != p) && rPt.size() < 32000) {
								rCol.push_back(ofApp::currentColor);
								rPt.push_back(p);
								rPr.push_back(pressure);
								//rPt.shrink_to_fit();
								//rCol.shrink_to_fit();
								//rPr.shrink_to_fit();
								if (rPt.size() > 1) {
									if (!ofInRange(rPt.back().x - rPt[rPt.size() - 2].x, -2, 2)
										|| !ofInRange(rPt.back().y - rPt[rPt.size() - 2].y, -2, 2))
										if (!drawing)
											{
											drawing = true;
									for (int i=0; i<w.size();i++)
									 w[i].nowDrawing=false;
									}
										else {
											float angle = ofApp::getPointsAngle(rPt[rPt.size() - 2], rPt.back());
											rCol.push_back(ofApp::currentColor);
											rPt.push_back(ofPoint(rPt[rPt.size() - 2].x + cos(ofDegToRad(angle)), rPt[rPt.size() - 2].y + sin(ofDegToRad(angle))));
											rPr.push_back(pressure);
											//rPt.shrink_to_fit();
											//rCol.shrink_to_fit();
											//rPr.shrink_to_fit();
										}
								}
							}
						}
						if (ofApp::pointOnRulerInchSide(penPos))
						{
							ofPoint p = ofApp::perfectPointForInch(penPos);
							if ((rPt.empty() || rPt.back() != p) && rPt.size() < 32000) {
								rCol.push_back(ofApp::currentColor);
								rPt.push_back(p);
								rPr.push_back(pressure);
								//rPt.shrink_to_fit();
								//rCol.shrink_to_fit();
								//rPr.shrink_to_fit();
								if (rPt.size() > 1) {
									if (!ofInRange(rPt.back().x - rPt[rPt.size() - 2].x, -2, 2)
										|| !ofInRange(rPt.back().y - rPt[rPt.size() - 2].y, -2, 2))
										if (!drawing)
											{
											drawing = true;
									for (int i=0; i<w.size();i++)
									w[i].nowDrawing=false;
									}
										else {
											float angle = ofApp::getPointsAngle(rPt[rPt.size() - 2], rPt.back());
											rCol.push_back(ofApp::currentColor);
											rPt.push_back(ofPoint(rPt[rPt.size() - 2].x + cos(ofDegToRad(angle)), rPt[rPt.size() - 2].y + sin(ofDegToRad(angle))));
											rPr.push_back(pressure);
											//rPt.shrink_to_fit();
											//rCol.shrink_to_fit();
											//rPr.shrink_to_fit();
										}
								}
							}
						}
					}
				}
			}
		}
		else if (penSide == 2) {
			if (!widgetId) {
				for (int i = 0; i < rPt.size(); i++)
					//if (rPt[i] == penPos) {
					if (ofInRange(rPt[i].x, penX - pressure * 5, penX + pressure * 5)
						&& ofInRange(rPt[i].y, penY - pressure * 5, penY + pressure * 5)) {
						if (!ofApp::rulerDraw || !ofApp::pointOnRuler(rPt[i])) {
							rPt.erase(rPt.begin() + i);
							rCol.erase(rCol.begin() + i);
							rPr.erase(rPr.begin() + i);
							//rPt.shrink_to_fit();
							//rCol.shrink_to_fit();
							//rPr.shrink_to_fit();
							break;
						}
					}
			}
			else {
				angleForWidget = ofAngleDifferenceDegrees(w[widgetId - 1].getAngle(),
					ofApp::getPointsAngle(w[widgetId - 1].getStartPoint(), penPos));
				distFromStart = ofDist(w[widgetId - 1].getStartPoint().x, w[widgetId - 1].getStartPoint().y, penX, penY);
				penPosForWidget.x = distFromStart * cos(ofDegToRad(angleForWidget));
				penPosForWidget.y = distFromStart * sin(ofDegToRad(angleForWidget));
				for (int i = 0; i < w[widgetId - 1].pointsDrawn.size(); i++)
					//if (w[widgetId - 1].pointsDrawn[i] == penPosForWidget) {
					if (ofInRange(w[widgetId - 1].pointsDrawn[i].x, penPosForWidget.x - pressure * 5, penPosForWidget.x + pressure * 5)
						&& ofInRange(w[widgetId - 1].pointsDrawn[i].y, penPosForWidget.y - pressure * 5, penPosForWidget.y + pressure * 5)) {
						if (!ofApp::rulerDraw || !ofApp::pointOnRuler(w[widgetId - 1].pointsDrawn[i])) {
							w[widgetId - 1].pointsDrawn.erase(w[widgetId - 1].pointsDrawn.begin() + i);
							w[widgetId - 1].colorsDrawn.erase(w[widgetId - 1].colorsDrawn.begin() + i);
							w[widgetId - 1].pointPressures.erase(w[widgetId - 1].pointPressures.begin() + i);
							//w[widgetId - 1].pointsDrawn.shrink_to_fit();
							//w[widgetId - 1].colorsDrawn.shrink_to_fit();
							//w[widgetId - 1].pointPressures.shrink_to_fit();
							break;
						}
					}
			}
		}
	}
}

float PenThread::getPenAngle()
{
	if (!rotAngle) return 90;
	if (rotX > 0)
		return 90 * (1 - rotAngle);
	else return 90 + 90 * (rotAngle);
}

float PenThread::getPenVAngle() {
	if (!rotAngle) return 90;
	if (rotY <= 0)
		return 90 * (1 - rotAngle);
	else return 90 + 90 * (rotAngle);
}

float PenThread::getPenOrientation()
{
	if (rotY >= 0)
		return ofRadToDeg(acos(rotX));
	else
		return 180 + ofRadToDeg(acos(-rotX));
}

float PenThread::getPenPositiveOrientation()
{
	float angle=getPenOrientation();
	if (angle < 0)
		return angle + 360;
	else return angle;
}

void PenThread::getSaturationFromTilt()
{
	adjusting = true;
	int a = (int)(getPenVAngle() / 180 * 255);
	for (int i = 0; i < 9; i++) {
		if (a > 0)
		ofApp::recentColors[i].setSaturation(a);
		ofApp::recentColors[i].clamp();
	}
}

void PenThread::getBrightnessFromTilt()
{
	adjusting = true;
	int a = (int)(getPenVAngle() / 180 * 255);
	for (int i = 0; i < 9; i++) {
		if (a > 0)
		ofApp::recentColors[i].setBrightness(a);
		ofApp::recentColors[i].clamp();
	}

}






