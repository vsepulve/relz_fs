
#include "NanoTimer.h"

NanoTimer::NanoTimer(){
	clock_gettime(CLOCK_MONOTONIC, &t_ini);
}

NanoTimer::~NanoTimer(){}

unsigned long long NanoTimer::getNanosec(){
	timespec t_fin, latencia;
	clock_gettime( CLOCK_MONOTONIC, &t_fin );
	diffTime( &t_fin, &t_ini, &latencia );
	return ((unsigned long long)latencia.tv_sec)*1000000000ULL + latencia.tv_nsec;
	
}

double NanoTimer::getMilisec(){
	timespec t_fin, latencia;
	clock_gettime( CLOCK_MONOTONIC, &t_fin );
	diffTime( &t_fin, &t_ini, &latencia );
	return (((long double)latencia.tv_sec)*1000000000ULL + latencia.tv_nsec)/1000000;
}

void NanoTimer::reset(){
	clock_gettime(CLOCK_MONOTONIC, &t_ini);
}

void NanoTimer::diffTime( timespec *t_fin, timespec *t_ini, timespec *delta ){
	if( ( (*t_fin).tv_nsec - (*t_ini).tv_nsec ) < 0 ){
		if( (*t_fin).tv_sec == (*t_ini).tv_sec ){
			(*delta).tv_sec= 0;
			(*delta).tv_nsec = 1000000000 + (*t_fin).tv_nsec - (*t_ini).tv_nsec;
		}
		else{
			(*delta).tv_sec= (*t_fin).tv_sec - (*t_ini).tv_sec - 1;
			(*delta).tv_nsec = 1000000000 + (*t_fin).tv_nsec - (*t_ini).tv_nsec;
		}
	}
	else{
		if( (*t_fin).tv_sec == (*t_ini).tv_sec ){
			(*delta).tv_sec= 0;
			(*delta).tv_nsec = (*t_fin).tv_nsec - (*t_ini).tv_nsec;
		}
		else{
			(*delta).tv_sec= (*t_fin).tv_sec - (*t_ini).tv_sec;
			(*delta).tv_nsec = (*t_fin).tv_nsec - (*t_ini).tv_nsec;
		}
	}
}
