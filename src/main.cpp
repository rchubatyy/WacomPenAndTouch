#include "ofMain.h"
#include "ofApp.h"
#pragma comment (linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

//========================================================================
int main( ){
#ifdef _DEBUG
	ofSetupOpenGL(1920,1080,OF_WINDOW);	// <-------- setup the GL context
#else
	ofSetupOpenGL(1920, 1080, OF_FULLSCREEN);	// <-------- setup the GL context
#endif
	ofSetWindowTitle("Roman Chubatyy's Thesis App");

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
