#pragma once

class Timer  
{
public:
	volatile double ElapsedTime();
	void Start();
	Timer();

protected:
	double   _Period;
	__int64  _Start;
};
