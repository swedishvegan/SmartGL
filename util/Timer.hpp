#ifndef TIMER_HPP
#define TIMER_HPP

#include <time.h>

class Timer {
public:
	
	void reset() { start = clock(); }
	
	Timer() { reset(); }
	
	double time() { 
		
		cur = clock();
		return (double)(cur - start) / CPS;
		
	}
	
private:
	
	clock_t start;
	clock_t cur;
	
	const static double CPS;
	
};

const double Timer::CPS = (double)CLOCKS_PER_SEC;

#endif