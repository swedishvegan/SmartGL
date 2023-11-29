
#include "./Timer.hpp"

void Timer::reset() { start = clock(); }

Timer::Timer() { reset(); }

double Timer::time() { 
    
    cur = clock();
    return (double)(cur - start) / CPS;
    
}

const double Timer::CPS = (double)CLOCKS_PER_SEC;