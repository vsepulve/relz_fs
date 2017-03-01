
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

#include <thread>

#include "LocalBuffer.h"
#include "NanoTimer.h"

using namespace std;

void thread_arr(int id, unsigned int min, unsigned int max){
	unsigned int size = rand() % (max - min) + min + 1;
	char *buff = new char[size];
	for(unsigned int j = 0; j < size; ++j){
		buff[j] = (char)(rand()%255);
	}
	delete [] buff;
}

void thread_lbuff(int id, unsigned int min, unsigned int max){
	unsigned int size = rand() % (max - min) + min + 1;
	LocalBuffer lbuff(size);
	for(unsigned int j = 0; j < size; ++j){
		lbuff.memory()[j] = (char)(rand()%255);
	}
}

int main(int argc, char *argv[]){
	
	unsigned int min_buff_size = 128;
	unsigned int max_buff_size = 1024*1024;
	unsigned int n_tests = 100;
	unsigned int n_threads = 12;
	
	cout<<"Inicio (n_tests: "<<n_tests<<", n_threads: "<<n_threads<<", min_buff_size: "<<min_buff_size<<", max_buff_size: "<<max_buff_size<<")\n";
	
	NanoTimer timer;
	vector<thread> arr_threads;
	for(unsigned int i = 0; i < n_tests; ++i){
//		cout<<"Iniciando Threads ["<<i<<"]\n";
		for(unsigned int j = 0; j < n_threads; ++j){
//			arr_threads.push_back( thread( thread_arr, j, min_buff_size, max_buff_size ) );
			arr_threads.push_back( thread( thread_lbuff, j, min_buff_size, max_buff_size ) );
		}
//		cout<<"Esperando Threads ["<<i<<"]\n";
		for(unsigned int j = 0; j < n_threads; ++j){
			arr_threads[j].join();
		}
//		cout<<"Terminando ["<<i<<"]\n";
		arr_threads.clear();
	}
	
	cout<<"Fin ("<<timer.getMilisec()<<")\n";
	
	return 0;
}













