#include "Timer.h"
#include <Windows.h>

Timer::Timer()
{
	LARGE_INTEGER iFreq;
	QueryPerformanceFrequency( &iFreq );
	_Period = 1.0 / iFreq.QuadPart;
	Start();
}

void Timer::Start()
{
	QueryPerformanceCounter( (LARGE_INTEGER*) &_Start );
}

volatile double Timer::ElapsedTime()
{
	LARGE_INTEGER iCurrent;
	QueryPerformanceCounter( &iCurrent );
	return _Period * ( iCurrent.QuadPart - _Start );
}
