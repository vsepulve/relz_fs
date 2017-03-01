#ifndef NANO_TIMER_H
#define NANO_TIMER_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

//Funciones de Tiempo (compilar con -lrt)
#include <time.h>
#include <sys/timeb.h>

using namespace std;
class NanoTimer{
	private:
		timespec t_ini;
		void diffTime( timespec *t_fin, timespec *t_ini, timespec *delta );
	public:
		NanoTimer();
		virtual ~NanoTimer();
		
		unsigned long long getNanosec();
		double getMilisec();
		void reset();
		
};

#endif //NANO_TIMER_H
