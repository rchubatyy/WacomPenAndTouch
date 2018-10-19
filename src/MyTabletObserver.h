#pragma once
#include "bbtablet.h"


class MyTabletObserver : public bbTabletObserver
{
private:
	bbTabletEvent g_evt;
	float _angle;
	float _axis[3];
public:
	int OnTabletEvent(const bbTabletEvent &evt)
	{
		g_evt = evt;
		return 0;
	}

	float getPressure() {
		return g_evt.pressure;
	}

	float getX() {
		return g_evt.x;
	}

	float getY() {
		return g_evt.y;
	}

	float getType() {
		return g_evt.type;
	}

	float getButtons() {
		return g_evt.buttons;
	}

	float getAngle() {
		g_evt.getOrientation(_angle, _axis);
		return _angle;
	}

	float getRotationX() {
		g_evt.getOrientation(_angle, _axis);
		return _axis[1];
	}

	float getRotationY() {
		g_evt.getOrientation(_angle, _axis);
		return _axis[0];
	}

	float getRotationZ() {
		g_evt.getOrientation(_angle, _axis);
		return _axis[2];
	}

	friend class PenThread;
	
};
