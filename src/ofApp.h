#pragma once

#ifndef OFAPP_H_
#define OFAPP_H_

#include "ofMain.h"
#include "MyTabletObserver.h"
#include "PenThread.h"
#include "WacomMultiTouch.h"
#include "Widget.h"

#define RULER_LENGTH 1807
#define RULER_WIDTH 151

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void exit();
		static void initColors();
		void initBBTabletAndMultiTouch();
		void stopBBTabletAndMultiTouch();
		void restartBBTabletAndMultiTouch();
		static void AttachCallback(WacomMTCapability deviceInfo, void* userRef);
		static void DetachCallback(int deviceID, void* userRef);
		WacomMTError RegisterCallback(int deviceID);
		WacomMTError UnregisterCallback(int deviceID);
		static int FingerCallback(WacomMTFingerCollection* fingerData, void* userRef);
		bbTabletDevice &tD = bbTabletDevice::getInstance();
		bbTabletEvent evt;
		PenThread penThread;
		int deviceDetect;
		static bool rulerDraw;
		bool rulerLock;
		static bool secondColorPaletteDraw;
		static ofPoint secondColorPalettePoint;
		ofPoint cursor;
		ofImage closeIcon, widgetIcon, rulerIcon, colorIcon, cameraIcon, fingerDrawIcon, deleteIcon;
		ofImage ruler;
		static vector<Widget> widgets;
		static bool canDraw;
		static bool canStraightOrCurve;
		static ofPoint straightOrCurve;
		void drawStraightOrCurve();
		bool canAddImage, canAddWidget;
		bool bt1pressed, bt2pressed;
		static bool paletteShown, paletteShown2;
		static ofPoint palettePoint;
		static ofColor recentColors[18];
		void drawColorPalette();
		static bool onPalette(ofPoint point);
		bool canDelete;
		void deleteWidget();
		// int showBar[10];
		// int hideBar[10];
		// void showPhotoBar(int f);
		// void hidePhotoBar(int f);
		static int sliderMoveWidget;
		static float sliderPos;
		static void ofApp::moveSlider(float pos, int w);
		static int canSwitchImage, canBringFront;
		void switchImage(int w, int x);
		int widgetForAddingImage;
		static int widgetForSwitchingImage;
		vector<ofPoint> newWidgetPos;
		void addImage(int w);
		void bringFront(int w);
		static vector<ofPoint> pointsDrawn;
		static vector<ofColor> colorsDrawn;
		static ofColor currentColor, colorToBeSet;
		static vector<float> pointPressures;
		enum tool {widgetTool, rulerTool, rulerToolv2, 
			colorTool, colorToolv2, photoTool, fingerDrawTouchTool, vidTool} currentTool;
		void drawRuler();
		void drawSecondPalette();
		void drawToolbar();
		void drawAllPenData();
		void drawPenCursor();
		static ofPoint rulerPt1, rulerPt2;
		static bool pointOnRuler(ofPoint point);
		static bool pointOnRulerCmSide(ofPoint point);
		static bool pointOnRulerInchSide(ofPoint point);
		static ofPoint perfectPointForCm(ofPoint point);
		static ofPoint perfectPointForInch(ofPoint point);	
		static float getPointsAngle(ofPoint a, ofPoint b);
		static int getHueFromTilt(float a, float b);
		ofEvent<ofPoint> click1, click2;
		ofEvent<Widget> imageLoad;
		static int getWidgetIdByPos(ofPoint point);
		int touchedWidget[10];
		void onTouched(vector<WacomMTFinger> & fingers);
		void onClick1(ofPoint &pos);
		void onClick2(ofPoint &pos);
		friend class Widget;
		friend class PenThread;
		static int w;
		static ofPoint touchedPts[10];
		static ofFile log;
		static ofBuffer data;
	private:
		
		ofPoint wp1[10];
};



#endif
