#ifndef TIMER_HPP
#define TIMER_HPP

#include <time.h>

class Timer {
public:
	
	void reset();
	
	Timer();
	
	double time();
	
private:
	
	clock_t start;
	clock_t cur;
	
	const static double CPS;
	
};

#endif